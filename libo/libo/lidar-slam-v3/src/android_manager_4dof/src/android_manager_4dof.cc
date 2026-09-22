// Central lifecycle owner for fusion_4dof localization. All external commands
// and watchdog/recovery requests become explicit state-machine transitions.
// This node is the only process allowed to execute restart_ros_nodes.sh.

#include "android_manager_4dof/lifecycle_mode.h"
#include "util/GpsPosition.h"
#include "util/LIOPose.h"
#include "util/LocalizationStatus.h"
#include "util/ManageLocalizationMap.h"
#include "util/Position.h"
#include <ros/ros.h>
#include <std_msgs/Bool.h>
#include <std_msgs/String.h>
#include <array>
#include <atomic>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <deque>
#include <fstream>
#include <mutex>
#include <string>
#include <system_error>
#include <thread>
#include <sys/wait.h>
#include <unistd.h>

namespace {
constexpr int kResumeFrames = 5;
constexpr int kFrequencyArmFrames = 50;
constexpr int kLowFrequencyWindows = 10;
constexpr double kMinimumPositionHz = 7.0;
constexpr double kPositionStopTimeoutSec = 1.0;
constexpr double kPositionRestartTimeoutSec = 10.0;
constexpr double kLioAliveTimeoutSec = 1.0;
constexpr double kAlgorithmStatusTimeoutSec = 2.0;
constexpr int kMaxAutomaticRestarts = 3;
constexpr double kAutomaticRestartWindowSec = 120.0;

bool IsProductionPosition(uint32_t state) {
  return state == 1 || state == 2 || state == 5;
}

using RecoveryRuntime = android_manager_4dof::RecoveryRuntime;
using GenerationToken = android_manager_4dof::GenerationToken;

struct RestartBaseline {
  GenerationToken fusion;
  GenerationToken lio;
};

bool IsKnownFusionRuntime(RecoveryRuntime runtime) {
  return runtime == RecoveryRuntime::kCreateOrigin ||
         runtime == RecoveryRuntime::kTrackSavedOrigin;
}

uint8_t RuntimeStatusValue(RecoveryRuntime runtime) {
  switch (runtime) {
  case RecoveryRuntime::kCreateOrigin:
    return util::LocalizationStatus::RUNTIME_CREATE_ORIGIN;
  case RecoveryRuntime::kTrackSavedOrigin:
    return util::LocalizationStatus::RUNTIME_TRACK_SAVED_ORIGIN;
  case RecoveryRuntime::kUnknown:
    return util::LocalizationStatus::RUNTIME_UNKNOWN;
  }
  return util::LocalizationStatus::RUNTIME_UNKNOWN;
}

uint8_t LegacyModeForRuntime(RecoveryRuntime runtime) {
  switch (runtime) {
  case RecoveryRuntime::kCreateOrigin:
    return util::LocalizationStatus::MODE_MAPPING;
  case RecoveryRuntime::kTrackSavedOrigin:
    return util::LocalizationStatus::MODE_LOCALIZATION;
  case RecoveryRuntime::kUnknown:
    return util::LocalizationStatus::MODE_UNKNOWN;
  }
  return util::LocalizationStatus::MODE_UNKNOWN;
}

const char *RuntimeName(RecoveryRuntime runtime) {
  switch (runtime) {
  case RecoveryRuntime::kCreateOrigin:
    return "CREATE_ORIGIN";
  case RecoveryRuntime::kTrackSavedOrigin:
    return "TRACK_SAVED_ORIGIN";
  case RecoveryRuntime::kUnknown:
    return "UNKNOWN";
  }
  return "INVALID";
}
bool SplitCommand(const std::string &input, std::string *verb,
                  std::string *argument) {
  if (verb == nullptr || argument == nullptr)
    return false;
  const std::size_t slash = input.find('/');
  *verb = input.substr(0, slash);
  *argument = slash == std::string::npos ? std::string()
                                         : input.substr(slash + 1);
  return !verb->empty();
}
bool IsSafeMapName(const std::string &name) {
  if (name.empty() || name.size() > 96 || name == "." || name == ".." ||
      name == "my_map")
    return false;
  for (char c : name) {
    const unsigned char u = static_cast<unsigned char>(c);
    if (!(std::isalnum(u) || c == '_' || c == '-')) return false;
  }
  return true;
}

bool ReadValidMapOrigin(const std::string &source_path,
                        std::array<char, 36> *data, std::string *error) {
  if (data == nullptr) return false;
  data->fill(0);
  std::ifstream source(source_path, std::ios::binary);
  if (!source.is_open()) {
    if (error) *error = "unable to open map-origin source: " + source_path;
    return false;
  }
  source.read(data->data(), static_cast<std::streamsize>(data->size()));
  const std::streamsize bytes_read = source.gcount();
  char trailing_byte = 0;
  const bool has_trailing_data = static_cast<bool>(source.read(&trailing_byte, 1));
  source.close();
  if (bytes_read != static_cast<std::streamsize>(data->size()) ||
      has_trailing_data) {
    if (error) *error = "map-origin file must contain exactly 36 bytes: " + source_path;
    return false;
  }
  double latitude = 0.0, longitude = 0.0, height = 0.0, yaw = 0.0;
  std::memcpy(&latitude, data->data() + 4, sizeof(double));
  std::memcpy(&longitude, data->data() + 12, sizeof(double));
  std::memcpy(&height, data->data() + 20, sizeof(double));
  std::memcpy(&yaw, data->data() + 28, sizeof(double));
  if (!std::isfinite(latitude) || latitude < -90.0 || latitude > 90.0 ||
      !std::isfinite(longitude) || longitude < -180.0 || longitude > 180.0 ||
      !std::isfinite(height) || !std::isfinite(yaw) || yaw < 0.0 || yaw >= 360.0) {
    if (error) *error = "map-origin values are invalid: " + source_path;
    return false;
  }
  return true;
}

} // namespace

class LocalizationManager {
public:
  LocalizationManager() : private_nh_("~") {
    private_nh_.param<std::string>("android_topic", android_topic_, "/signal");
    private_nh_.param<std::string>("gps_topic", gps_topic_, "/nanobot/gpsposition");
    private_nh_.param<std::string>("fusion_topic", fusion_topic_, "/Mower/position");
    private_nh_.param<std::string>("reboot_topic", reboot_topic_, "/Mower/reboot");
    private_nh_.param<std::string>("stop_car_topic", stop_car_topic_, "/mower/manager_stop_car");
    private_nh_.param<std::string>("fusion_stop_car_topic",
                                   fusion_stop_car_topic_,
                                   "/mower/fusion_stop_car");
    private_nh_.param<std::string>("final_stop_car_topic",
                                   final_stop_car_topic_,
                                   "/mower/stop_car");
    private_nh_.param<std::string>("lio_static_status_topic", lio_static_status_topic_, "/Mower/lio_static_initialized");
    private_nh_.param<std::string>("imu_odometry_ready_topic", imu_odometry_ready_topic_, "/Mower/imu_odometry/ready");
    private_nh_.param<std::string>("algorithm_status_topic", algorithm_status_topic_, "/Mower/localization_algorithm_status");
    private_nh_.param<std::string>("status_topic", status_topic_, "/Mower/localization_status");
    private_nh_.param<std::string>("map_service", map_service_name_, "/Mower/manage_localization_map");
    private_nh_.param<std::string>("map_path", map_path_, "./my_map.mp");
    private_nh_.param<std::string>("fusion_package", fusion_package_,
                                   "fusion_4dof");
    if (fusion_package_ != "fusion_4dof" &&
        fusion_package_ != "fusion_4dof_04m") {
      ROS_WARN("Unsupported fusion_package='%s'; use fusion_4dof.",
               fusion_package_.c_str());
      fusion_package_ = "fusion_4dof";
    }
    private_nh_.param<bool>("require_imu_odometry_ready", require_imu_odometry_ready_, true);
    private_nh_.param<double>("transition_status_timeout_sec",
                              transition_status_timeout_sec_, 10.0);
    if (!std::isfinite(transition_status_timeout_sec_) ||
        transition_status_timeout_sec_ < kAlgorithmStatusTimeoutSec) {
      ROS_WARN("Invalid transition_status_timeout_sec=%.3f; use 10.0 seconds.",
               transition_status_timeout_sec_);
      transition_status_timeout_sec_ = 10.0;
    }
    private_nh_.param<double>("restart_generation_timeout_sec",
                              restart_generation_timeout_sec_, 30.0);
    if (!std::isfinite(restart_generation_timeout_sec_) ||
        restart_generation_timeout_sec_ < kAlgorithmStatusTimeoutSec) {
      ROS_WARN("Invalid restart_generation_timeout_sec=%.3f; use 30.0 seconds.",
               restart_generation_timeout_sec_);
      restart_generation_timeout_sec_ = 30.0;
    }
    private_nh_.param<double>("measurement_max_age_sec",
                              measurement_max_age_sec_, 0.50);
    private_nh_.param<double>("measurement_max_future_sec",
                              measurement_max_future_sec_, 0.05);
    if (!std::isfinite(measurement_max_age_sec_) ||
        measurement_max_age_sec_ <= 0.0 ||
        !std::isfinite(measurement_max_future_sec_) ||
        measurement_max_future_sec_ < 0.0) {
      ROS_WARN("Invalid manager measurement timestamp limits; use 0.50s age "
               "and 0.05s future tolerance.");
      measurement_max_age_sec_ = 0.50;
      measurement_max_future_sec_ = 0.05;
    }

    pub_manager_stop_car_ =
        nh_.advertise<std_msgs::Bool>(stop_car_topic_, 1, true);
    pub_final_stop_car_ =
        nh_.advertise<std_msgs::Bool>(final_stop_car_topic_, 1, true);
    pub_status_ = nh_.advertise<util::LocalizationStatus>(status_topic_, 1, true);
    // Map commands are rare; a non-persistent client avoids retaining a stale
    // TCPROS connection after an independently recovered fusion process.
    map_client_ = nh_.serviceClient<util::ManageLocalizationMap>(map_service_name_, false);
    sub_android_ = nh_.subscribe(android_topic_, 10, &LocalizationManager::AndroidCallback, this);
    sub_gps_ = nh_.subscribe(gps_topic_, 10, &LocalizationManager::GpsCallback, this);
    sub_position_ = nh_.subscribe(fusion_topic_, 20, &LocalizationManager::PositionCallback, this);
    sub_reboot_ = nh_.subscribe(reboot_topic_, 1, &LocalizationManager::RebootCallback, this);
    sub_lio_ = nh_.subscribe("/Mower/lio_slam", 20, &LocalizationManager::LioCallback, this);
    sub_lio_static_ = nh_.subscribe(lio_static_status_topic_, 1, &LocalizationManager::LioStaticCallback, this);
    sub_imu_ready_ = nh_.subscribe(imu_odometry_ready_topic_, 1, &LocalizationManager::ImuReadyCallback, this);
    sub_algorithm_status_ = nh_.subscribe(algorithm_status_topic_, 10, &LocalizationManager::AlgorithmStatusCallback, this);
    sub_fusion_stop_car_ = nh_.subscribe(
        fusion_stop_car_topic_, 10,
        &LocalizationManager::FusionStopCallback, this);
    watchdog_timer_ = nh_.createTimer(ros::Duration(0.20), &LocalizationManager::WatchdogTimer, this);
    status_timer_ = nh_.createTimer(ros::Duration(0.20), &LocalizationManager::StatusTimer, this);

    state_.phase = util::LocalizationStatus::PHASE_WAIT_FAST_LIO;
    state_.transitioning = true;
    state_.stop_required = true;
    state_.reason = "localization manager startup";
    startup_steady_sec_ = ros::SteadyTime::now().toSec();
    transition_wait_start_sec_ = startup_steady_sec_;
    std::array<char, 36> ignored_map_data{};
    canonical_map_valid_ =
        ReadValidMapOrigin(map_path_, &ignored_map_data, &canonical_map_error_);
    PublishStop(true);
    PublishStatus();
    ROS_WARN("Localization lifecycle startup: keep stopped until readiness "
             "and %d valid position frames.", kResumeFrames);
  }
  ~LocalizationManager() {
    if (restart_worker_.joinable()) restart_worker_.join();
    if (map_worker_.joinable()) map_worker_.join();
  }

private:
  struct ManagerState {
    uint8_t requested_mode = util::LocalizationStatus::MODE_UNKNOWN;
    uint8_t active_mode = util::LocalizationStatus::MODE_UNKNOWN;
    uint8_t workflow_mode = util::LocalizationStatus::WORKFLOW_UNKNOWN;
    uint8_t requested_runtime_mode = util::LocalizationStatus::RUNTIME_UNKNOWN;
    uint8_t active_runtime_mode = util::LocalizationStatus::RUNTIME_UNKNOWN;
    uint8_t phase = util::LocalizationStatus::PHASE_UNKNOWN;
    bool transitioning = false;
    bool restart_in_progress = false;
    uint8_t restart_scope = util::LocalizationStatus::RESTART_NONE;
    bool watchdog_position_timeout = false;
    bool watchdog_low_frequency = false;
    bool automatic_restart_suspended = false;
    bool stop_required = true;
    uint16_t fault_code = util::LocalizationStatus::FAULT_NONE;
    std::string active_map, pending_map, last_created_map, reason;
    std::string active_map_uuid, pending_map_uuid, coordinate_frame;
    std::string origin_checksum;
    uint32_t map_schema_version = 0;
    bool map_origin_committed = false;
  };

  void PublishFinalStop() {
    std_msgs::Bool msg;
    msg.data = android_manager_4dof::AggregateLocalizationStop(
        manager_stop_required_.load(), fusion_stop_required_.load());
    pub_final_stop_car_.publish(msg);
  }
  void PublishStop(bool stop) {
    manager_stop_required_.store(stop);
    std_msgs::Bool msg;
    msg.data = stop;
    pub_manager_stop_car_.publish(msg);
    PublishFinalStop();
  }
  void FusionStopCallback(const std_msgs::BoolConstPtr &msg) {
    if (!msg) return;
    fusion_stop_required_.store(msg->data);
    PublishFinalStop();
  }
  void ResetMonitoringLocked() {
    valid_position_count_ = resume_position_count_ = 0;
    frequency_window_start_sec_ = 0.0;
    low_frequency_window_count_ = 0;
    frequency_armed_ = false;
    last_position_rx_sec_ = last_lio_rx_sec_ = last_lio_good_rx_sec_ = 0.0;
    last_position_measurement_stamp_ = last_lio_measurement_stamp_ = 0.0;
  }
  void SetFaultLocked(uint16_t code, const std::string &reason) {
    state_.phase = util::LocalizationStatus::PHASE_FAULT;
    state_.transitioning = state_.restart_in_progress = false;
    state_.restart_scope = util::LocalizationStatus::RESTART_NONE;
    state_.stop_required = true;
    state_.fault_code = code;
    state_.reason = reason;
    restart_verification_deadline_sec_ = 0.0;
    restart_generation_verification_required_ = false;
    restart_new_generation_seen_ = false;
    restart_generation_accepted_status_stamp_ = ros::Time(0);
  }
  bool EnterAlgorithmHeartbeatWaitLocked(const std::string &reason) {
    if (state_.transitioning || state_.restart_in_progress ||
        state_.fault_code != util::LocalizationStatus::FAULT_NONE)
      return false;
    state_.transitioning = true;
    state_.phase = util::LocalizationStatus::PHASE_WAIT_ALGORITHM;
    state_.stop_required = true;
    state_.reason = reason;
    state_.watchdog_position_timeout = true;
    transition_wait_start_sec_ = ros::SteadyTime::now().toSec();
    position_recovery_wait_ = true;
    resume_position_count_ = 0;
    return true;
  }
  void ResetAutomaticRestartGuardLocked() {
    automatic_restart_times_.clear();
    state_.automatic_restart_suspended = false;
  }

  void ClearPendingRecoveryLocked() {
    pending_recovery_scope_ = util::LocalizationStatus::RESTART_NONE;
    pending_recovery_runtime_ = RecoveryRuntime::kUnknown;
    pending_recovery_reason_.clear();
    pending_recovery_revalidate_after_lio_ = false;
    pending_recovery_lio_baseline_ = GenerationToken{};
  }

  void QueuePendingRecoveryLocked(uint8_t scope,
                                  RecoveryRuntime target_runtime,
                                  const std::string &reason) {
    if (pending_recovery_scope_ == util::LocalizationStatus::RESTART_NONE) {
      pending_recovery_scope_ = scope;
      pending_recovery_runtime_ = target_runtime;
      pending_recovery_reason_ = reason;
    } else if (pending_recovery_scope_ != scope) {
      // Different partial recoveries are not substitutes for each other.
      // Coalesce them into a full recovery; RESTART_ALL always dominates.
      pending_recovery_scope_ = util::LocalizationStatus::RESTART_ALL;
      if (scope == util::LocalizationStatus::RESTART_ALL)
        pending_recovery_runtime_ = target_runtime;
      else if (!IsKnownFusionRuntime(pending_recovery_runtime_))
        pending_recovery_runtime_ = AutomaticRecoveryRuntimeLocked();
      pending_recovery_reason_ = "coalesced automatic recovery: " +
                                 pending_recovery_reason_ + "; " + reason;
    }
    if (pending_recovery_scope_ == util::LocalizationStatus::RESTART_ALL &&
        state_.restart_scope == util::LocalizationStatus::RESTART_LIO &&
        lio_restart_baseline_.valid) {
      pending_recovery_revalidate_after_lio_ = true;
      pending_recovery_lio_baseline_ = lio_restart_baseline_;
    }
    ROS_WARN("Queue overlapping automatic localization recovery: scope=%u, %s",
             static_cast<unsigned>(pending_recovery_scope_),
             pending_recovery_reason_.c_str());
  }
  bool PermitAutomaticRestartLocked(const std::string &reason) {
    if (state_.automatic_restart_suspended) return false;
    const double now = ros::SteadyTime::now().toSec();
    while (!automatic_restart_times_.empty() &&
           now - automatic_restart_times_.front() > kAutomaticRestartWindowSec)
      automatic_restart_times_.pop_front();
    if (automatic_restart_times_.size() >= kMaxAutomaticRestarts) {
      state_.automatic_restart_suspended = true;
      SetFaultLocked(util::LocalizationStatus::FAULT_RESTART_SUPPRESSED,
                     "automatic restart suppressed: " + reason);
      return false;
    }
    automatic_restart_times_.push_back(now);
    return true;
  }

  RecoveryRuntime AutomaticRecoveryRuntimeLocked() const {
    // During one live manager session we know whether this mapping workflow
    // has durably committed its origin. Before that point it must restart the
    // create runtime; afterwards it intentionally tracks the saved origin.
    // Proving the same fact across a manager restart needs the future persisted
    // mapping_session_id and remains an explicit TODO.
    if (state_.workflow_mode == util::LocalizationStatus::WORKFLOW_UNKNOWN)
      return state_.active_runtime_mode ==
                     util::LocalizationStatus::RUNTIME_CREATE_ORIGIN
                 ? RecoveryRuntime::kCreateOrigin
                 : RecoveryRuntime::kTrackSavedOrigin;
    return android_manager_4dof::SelectAutomaticRecoveryRuntime(
        state_.workflow_mode == util::LocalizationStatus::WORKFLOW_MAPPING,
        state_.map_origin_committed);
  }

  bool BeginRestart(uint8_t scope, RecoveryRuntime target_runtime,
                    const std::string &reason, bool automatic,
                    uint8_t workflow_mode =
                        util::LocalizationStatus::WORKFLOW_UNKNOWN,
                    const std::string &pending_map = std::string()) {
    const bool fusion_restart =
        scope == util::LocalizationStatus::RESTART_FUSION ||
        scope == util::LocalizationStatus::RESTART_ALL;
    const bool lio_only_restart =
        scope == util::LocalizationStatus::RESTART_LIO;
    // A fusion restart must name its runtime explicitly.  Conversely, a
    // LIO-only recovery has no fusion runtime target and must use kUnknown as
    // an intentional "not applicable" value.  Reject every other combination
    // before consuming restart budget or changing workflow state.
    if ((!fusion_restart && !lio_only_restart) ||
        (fusion_restart && !IsKnownFusionRuntime(target_runtime)) ||
        (lio_only_restart && target_runtime != RecoveryRuntime::kUnknown)) {
      const std::string error =
          "invalid restart request: scope=" + std::to_string(scope) +
          ", target_runtime=" + RuntimeName(target_runtime);
      {
        std::lock_guard<std::mutex> lock(mutex_);
        SetFaultLocked(util::LocalizationStatus::FAULT_RESTART_FAILED, error);
      }
      ROS_ERROR("%s", error.c_str());
      PublishStop(true);
      PublishStatus();
      return false;
    }

    std::thread completed_worker;
    RestartBaseline restart_baseline;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      const bool replacing_verified_but_unusable_generation =
          automatic && state_.transitioning && !state_.restart_in_progress &&
          !restart_worker_running_ && !map_worker_running_ &&
          restart_generation_verification_required_ &&
          restart_new_generation_seen_ && position_recovery_wait_;
      if (!replacing_verified_but_unusable_generation &&
          (state_.restart_in_progress || restart_worker_running_ ||
          map_worker_running_ ||
          (state_.transitioning && restart_generation_verification_required_))) {
        if (automatic) {
          // Repeated requests from the process being replaced are not new
          // recovery events. A different scope can still be queued.
          if (scope == state_.restart_scope) return true;
          QueuePendingRecoveryLocked(scope, target_runtime, reason);
          return true;
        }
        ROS_WARN("Ignore overlapping localization restart: %s", reason.c_str());
        return false;
      }
      if (state_.transitioning && !state_.pending_map.empty() && !automatic) {
        ROS_WARN("Reject explicit restart while a localization transition is active: %s",
                 reason.c_str());
        return false;
      }
      if (automatic && !PermitAutomaticRestartLocked(reason)) {
        PublishStop(true);
        return false;
      }
      if (!automatic) {
        ResetAutomaticRestartGuardLocked();
        ClearPendingRecoveryLocked();
      }
      if (restart_worker_.joinable()) completed_worker = std::move(restart_worker_);
      if (have_algorithm_status_) {
        restart_baseline.fusion = {
            algorithm_status_.fusion_generation_valid != 0,
            algorithm_status_.fusion_generation_sec,
            algorithm_status_.fusion_generation_nsec};
        restart_baseline.lio = {
            algorithm_status_.lio_generation_valid != 0,
            algorithm_status_.lio_generation_sec,
            algorithm_status_.lio_generation_nsec};
      }
      if (lio_only_restart)
        lio_restart_baseline_ = restart_baseline.lio;
      restart_generation_baseline_ = restart_baseline;
      const ros::WallTime request_wall_time = ros::WallTime::now();
      restart_generation_request_floor_ = {
          true, request_wall_time.sec, request_wall_time.nsec};
      restart_generation_scope_ = scope;
      restart_generation_verification_required_ = true;
      restart_new_generation_seen_ = false;
      restart_generation_accepted_status_stamp_ = ros::Time(0);
      // Readiness belongs to the process generation being replaced. Bool
      // callback ordering must never carry a previous true into the new one.
      fast_lio_static_ready_ = false;
      imu_odometry_ready_ = false;
      if (workflow_mode != util::LocalizationStatus::WORKFLOW_UNKNOWN) {
        state_.workflow_mode = workflow_mode;
        if (workflow_mode == util::LocalizationStatus::WORKFLOW_MAPPING)
          state_.map_origin_committed = false;
      }
      if (IsKnownFusionRuntime(target_runtime)) {
        state_.requested_mode = LegacyModeForRuntime(target_runtime);
        state_.requested_runtime_mode = RuntimeStatusValue(target_runtime);
      }
      if (!pending_map.empty() || !automatic) {
        state_.pending_map = pending_map;
        if (pending_map.empty()) state_.pending_map_uuid.clear();
      }
      state_.transitioning = state_.restart_in_progress = true;
      // A fusion-only restart has a bounded chance to recover. If its verified
      // new generation still produces no fresh position, the watchdog must be
      // allowed to escalate to RESTART_ALL instead of waiting forever.
      position_recovery_wait_ =
          scope == util::LocalizationStatus::RESTART_FUSION;
      transition_wait_start_sec_ = 0.0;
      restart_completed_ros_time_ = ros::Time(0);
      state_.restart_scope = scope;
      state_.stop_required = true;
      state_.fault_code = util::LocalizationStatus::FAULT_NONE;
      state_.reason = reason;
      state_.phase = scope == util::LocalizationStatus::RESTART_FUSION
                         ? util::LocalizationStatus::PHASE_RESTARTING_FUSION
                         : scope == util::LocalizationStatus::RESTART_LIO
                               ? util::LocalizationStatus::PHASE_RECOVERING_LIO
                               : util::LocalizationStatus::PHASE_RESTARTING_ALL;
      if (!pending_map.empty()) state_.phase = util::LocalizationStatus::PHASE_SWITCHING_MAP;
      restart_worker_running_ = true;
      restart_verification_deadline_sec_ = 0.0;
      ResetMonitoringLocked();
    }
    if (completed_worker.joinable()) completed_worker.join();
    PublishStop(true);
    PublishStatus();
    try {
      restart_worker_ = std::thread(&LocalizationManager::RestartWorker, this,
                                    scope, target_runtime, reason);
    } catch (const std::system_error &error) {
      {
        std::lock_guard<std::mutex> lock(mutex_);
        restart_worker_running_ = false;
        SetFaultLocked(util::LocalizationStatus::FAULT_RESTART_FAILED,
                       "unable to create restart worker: " +
                           std::string(error.what()));
      }
      PublishStop(true);
      PublishStatus();
      return false;
    }
    return true;
  }

  void StartPendingRecoveryIfIdle() {
    uint8_t scope = util::LocalizationStatus::RESTART_NONE;
    RecoveryRuntime runtime = RecoveryRuntime::kUnknown;
    std::string reason;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (pending_recovery_scope_ == util::LocalizationStatus::RESTART_NONE ||
          state_.restart_in_progress || restart_worker_running_ ||
          map_worker_running_ ||
          (state_.transitioning && restart_generation_verification_required_))
        return;
      if (pending_recovery_revalidate_after_lio_ &&
          pending_recovery_scope_ == util::LocalizationStatus::RESTART_ALL) {
        const double now = ros::SteadyTime::now().toSec();
        const GenerationToken current_generation{
            algorithm_status_.lio_generation_valid != 0,
            algorithm_status_.lio_generation_sec,
            algorithm_status_.lio_generation_nsec};
        android_manager_4dof::RecoveryHealthEvidence evidence;
        evidence.algorithm_status_fresh =
            have_algorithm_status_ &&
            now - last_algorithm_status_rx_sec_ <=
                kAlgorithmStatusTimeoutSec;
        evidence.algorithm_fault_free =
            algorithm_status_.fault_code ==
            util::LocalizationStatus::FAULT_NONE;
        evidence.algorithm_restart_idle =
            algorithm_status_.restart_scope ==
            util::LocalizationStatus::RESTART_NONE;
        evidence.fast_lio_initialized =
            algorithm_status_.fast_lio_initialized;
        evidence.algorithm_lio_good = algorithm_status_.lio_good;
        evidence.alignment_ready = algorithm_status_.alignment_ready;
        evidence.map_valid = algorithm_status_.map_valid;
        evidence.position_valid = algorithm_status_.position_valid;
        evidence.manager_lio_stream_fresh =
            last_lio_rx_sec_ > 0.0 &&
            now - last_lio_rx_sec_ <= kLioAliveTimeoutSec;
        evidence.manager_lio_quality_good =
            last_lio_good_rx_sec_ > 0.0 &&
            now - last_lio_good_rx_sec_ <= kLioAliveTimeoutSec;
        evidence.manager_lio_static_ready = fast_lio_static_ready_;
        const bool stale = android_manager_4dof::QueuedFullRecoveryIsStale(
            pending_recovery_lio_baseline_, current_generation, evidence);
        if (stale) {
          ROS_WARN("Cancel stale queued full recovery: LIO-only recovery "
                   "produced a new, fully healthy generation.");
          ClearPendingRecoveryLocked();
          full_recovery_request_consumed_ = false;
          return;
        }
      }
      scope = pending_recovery_scope_;
      runtime = pending_recovery_runtime_;
      reason = pending_recovery_reason_;
      ClearPendingRecoveryLocked();
    }
    // Called only from the ROS callback thread, never from restart_worker_:
    // BeginRestart can therefore safely join the completed worker object.
    if (!BeginRestart(scope, runtime, reason, true))
      ROS_ERROR("Unable to start queued localization recovery: %s",
                reason.c_str());
  }

  std::string RestartCommand(uint8_t scope,
                             RecoveryRuntime target_runtime) const {
    if (scope == util::LocalizationStatus::RESTART_FUSION) {
      if (!IsKnownFusionRuntime(target_runtime)) return std::string();
      const char *location_mode = target_runtime == RecoveryRuntime::kCreateOrigin
                                      ? "false"
                                      : "true";
      return std::string("rosrun ") + fusion_package_ +
             " restart_ros_nodes.sh /fusion_4dof -- roslaunch " +
             fusion_package_ + " fusion_only.launch location_mode:=" +
             location_mode;
    }
    if (scope == util::LocalizationStatus::RESTART_LIO &&
        target_runtime == RecoveryRuntime::kUnknown)
      return std::string("rosrun ") + fusion_package_ +
             " restart_ros_nodes.sh /laserMapping -- roslaunch " +
             fusion_package_ + " laserMapping_only.launch";
    if (scope == util::LocalizationStatus::RESTART_ALL &&
        IsKnownFusionRuntime(target_runtime)) {
      const char *launch_file = target_runtime == RecoveryRuntime::kCreateOrigin
                                    ? "create_mode_4dof.launch"
                                    : "location_mode_4dof.launch";
      return std::string("rosrun ") + fusion_package_ +
             " restart_ros_nodes.sh /fusion_4dof /laserMapping -- roslaunch " +
             fusion_package_ + " " + launch_file;
    }
    return std::string();
  }
  void RestartWorker(uint8_t scope, RecoveryRuntime target_runtime,
                     const std::string &reason) {
    const std::string command = RestartCommand(scope, target_runtime);
    const int result = command.empty() ? -1 : std::system(command.c_str());
    const int exit_code = result >= 0 && WIFEXITED(result)
                              ? WEXITSTATUS(result)
                              : -1;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      restart_worker_running_ = false;
      state_.restart_in_progress = false;
      if (command.empty() || result == -1 || exit_code != 0) {
        SetFaultLocked(util::LocalizationStatus::FAULT_RESTART_FAILED,
                       "restart wrapper could not be launched (exit=" +
                           std::to_string(exit_code) + "): " + reason);
      } else {
        // A wrapper exit code describes command execution, not whether the
        // requested ROS processes changed generation. Bound only that process
        // identity proof; sensor/operator-dependent readiness has no hard
        // deadline and keeps the stop latched.
        state_.stop_required = true;
        state_.fault_code = util::LocalizationStatus::FAULT_NONE;
        transition_wait_start_sec_ = ros::SteadyTime::now().toSec();
        // Readiness must be proven by a status heartbeat emitted after the
        // wrapper completed its node registration/reachability checks.
        restart_completed_ros_time_ = ros::Time::now();
        restart_verification_deadline_sec_ = restart_new_generation_seen_
            ? 0.0
            : transition_wait_start_sec_ + restart_generation_timeout_sec_;
        if (restart_new_generation_seen_) {
          // The status callback can prove the new generations while the
          // wrapper is still checking ROS registration/reachability.
          state_.phase = algorithm_status_.phase;
          state_.reason = algorithm_status_.reason.empty()
                              ? "new process generation(s) verified; waiting "
                                "for localization readiness"
                              : algorithm_status_.reason;
        } else {
          state_.phase = util::LocalizationStatus::PHASE_RESTART_VERIFYING;
          state_.reason =
              "restart wrapper exit=" + std::to_string(exit_code) +
              "; verifying expected process generation(s)";
        }
      }
    }
    PublishStatus();
  }

  bool AlgorithmReadyForResumeLocked() const {
    const bool fresh = have_algorithm_status_ &&
        ros::SteadyTime::now().toSec() - last_algorithm_status_rx_sec_ <=
            kAlgorithmStatusTimeoutSec;
    const bool post_restart_status = restart_completed_ros_time_.isZero() ||
        (!algorithm_status_.header.stamp.isZero() &&
         algorithm_status_.header.stamp >= restart_completed_ros_time_);
    const bool map_identity_ready = state_.pending_map.empty() ||
        (algorithm_status_.active_map == state_.pending_map &&
         (state_.pending_map_uuid.empty() ||
          algorithm_status_.active_map_uuid == state_.pending_map_uuid));
    const GenerationToken current_fusion{
        algorithm_status_.fusion_generation_valid != 0,
        algorithm_status_.fusion_generation_sec,
        algorithm_status_.fusion_generation_nsec};
    const GenerationToken current_lio{
        algorithm_status_.lio_generation_valid != 0,
        algorithm_status_.lio_generation_sec,
        algorithm_status_.lio_generation_nsec};
    const bool generations_verified =
        !restart_generation_verification_required_ ||
        (restart_new_generation_seen_ &&
         android_manager_4dof::ExpectedRestartGenerationsObserved(
             restart_generation_scope_ ==
                     util::LocalizationStatus::RESTART_FUSION ||
                 restart_generation_scope_ ==
                     util::LocalizationStatus::RESTART_ALL,
             restart_generation_scope_ ==
                     util::LocalizationStatus::RESTART_LIO ||
                 restart_generation_scope_ ==
                     util::LocalizationStatus::RESTART_ALL,
             restart_generation_baseline_.fusion, current_fusion,
             restart_generation_baseline_.lio, current_lio,
             restart_generation_request_floor_));
    return fresh && post_restart_status && map_identity_ready &&
        generations_verified &&
        state_.fault_code == util::LocalizationStatus::FAULT_NONE &&
        algorithm_status_.fast_lio_initialized &&
        algorithm_status_.map_valid && algorithm_status_.alignment_ready &&
        algorithm_status_.restart_scope ==
            util::LocalizationStatus::RESTART_NONE &&
        algorithm_status_.position_valid &&
        algorithm_status_.fault_code == util::LocalizationStatus::FAULT_NONE &&
        !algorithm_status_.stop_required &&
        !algorithm_status_.transitioning &&
        (!require_imu_odometry_ready_ ||
         algorithm_status_.imu_odometry_ready);
  }

  void CompleteTransitionIfReadyLocked() {
    if (!state_.transitioning || state_.restart_in_progress ||
        map_worker_running_) return;
    if (!AlgorithmReadyForResumeLocked() ||
        resume_position_count_ < kResumeFrames) return;
    state_.active_mode = algorithm_status_.active_mode;
    state_.active_runtime_mode = algorithm_status_.active_runtime_mode;
    if (!state_.pending_map.empty()) {
      state_.active_map = algorithm_status_.active_map;
      state_.active_map_uuid = algorithm_status_.active_map_uuid;
      state_.pending_map.clear();
      state_.pending_map_uuid.clear();
    }
    state_.map_schema_version = algorithm_status_.map_schema_version;
    state_.coordinate_frame = algorithm_status_.coordinate_frame;
    state_.origin_checksum = algorithm_status_.origin_checksum;
    state_.phase = algorithm_status_.phase;
    state_.transitioning = false;
    position_recovery_wait_ = false;
    transition_wait_start_sec_ = 0.0;
    restart_verification_deadline_sec_ = 0.0;
    restart_generation_verification_required_ = false;
    restart_new_generation_seen_ = false;
    restart_generation_accepted_status_stamp_ = ros::Time(0);
    restart_completed_ros_time_ = ros::Time(0);
    state_.restart_scope = util::LocalizationStatus::RESTART_NONE;
    state_.stop_required = false;
    state_.fault_code = util::LocalizationStatus::FAULT_NONE;
    state_.watchdog_position_timeout = state_.watchdog_low_frequency = false;
    state_.reason = "localization transition complete";
    // Do not let frames/time accumulated while stopped contaminate the normal
    // low-frequency watchdog window. Preserve the latest position/LIO receipt
    // timestamps for outage detection.
    valid_position_count_ = 0;
    frequency_window_start_sec_ = 0.0;
    low_frequency_window_count_ = 0;
    frequency_armed_ = false;
    PublishStop(false);
    ROS_WARN("Localization transition complete; manager stop released.");
  }

  bool StartMapTransaction(uint8_t command, const std::string &map_name) {
    std::thread completed_worker;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (state_.restart_in_progress || restart_worker_running_ ||
          map_worker_running_ || state_.transitioning) {
        ROS_WARN("Reject map transaction '%s' while another transition is active.",
                 map_name.c_str());
        return false;
      }
      if (map_worker_.joinable()) completed_worker = std::move(map_worker_);
      ResetAutomaticRestartGuardLocked();
      state_.requested_mode = util::LocalizationStatus::MODE_LOCALIZATION;
      state_.pending_map = map_name;
      state_.pending_map_uuid.clear();
      state_.transitioning = true;
      state_.phase = util::LocalizationStatus::PHASE_SWITCHING_MAP;
      state_.stop_required = true;
      state_.fault_code = util::LocalizationStatus::FAULT_NONE;
      state_.reason = command == util::ManageLocalizationMap::Request::SAVE_CURRENT_AND_ACTIVATE
                          ? "saving and immediately activating map " + map_name
                          : "hot-switching to map " + map_name;
      transition_wait_start_sec_ = ros::SteadyTime::now().toSec();
      position_recovery_wait_ = false;
      map_worker_running_ = true;
      ResetMonitoringLocked();
    }
    if (completed_worker.joinable()) completed_worker.join();
    PublishStop(true);
    PublishStatus();
    try {
      map_worker_ = std::thread(&LocalizationManager::MapTransactionWorker,
                                this, command, map_name);
    } catch (const std::system_error &exception) {
      std::lock_guard<std::mutex> lock(mutex_);
      map_worker_running_ = false;
      SetFaultLocked(util::LocalizationStatus::FAULT_ALGORITHM,
                     "unable to create map transaction worker: " +
                         std::string(exception.what()));
      return false;
    }
    return true;
  }

  void MapTransactionWorker(uint8_t command, const std::string &map_name) {
    util::ManageLocalizationMap service;
    service.request.command = command;
    service.request.map_name = map_name;
    service.request.overwrite = false;
    bool transport_ok = map_client_.waitForExistence(ros::Duration(2.0)) &&
                        map_client_.call(service);
    {
      std::lock_guard<std::mutex> lock(mutex_);
      map_worker_running_ = false;
      if (!transport_ok || !service.response.success) {
        const std::string reason = transport_ok
            ? service.response.reason
            : "fusion map service unavailable or transport failed";
        SetFaultLocked(transport_ok ? util::LocalizationStatus::FAULT_MAP_INVALID
                                    : util::LocalizationStatus::FAULT_ALGORITHM,
                       "map transaction failed: " + reason);
      } else {
        canonical_map_valid_ = true;
        canonical_map_error_.clear();
        state_.requested_mode = util::LocalizationStatus::MODE_LOCALIZATION;
        state_.active_mode = util::LocalizationStatus::MODE_LOCALIZATION;
        state_.workflow_mode = util::LocalizationStatus::WORKFLOW_LOCALIZATION;
        state_.requested_runtime_mode =
            util::LocalizationStatus::RUNTIME_TRACK_SAVED_ORIGIN;
        state_.active_runtime_mode =
            util::LocalizationStatus::RUNTIME_TRACK_SAVED_ORIGIN;
        state_.active_map = service.response.active_map;
        state_.active_map_uuid = service.response.active_map_uuid;
        state_.pending_map = service.response.switch_pending
                                 ? service.response.pending_map : map_name;
        state_.pending_map_uuid = service.response.switch_pending
                                      ? service.response.pending_map_uuid
                                      : service.response.active_map_uuid;
        state_.map_schema_version = service.response.map_schema_version;
        state_.coordinate_frame = service.response.coordinate_frame;
        state_.origin_checksum = service.response.origin_checksum;
        state_.map_origin_committed = true;
        if (command == util::ManageLocalizationMap::Request::SAVE_CURRENT_AND_ACTIVATE)
          state_.last_created_map = map_name;
        state_.phase = util::LocalizationStatus::PHASE_SWITCHING_MAP;
        state_.reason = service.response.reason;
        transition_wait_start_sec_ = ros::SteadyTime::now().toSec();
        restart_completed_ros_time_ = ros::Time(0);
        resume_position_count_ = 0;
      }
    }
    PublishStatus();
  }

  void AndroidCallback(const std_msgs::StringConstPtr &msg) {
    if (!msg) return;
    std::string verb, argument;
    if (!SplitCommand(msg->data, &verb, &argument)) return;
    if (verb == "app_start") { PublishStatus(); return; }
    if (verb == "p_mode") {
      std::lock_guard<std::mutex> lock(mutex_);
      // p_mode is only an acknowledgement/query. Resetting watchdog baselines
      // here can erase the last valid-position time and permanently mask an
      // outage that begins at the same moment.
      state_.reason = "p_mode acknowledged; localization mode unchanged";
      return;
    }
    if (verb == "save_map") {
      if (!IsSafeMapName(argument)) { ROS_ERROR("Reject invalid map name '%s'.", argument.c_str()); return; }
      StartMapTransaction(
          util::ManageLocalizationMap::Request::SAVE_CURRENT_AND_ACTIVATE,
          argument);
      return;
    }
    if (verb == "use_map") {
      if (!IsSafeMapName(argument)) { ROS_ERROR("Reject invalid map name '%s'.", argument.c_str()); return; }
      StartMapTransaction(
          util::ManageLocalizationMap::Request::ACTIVATE_EXISTING, argument);
      return;
    }
    if (verb == "m_mode") {
      BeginRestart(util::LocalizationStatus::RESTART_ALL,
                   RecoveryRuntime::kCreateOrigin,
                   "mapping mode requested", false,
                   util::LocalizationStatus::WORKFLOW_MAPPING); return;
    }
    if (verb == "true") {
      BeginRestart(util::LocalizationStatus::RESTART_ALL,
                   RecoveryRuntime::kTrackSavedOrigin,
                   "localization mode requested", false,
                   util::LocalizationStatus::WORKFLOW_LOCALIZATION); return;
    }
    ROS_ERROR("Unknown localization command '%s'.", verb.c_str());
  }
  void EnterMapFault(const std::string &error) {
    ROS_ERROR("%s", error.c_str());
    { std::lock_guard<std::mutex> lock(mutex_); SetFaultLocked(util::LocalizationStatus::FAULT_MAP_INVALID, error); }
    PublishStop(true); PublishStatus();
  }
  void RebootCallback(const std_msgs::BoolConstPtr &msg) {
    if (msg && msg->data) {
      RecoveryRuntime target_runtime = RecoveryRuntime::kUnknown;
      {
        std::lock_guard<std::mutex> lock(mutex_);
        if (full_recovery_request_consumed_ ||
            state_.restart_scope == util::LocalizationStatus::RESTART_ALL ||
            pending_recovery_scope_ == util::LocalizationStatus::RESTART_ALL)
          return;
        // Bool has no process identity. A delayed/latching reboot Bool from
        // the replaced fusion must not restart an already healthy generation.
        // The generation-tagged status is the authoritative request; if it
        // has not arrived yet, its callback will start the same recovery.
        if (!have_algorithm_status_ ||
            (algorithm_status_.restart_scope !=
                 util::LocalizationStatus::RESTART_ALL &&
             algorithm_status_.fault_code ==
                 util::LocalizationStatus::FAULT_NONE))
          return;
        target_runtime = AutomaticRecoveryRuntimeLocked();
      }
      const bool accepted =
          BeginRestart(util::LocalizationStatus::RESTART_ALL, target_runtime,
                       "fusion algorithm FAULT", true);
      if (accepted) {
        std::lock_guard<std::mutex> lock(mutex_);
        // Mark consumed only after the request was started or durably queued.
        full_recovery_request_consumed_ = true;
      }
    }
  }
  void AlgorithmStatusCallback(const util::LocalizationStatusConstPtr &msg) {
    if (!msg) return;
    if (!msg->algorithm_status_valid || msg->header.stamp.isZero() ||
        !msg->fusion_generation_valid ||
        (msg->position_valid &&
         (!IsProductionPosition(msg->position_state) ||
          !android_manager_4dof::UsablePositionInvariant(
              true, msg->stop_required, msg->transitioning,
              msg->fault_code, msg->restart_scope))) ||
        !android_manager_4dof::RequiredStopInvariant(
            msg->stop_required, msg->transitioning, msg->fault_code,
            msg->restart_scope)) {
      ROS_ERROR_THROTTLE(1.0, "Reject incoherent localization algorithm "
                              "status contract.");
      return;
    }
    uint8_t requested_scope = util::LocalizationStatus::RESTART_NONE;
    RecoveryRuntime requested_runtime = RecoveryRuntime::kUnknown;
    bool request_stop = false;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (have_algorithm_status_) {
        const GenerationToken previous_fusion{
            algorithm_status_.fusion_generation_valid != 0,
            algorithm_status_.fusion_generation_sec,
            algorithm_status_.fusion_generation_nsec};
        const GenerationToken incoming_fusion{
            msg->fusion_generation_valid != 0, msg->fusion_generation_sec,
            msg->fusion_generation_nsec};
        const GenerationToken previous_lio{
            algorithm_status_.lio_generation_valid != 0,
            algorithm_status_.lio_generation_sec,
            algorithm_status_.lio_generation_nsec};
        const GenerationToken incoming_lio{
            msg->lio_generation_valid != 0, msg->lio_generation_sec,
            msg->lio_generation_nsec};
        if (android_manager_4dof::GenerationRegressed(previous_fusion,
                                                      incoming_fusion) ||
            android_manager_4dof::GenerationRegressed(previous_lio,
                                                      incoming_lio))
          return;
      }
      bool unexpected_generation_change = false;
      if (have_algorithm_status_ &&
          !restart_generation_verification_required_) {
        const GenerationToken previous_fusion{
            algorithm_status_.fusion_generation_valid != 0,
            algorithm_status_.fusion_generation_sec,
            algorithm_status_.fusion_generation_nsec};
        const GenerationToken incoming_fusion{
            msg->fusion_generation_valid != 0, msg->fusion_generation_sec,
            msg->fusion_generation_nsec};
        const GenerationToken previous_lio{
            algorithm_status_.lio_generation_valid != 0,
            algorithm_status_.lio_generation_sec,
            algorithm_status_.lio_generation_nsec};
        const GenerationToken incoming_lio{
            msg->lio_generation_valid != 0, msg->lio_generation_sec,
            msg->lio_generation_nsec};
        unexpected_generation_change =
            android_manager_4dof::GenerationAdvanced(previous_fusion,
                                                      incoming_fusion) ||
            android_manager_4dof::GenerationAdvanced(previous_lio,
                                                      incoming_lio);
      }
      if (restart_generation_verification_required_) {
        const GenerationToken fusion_generation{
            msg->fusion_generation_valid != 0, msg->fusion_generation_sec,
            msg->fusion_generation_nsec};
        const GenerationToken lio_generation{
            msg->lio_generation_valid != 0, msg->lio_generation_sec,
            msg->lio_generation_nsec};
        if (!android_manager_4dof::ExpectedRestartGenerationsObserved(
                restart_generation_scope_ == util::LocalizationStatus::RESTART_FUSION ||
                    restart_generation_scope_ == util::LocalizationStatus::RESTART_ALL,
                restart_generation_scope_ == util::LocalizationStatus::RESTART_LIO ||
                    restart_generation_scope_ == util::LocalizationStatus::RESTART_ALL,
                restart_generation_baseline_.fusion, fusion_generation,
                restart_generation_baseline_.lio, lio_generation,
                restart_generation_request_floor_)) {
          // A queued/latched status from an old process is not authoritative
          // for fault, runtime, map, restart request, or readiness decisions.
          return;
        }
        if (!restart_new_generation_seen_) {
          restart_new_generation_seen_ = true;
          restart_generation_accepted_status_stamp_ = msg->header.stamp;
          // Process identity is now proven. Static LIO initialization, INS
          // dynamic initialization, alignment and valid output may legitimately
          // take much longer and must not trip FAULT_RESTART_PARTIAL.
          restart_verification_deadline_sec_ = 0.0;
          resume_position_count_ = 0;
          last_position_rx_sec_ = 0.0;
        }
      }
      algorithm_status_ = *msg;
      have_algorithm_status_ = true;
      last_algorithm_status_rx_sec_ = ros::SteadyTime::now().toSec();
      // These generation-tagged fields are authoritative. The legacy Bool
      // topics have no generation identity and may reconnect out of order.
      fast_lio_static_ready_ = msg->fast_lio_initialized;
      imu_odometry_ready_ = msg->imu_odometry_ready;
      const bool post_restart_status = restart_completed_ros_time_.isZero() ||
          (!msg->header.stamp.isZero() &&
           msg->header.stamp >= restart_completed_ros_time_);
      const bool controlled_lio_recovery =
          msg->phase == util::LocalizationStatus::PHASE_RECOVERING_LIO;
      if (state_.transitioning &&
          state_.phase == util::LocalizationStatus::PHASE_WAIT_ALGORITHM) {
        // Require the five release frames after, not during, the heartbeat
        // outage. A newly fresh status is the coherence boundary.
        resume_position_count_ = 0;
      }
      if (unexpected_generation_change && !state_.restart_in_progress &&
          state_.fault_code == util::LocalizationStatus::FAULT_NONE) {
        state_.transitioning = true;
        state_.stop_required = true;
        state_.fault_code = util::LocalizationStatus::FAULT_NONE;
        state_.phase = util::LocalizationStatus::PHASE_RESTART_VERIFYING;
        state_.reason = "external localization process generation changed; "
                        "revalidating readiness";
        transition_wait_start_sec_ = ros::SteadyTime::now().toSec();
        position_recovery_wait_ = false;
        ResetMonitoringLocked();
        request_stop = true;
      }
      if (state_.requested_mode == util::LocalizationStatus::MODE_UNKNOWN) state_.requested_mode = msg->active_mode;
      if (state_.active_mode == util::LocalizationStatus::MODE_UNKNOWN) state_.active_mode = msg->active_mode;
      // Never infer the user workflow from the fusion runtime after a manager
      // restart. A mapping workflow may legitimately run TRACK_SAVED_ORIGIN
      // after committing its origin, so that inference silently relabels an
      // unfinished mapping session as localization. Keep WORKFLOW_UNKNOWN
      // until an explicit mode/map command establishes user intent.
      // Do not let a latched frame from the process being killed overwrite the
      // target runtime or re-latch the previous mapping origin after m_mode.
      if (!state_.restart_in_progress && post_restart_status) {
        state_.requested_runtime_mode = msg->requested_runtime_mode;
        state_.active_runtime_mode = msg->active_runtime_mode;
        if (msg->map_origin_committed) state_.map_origin_committed = true;
      }
      if (!msg->active_map.empty() &&
          (state_.active_map.empty() ||
           (!state_.transitioning && !state_.restart_in_progress))) {
        state_.active_map = msg->active_map;
        state_.active_map_uuid = msg->active_map_uuid;
        state_.map_schema_version = msg->map_schema_version;
        state_.coordinate_frame = msg->coordinate_frame;
        state_.origin_checksum = msg->origin_checksum;
      }
      if (state_.pending_map.empty() && msg->transitioning &&
          !msg->pending_map.empty() && !state_.restart_in_progress) {
        state_.pending_map = msg->pending_map;
        state_.pending_map_uuid = msg->pending_map_uuid;
        state_.transitioning = true;
        state_.phase = util::LocalizationStatus::PHASE_SWITCHING_MAP;
        state_.stop_required = true;
        state_.reason = msg->reason;
        transition_wait_start_sec_ = ros::SteadyTime::now().toSec();
        resume_position_count_ = 0;
        request_stop = true;
      }
      if (msg->restart_scope == util::LocalizationStatus::RESTART_NONE) {
        // Do not re-arm a request while the accepted recovery is still queued
        // or active.  Otherwise a transient RESTART_NONE from the old process
        // can turn one queued full recovery into a duplicate second restart.
        const bool lio_recovery_owned =
            pending_recovery_scope_ == util::LocalizationStatus::RESTART_LIO ||
            state_.restart_scope == util::LocalizationStatus::RESTART_LIO;
        const bool full_recovery_owned =
            pending_recovery_scope_ == util::LocalizationStatus::RESTART_ALL ||
            state_.restart_scope == util::LocalizationStatus::RESTART_ALL;
        if (!lio_recovery_owned) lio_recovery_request_consumed_ = false;
        if (!full_recovery_owned) full_recovery_request_consumed_ = false;
      } else if (msg->restart_scope == util::LocalizationStatus::RESTART_LIO &&
                 !lio_recovery_request_consumed_ &&
                 post_restart_status) {
        requested_scope = util::LocalizationStatus::RESTART_LIO;
      } else if (msg->restart_scope == util::LocalizationStatus::RESTART_ALL &&
                 !full_recovery_request_consumed_ &&
                 post_restart_status) {
        requested_scope = util::LocalizationStatus::RESTART_ALL;
      }
      if (!state_.transitioning && !state_.restart_in_progress &&
          state_.fault_code == util::LocalizationStatus::FAULT_NONE) {
        // Manager-owned failures are sticky until an explicit command starts
        // a new transition. A healthy algorithm heartbeat must not erase a
        // failed map copy, failed restart, or restart-storm lockout.
        if (msg->fault_code != util::LocalizationStatus::FAULT_NONE) {
          // Fault dominates every waiting/transition state. Copying the code
          // and then handling position_valid=false used to create the
          // impossible internal tuple fault+transitioning, even though the
          // publication layer later hid it. Use the single fault entry point
          // so internal transition guards and the public tuple agree.
          SetFaultLocked(msg->fault_code,
                         msg->reason.empty()
                             ? "localization algorithm reported a fault"
                             : msg->reason);
          request_stop = true;
        } else {
          state_.phase = msg->phase;
          state_.reason = msg->reason;
          state_.stop_required = msg->stop_required;
        }
        if (msg->fault_code == util::LocalizationStatus::FAULT_NONE &&
            !msg->position_valid) {
          // This also covers simultaneous LIO+position loss, where the old
          // LioCallback-driven watchdog had no callback from which to stop.
          state_.transitioning = true;
          state_.stop_required = true;
          state_.watchdog_position_timeout = true;
          // A deliberate LIO-only restart owns this no-position interval.
          // Keep the stop latched, but do not feed the generic position
          // watchdog and cascade into fusion/full recovery.
          position_recovery_wait_ = !controlled_lio_recovery;
          if (controlled_lio_recovery)
            state_.phase = util::LocalizationStatus::PHASE_RECOVERING_LIO;
          state_.reason = msg->reason.empty()
                              ? "algorithm reports no valid production position"
                              : msg->reason;
          transition_wait_start_sec_ = ros::SteadyTime::now().toSec();
          resume_position_count_ = 0;
          request_stop = true;
        }
      } else if (state_.transitioning && !state_.restart_in_progress &&
                 !map_worker_running_ &&
                 state_.fault_code == util::LocalizationStatus::FAULT_NONE) {
        // Static initialization and alignment may legitimately wait without a
        // deadline. Once both are ready, WAIT_TRUSTED_OUTPUT is no longer an
        // operator-dependent wait: bound it so a full restart cannot become a
        // second permanent stale-timestamp state.
        if (android_manager_4dof::FreshOutputWatchdogEligible(
                msg->fast_lio_initialized, msg->map_valid,
                msg->alignment_ready, msg->position_valid,
                msg->phase ==
                    util::LocalizationStatus::PHASE_WAIT_TRUSTED_OUTPUT))
          position_recovery_wait_ = true;
        if (msg->fault_code != util::LocalizationStatus::FAULT_NONE) {
          SetFaultLocked(msg->fault_code,
                         msg->reason.empty()
                             ? "localization algorithm fault during transition"
                             : msg->reason);
          request_stop = true;
        } else if (state_.pending_map.empty()) {
          // Keep the wrapper's explicit partial-success phase visible while
          // generation-backed readiness verification is still running.
          if (restart_verification_deadline_sec_ <= 0.0) {
            state_.phase = msg->phase;
            state_.reason = msg->reason;
          }
          state_.stop_required = true;
        }
      }
      const bool only_waiting_for_imu_odometry =
          state_.transitioning && !state_.restart_in_progress &&
          !map_worker_running_ && state_.pending_map.empty() &&
          state_.fault_code == util::LocalizationStatus::FAULT_NONE &&
          post_restart_status && require_imu_odometry_ready_ &&
          !msg->imu_odometry_ready && msg->fast_lio_initialized &&
          msg->map_valid && msg->alignment_ready && msg->position_valid &&
          msg->restart_scope == util::LocalizationStatus::RESTART_NONE &&
          msg->fault_code == util::LocalizationStatus::FAULT_NONE &&
          !msg->stop_required && !msg->transitioning;
      if (only_waiting_for_imu_odometry) {
        state_.phase = util::LocalizationStatus::PHASE_WAIT_IMU_ODOMETRY;
        state_.reason =
            "localization pose is ready; waiting for IMU odometry readiness";
        state_.stop_required = true;
      }
      const bool algorithm_ready_for_resume =
          AlgorithmReadyForResumeLocked();
      if (!algorithm_ready_for_resume) {
        resume_position_count_ = 0;
      } else if (state_.transitioning && !state_.restart_in_progress &&
                 !map_worker_running_ &&
                 resume_position_count_ < kResumeFrames) {
        // READY describes released production operation. While manager still
        // owns the final consecutive-frame proof, expose that proof explicitly
        // instead of the contradictory READY + stop_required tuple.
        state_.phase = util::LocalizationStatus::PHASE_WAIT_TRUSTED_OUTPUT;
        state_.reason = "algorithm is ready; verifying consecutive trusted "
                        "production positions before releasing stop";
        state_.stop_required = true;
      }
      CompleteTransitionIfReadyLocked();
      if (requested_scope != util::LocalizationStatus::RESTART_NONE) {
        requested_runtime =
            requested_scope == util::LocalizationStatus::RESTART_LIO
                ? RecoveryRuntime::kUnknown
                : AutomaticRecoveryRuntimeLocked();
      }
    }
    if (request_stop) PublishStop(true);
    if (requested_scope != util::LocalizationStatus::RESTART_NONE) {
      const bool accepted = BeginRestart(
          requested_scope, requested_runtime,
          msg->reason.empty()
              ? (requested_scope == util::LocalizationStatus::RESTART_ALL
                     ? "fusion requested full recovery"
                     : "fusion requested LIO recovery")
              : msg->reason,
          true);
      if (accepted) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (requested_scope == util::LocalizationStatus::RESTART_LIO)
          lio_recovery_request_consumed_ = true;
        else if (requested_scope == util::LocalizationStatus::RESTART_ALL)
          full_recovery_request_consumed_ = true;
      }
    }
  }
  void LioStaticCallback(const std_msgs::BoolConstPtr &msg) {
    if (!msg) return;
    // Compatibility/diagnostic subscription only. This Bool has no process
    // identity, so a delayed latched value must not drive lifecycle state.
    // The generation-tagged algorithm status is authoritative.
  }
  void ImuReadyCallback(const std_msgs::BoolConstPtr &msg) {
    if (!msg) return;
    // Compatibility/diagnostic subscription only; see LioStaticCallback.
  }
  void PositionCallback(const util::PositionConstPtr &msg) {
    if (!msg || !IsProductionPosition(msg->position_state)) return;
    const double ros_now = ros::Time::now().toSec();
    const double stamp = msg->header.stamp.toSec();
    if (!android_manager_4dof::MeasurementStampFresh(
            stamp, ros_now, measurement_max_age_sec_,
            measurement_max_future_sec_)) {
      ROS_WARN_THROTTLE(1.0,
                        "Ignore stale/future production position in manager "
                        "watchdog (age=%.3fs).",
                        ros_now - stamp);
      return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (last_position_measurement_stamp_ > 0.0 &&
        stamp < last_position_measurement_stamp_) {
      ROS_WARN_THROTTLE(1.0,
                        "Ignore regressed production position timestamp.");
      return;
    }
    if (restart_generation_verification_required_ &&
        (!restart_new_generation_seen_ ||
         msg->header.stamp.isZero() ||
         msg->header.stamp < restart_generation_accepted_status_stamp_))
      return;
    // A numeric production state alone is insufficient: it can be an old
    // publisher, a frame emitted while fusion is still releasing its stop, or
    // a frame from before map/generation readiness. Only count/output-watchdog
    // frames while the generation-tagged algorithm contract is coherently
    // usable. Any gap breaks the manager's consecutive release proof.
    if (!AlgorithmReadyForResumeLocked()) {
      if (state_.transitioning) resume_position_count_ = 0;
      return;
    }
    const double now = ros::SteadyTime::now().toSec();
    last_position_measurement_stamp_ = stamp;
    last_position_rx_sec_ = now;
    ++valid_position_count_;
    if (valid_position_count_ >= kFrequencyArmFrames) frequency_armed_ = true;
    if (frequency_window_start_sec_ <= 0.0) frequency_window_start_sec_ = now;
    if (state_.transitioning && !state_.restart_in_progress &&
        (!restart_generation_verification_required_ ||
         restart_new_generation_seen_)) {
      if (!require_imu_odometry_ready_ ||
          algorithm_status_.imu_odometry_ready)
        ++resume_position_count_;
      else resume_position_count_ = 0;
    }
    CompleteTransitionIfReadyLocked();
  }
  void GpsCallback(const util::GpsPositionConstPtr &) {}
  void LioCallback(const util::LIOPoseConstPtr &msg) {
    if (!msg) return;
    const double ros_now = ros::Time::now().toSec();
    const double stamp = msg->header.stamp.toSec();
    if (!android_manager_4dof::MeasurementStampFresh(
            stamp, ros_now, measurement_max_age_sec_,
            measurement_max_future_sec_)) {
      ROS_WARN_THROTTLE(1.0,
                        "LIO callback is active but measurement time is not "
                        "fresh (age=%.3fs); do not refresh LIO health.",
                        ros_now - stamp);
      return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (last_lio_measurement_stamp_ > 0.0 &&
        stamp < last_lio_measurement_stamp_) {
      ROS_WARN_THROTTLE(1.0, "Ignore regressed LIO timestamp in manager health.");
      return;
    }
    const double now = ros::SteadyTime::now().toSec();
    last_lio_measurement_stamp_ = stamp;
    last_lio_rx_sec_ = now;
    if (msg->lio_state) last_lio_good_rx_sec_ = now;
  }

  void WatchdogTimer(const ros::TimerEvent &) {
    bool stop = false, frequency_restart = false, position_restart = false;
    bool full_position_restart = false, low_frequency_full_restart = true;
    RecoveryRuntime recovery_runtime = RecoveryRuntime::kUnknown;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      const double now = ros::SteadyTime::now().toSec();
      const bool algorithm_stale = have_algorithm_status_ &&
          now - last_algorithm_status_rx_sec_ > kAlgorithmStatusTimeoutSec;
      const bool status_is_post_restart = restart_completed_ros_time_.isZero() ||
          (have_algorithm_status_ && !algorithm_status_.header.stamp.isZero() &&
           algorithm_status_.header.stamp >= restart_completed_ros_time_);
      const bool transition_status_timeout = state_.transitioning &&
          !state_.restart_in_progress && !map_worker_running_ &&
          !restart_generation_verification_required_ &&
          transition_wait_start_sec_ > 0.0 &&
          now - transition_wait_start_sec_ > transition_status_timeout_sec_ &&
          (!have_algorithm_status_ || algorithm_stale ||
           !status_is_post_restart);
      const bool partial_restart_timeout =
          android_manager_4dof::RestartGenerationVerificationTimedOut(
              restart_generation_verification_required_,
              restart_new_generation_seen_,
              restart_verification_deadline_sec_, now);
      const bool post_restart_heartbeat_stale =
          android_manager_4dof::PostRestartReadinessHeartbeatStale(
              state_.transitioning, state_.restart_in_progress,
              map_worker_running_, restart_generation_verification_required_,
              restart_new_generation_seen_, algorithm_stale);
      if (partial_restart_timeout &&
          state_.fault_code == util::LocalizationStatus::FAULT_NONE) {
        SetFaultLocked(
            util::LocalizationStatus::FAULT_RESTART_PARTIAL,
            "expected process generation(s) were not verified within " +
                std::to_string(restart_generation_timeout_sec_) +
                " seconds after the restart wrapper completed");
        stop = true;
      }
      if (post_restart_heartbeat_stale &&
          state_.fault_code == util::LocalizationStatus::FAULT_NONE) {
        SetFaultLocked(util::LocalizationStatus::FAULT_ALGORITHM,
                       "post-restart algorithm status heartbeat became stale "
                       "while waiting for localization readiness");
        stop = true;
      }
      if (transition_status_timeout &&
          state_.fault_code == util::LocalizationStatus::FAULT_NONE) {
        SetFaultLocked(
            canonical_map_valid_ ? util::LocalizationStatus::FAULT_ALGORITHM
                                 : util::LocalizationStatus::FAULT_MAP_INVALID,
            canonical_map_valid_
                ? "fusion algorithm status did not appear within " +
                      std::to_string(transition_status_timeout_sec_) +
                      " seconds"
                : "fusion did not start and canonical map is invalid: " +
                      canonical_map_error_);
        stop = true;
      }
      if (!state_.transitioning && !state_.restart_in_progress &&
          algorithm_stale &&
          state_.fault_code == util::LocalizationStatus::FAULT_NONE) {
        stop = EnterAlgorithmHeartbeatWaitLocked(
            "fusion algorithm status heartbeat is stale; waiting for "
            "coherent recovery");
      }
      if (!state_.transitioning && !state_.restart_in_progress &&
          state_.fault_code == util::LocalizationStatus::FAULT_NONE &&
          frequency_armed_ &&
          frequency_window_start_sec_ > 0.0 && now - frequency_window_start_sec_ >= 0.95) {
        const double frequency = valid_position_count_ / (now - frequency_window_start_sec_);
        if (frequency < kMinimumPositionHz) {
          ++low_frequency_window_count_; state_.watchdog_low_frequency = true;
          state_.reason = "valid position frequency is low: " + std::to_string(frequency) + " Hz";
          frequency_restart = low_frequency_window_count_ >= kLowFrequencyWindows;
        } else { low_frequency_window_count_ = 0; state_.watchdog_low_frequency = false; }
        valid_position_count_ = 0; frequency_window_start_sec_ = now;
      }
      const bool lio_alive = last_lio_rx_sec_ > 0.0 && now - last_lio_rx_sec_ <= kLioAliveTimeoutSec;
      const bool lio_quality_good =
          last_lio_good_rx_sec_ > 0.0 &&
          now - last_lio_good_rx_sec_ <= kLioAliveTimeoutSec;
      if (frequency_restart) {
        low_frequency_full_restart =
            android_manager_4dof::LowFrequencyNeedsFullRestart(
                lio_alive, lio_quality_good, fast_lio_static_ready_);
      }
      const bool monitor_position =
          state_.fault_code == util::LocalizationStatus::FAULT_NONE &&
          android_manager_4dof::PositionWatchdogAllowed(
              state_.transitioning, position_recovery_wait_,
              state_.restart_in_progress,
              have_algorithm_status_ &&
                  (algorithm_status_.phase ==
                       util::LocalizationStatus::PHASE_RECOVERING_LIO ||
                   algorithm_status_.phase ==
                       util::LocalizationStatus::PHASE_READY_IMU_RESTRICTED)) &&
          (last_position_rx_sec_ > 0.0 ||
           (position_recovery_wait_ && transition_wait_start_sec_ > 0.0));
      if (monitor_position) {
        const double position_reference_sec =
            last_position_rx_sec_ > 0.0 ? last_position_rx_sec_
                                        : transition_wait_start_sec_;
        const double age = now - position_reference_sec;
        if (age > kPositionStopTimeoutSec) { state_.watchdog_position_timeout = true;
          state_.stop_required = true;
          state_.reason = lio_alive
                              ? "valid position timed out while LIO is alive"
                              : "valid position and LIO streams timed out";
          stop = true; }
        if (age > kPositionRestartTimeoutSec) {
          const bool verified_fusion_only_recovery_wait =
              restart_generation_verification_required_ &&
              restart_new_generation_seen_ &&
              restart_generation_scope_ ==
                  util::LocalizationStatus::RESTART_FUSION;
          full_position_restart =
              android_manager_4dof::PositionTimeoutNeedsFullRestart(
                  lio_alive, verified_fusion_only_recovery_wait);
          position_restart = !full_position_restart;
        }
      }
      recovery_runtime = AutomaticRecoveryRuntimeLocked();
    }
    if (stop) PublishStop(true);
    if (frequency_restart) {
      const uint8_t scope =
          low_frequency_full_restart
              ? util::LocalizationStatus::RESTART_ALL
              : util::LocalizationStatus::RESTART_FUSION;
      BeginRestart(scope, recovery_runtime,
                   low_frequency_full_restart
                       ? "low valid-position frequency with unhealthy LIO"
                       : "low valid-position frequency while LIO remains healthy",
                   true);
    }
    else if (position_restart)
      BeginRestart(util::LocalizationStatus::RESTART_FUSION, recovery_runtime,
                   "fusion position timeout while LIO remains alive", true);
    else if (full_position_restart)
      BeginRestart(util::LocalizationStatus::RESTART_ALL, recovery_runtime,
                   "position and LIO timeout", true);
  }
  void StatusTimer(const ros::TimerEvent &) {
    StartPendingRecoveryIfIdle();
    PublishStatus();
  }
  void PublishStatus() {
    util::LocalizationStatus output;
    bool publish_stop = false;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      const bool fresh = have_algorithm_status_ &&
          ros::SteadyTime::now().toSec() - last_algorithm_status_rx_sec_ <= kAlgorithmStatusTimeoutSec;
      if (!fresh && !state_.restart_in_progress)
        publish_stop = EnterAlgorithmHeartbeatWaitLocked(
            "fusion algorithm status heartbeat is stale; waiting for "
            "coherent recovery");
      const bool post_restart = restart_completed_ros_time_.isZero() ||
          (have_algorithm_status_ && !algorithm_status_.header.stamp.isZero() &&
           algorithm_status_.header.stamp >= restart_completed_ros_time_);
      const GenerationToken current_fusion{
          algorithm_status_.fusion_generation_valid != 0,
          algorithm_status_.fusion_generation_sec,
          algorithm_status_.fusion_generation_nsec};
      const GenerationToken current_lio{
          algorithm_status_.lio_generation_valid != 0,
          algorithm_status_.lio_generation_sec,
          algorithm_status_.lio_generation_nsec};
      const bool generation_verified =
          !restart_generation_verification_required_ ||
          android_manager_4dof::ExpectedRestartGenerationsObserved(
              restart_generation_scope_ == util::LocalizationStatus::RESTART_FUSION ||
                  restart_generation_scope_ == util::LocalizationStatus::RESTART_ALL,
              restart_generation_scope_ == util::LocalizationStatus::RESTART_LIO ||
                  restart_generation_scope_ == util::LocalizationStatus::RESTART_ALL,
              restart_generation_baseline_.fusion, current_fusion,
              restart_generation_baseline_.lio, current_lio,
              restart_generation_request_floor_);
      if (have_algorithm_status_) output = algorithm_status_;
      output.header.stamp = ros::Time::now();
      output.reporter = ros::this_node::getName();
      output.requested_mode = state_.requested_mode; output.active_mode = state_.active_mode;
      output.workflow_mode = state_.workflow_mode;
      output.requested_runtime_mode = state_.requested_runtime_mode;
      output.active_runtime_mode = state_.active_runtime_mode;
      output.phase = state_.phase; output.active_map = state_.active_map;
      output.pending_map = state_.pending_map; output.last_created_map = state_.last_created_map;
      output.active_map_uuid = state_.active_map_uuid;
      output.pending_map_uuid = state_.pending_map_uuid;
      output.map_schema_version = state_.map_schema_version;
      output.coordinate_frame = state_.coordinate_frame;
      output.origin_checksum = state_.origin_checksum;
      output.map_origin_committed = state_.map_origin_committed;
      output.algorithm_status_valid = fresh && post_restart && generation_verified;
      output.transitioning = state_.transitioning;
      output.restart_in_progress = state_.restart_in_progress; output.restart_scope = state_.restart_scope;
      output.watchdog_position_timeout = state_.watchdog_position_timeout;
      output.watchdog_low_frequency = state_.watchdog_low_frequency;
      output.automatic_restart_suspended = state_.automatic_restart_suspended;
      output.automatic_restart_count = automatic_restart_times_.size();
      output.stop_required = state_.stop_required; output.fault_code = state_.fault_code;
      output.reason = state_.reason;
      // This field is the *usable production output*, not merely the last
      // algorithm sample. Never report it valid while the lifecycle owner is
      // stopped, faulted, or still verifying a transition.
      if (!generation_verified || state_.stop_required ||
          state_.transitioning ||
          state_.fault_code != util::LocalizationStatus::FAULT_NONE)
        output.position_valid = false;
      if (state_.fault_code == util::LocalizationStatus::FAULT_MAP_INVALID)
        output.map_valid = false;
      if (!fresh && !state_.restart_in_progress) {
        output.position_valid = false;
      }
      if (state_.fault_code != util::LocalizationStatus::FAULT_NONE) {
        // A manager-owned fault is sticky and internally consistent until an
        // explicit/automatic recovery begins. Algorithm heartbeats must not
        // rewrite only its phase/reason while leaving the fault code latched.
        output.phase = util::LocalizationStatus::PHASE_FAULT;
        output.transitioning = false;
        output.stop_required = true;
        output.position_valid = false;
      }
    }
    if (publish_stop) PublishStop(true);
    pub_status_.publish(output);
  }

  ros::NodeHandle nh_, private_nh_;
  ros::Publisher pub_manager_stop_car_, pub_final_stop_car_, pub_status_;
  ros::ServiceClient map_client_;
  ros::Subscriber sub_android_, sub_gps_, sub_position_, sub_reboot_, sub_lio_;
  ros::Subscriber sub_lio_static_, sub_imu_ready_, sub_algorithm_status_;
  ros::Subscriber sub_fusion_stop_car_;
  ros::Timer watchdog_timer_, status_timer_;
  std::string android_topic_, gps_topic_, fusion_topic_, reboot_topic_, stop_car_topic_;
  std::string fusion_stop_car_topic_, final_stop_car_topic_;
  std::string lio_static_status_topic_, imu_odometry_ready_topic_;
  std::string algorithm_status_topic_, status_topic_, map_path_, map_service_name_;
  std::string fusion_package_ = "fusion_4dof";
  bool require_imu_odometry_ready_ = true;
  // Default true is fail-safe until the latched fusion request is received.
  std::atomic<bool> manager_stop_required_{true};
  std::atomic<bool> fusion_stop_required_{true};
  double transition_status_timeout_sec_ = 10.0;
  double restart_generation_timeout_sec_ = 30.0;
  double measurement_max_age_sec_ = 0.50;
  double measurement_max_future_sec_ = 0.05;
  std::mutex mutex_;
  ManagerState state_;
  util::LocalizationStatus algorithm_status_;
  bool have_algorithm_status_ = false, fast_lio_static_ready_ = false;
  bool imu_odometry_ready_ = false, lio_recovery_request_consumed_ = false;
  bool full_recovery_request_consumed_ = false;
  uint8_t pending_recovery_scope_ = util::LocalizationStatus::RESTART_NONE;
  RecoveryRuntime pending_recovery_runtime_ = RecoveryRuntime::kUnknown;
  std::string pending_recovery_reason_;
  bool pending_recovery_revalidate_after_lio_ = false;
  GenerationToken pending_recovery_lio_baseline_;
  GenerationToken lio_restart_baseline_;
  RestartBaseline restart_generation_baseline_;
  GenerationToken restart_generation_request_floor_;
  uint8_t restart_generation_scope_ =
      util::LocalizationStatus::RESTART_NONE;
  bool restart_generation_verification_required_ = false;
  bool restart_new_generation_seen_ = false;
  ros::Time restart_generation_accepted_status_stamp_;
  double last_algorithm_status_rx_sec_ = 0.0, last_lio_rx_sec_ = 0.0;
  double last_lio_good_rx_sec_ = 0.0;
  double last_lio_measurement_stamp_ = 0.0;
  double startup_steady_sec_ = 0.0;
  double transition_wait_start_sec_ = 0.0;
  double restart_verification_deadline_sec_ = 0.0;
  ros::Time restart_completed_ros_time_;
  double last_position_rx_sec_ = 0.0, frequency_window_start_sec_ = 0.0;
  double last_position_measurement_stamp_ = 0.0;
  int valid_position_count_ = 0, resume_position_count_ = 0;
  int low_frequency_window_count_ = 0;
  bool frequency_armed_ = false;
  bool position_recovery_wait_ = false;
  bool canonical_map_valid_ = false;
  std::string canonical_map_error_;
  std::deque<double> automatic_restart_times_;
  std::thread restart_worker_;
  bool restart_worker_running_ = false;
  std::thread map_worker_;
  bool map_worker_running_ = false;
};

int main(int argc, char **argv) {
  ros::init(argc, argv, "android_manager_4dof");
  LocalizationManager manager;
  ros::spin();
  return 0;
}
