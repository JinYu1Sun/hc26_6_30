;; Auto-generated. Do not edit!


(when (boundp 'util::LocalizationStatus)
  (if (not (find-package "UTIL"))
    (make-package "UTIL"))
  (shadow 'LocalizationStatus (find-package "UTIL")))
(unless (find-package "UTIL::LOCALIZATIONSTATUS")
  (make-package "UTIL::LOCALIZATIONSTATUS"))

(in-package "ROS")
;;//! \htmlinclude LocalizationStatus.msg.html
(if (not (find-package "STD_MSGS"))
  (ros::roseus-add-msgs "std_msgs"))


(intern "*MODE_UNKNOWN*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*MODE_UNKNOWN* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*MODE_UNKNOWN* 0)
(intern "*MODE_IDLE*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*MODE_IDLE* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*MODE_IDLE* 1)
(intern "*MODE_MAPPING*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*MODE_MAPPING* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*MODE_MAPPING* 2)
(intern "*MODE_LOCALIZATION*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*MODE_LOCALIZATION* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*MODE_LOCALIZATION* 3)
(intern "*WORKFLOW_UNKNOWN*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*WORKFLOW_UNKNOWN* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*WORKFLOW_UNKNOWN* 0)
(intern "*WORKFLOW_MAPPING*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*WORKFLOW_MAPPING* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*WORKFLOW_MAPPING* 1)
(intern "*WORKFLOW_LOCALIZATION*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*WORKFLOW_LOCALIZATION* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*WORKFLOW_LOCALIZATION* 2)
(intern "*RUNTIME_UNKNOWN*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*RUNTIME_UNKNOWN* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*RUNTIME_UNKNOWN* 0)
(intern "*RUNTIME_CREATE_ORIGIN*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*RUNTIME_CREATE_ORIGIN* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*RUNTIME_CREATE_ORIGIN* 1)
(intern "*RUNTIME_TRACK_SAVED_ORIGIN*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*RUNTIME_TRACK_SAVED_ORIGIN* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*RUNTIME_TRACK_SAVED_ORIGIN* 2)
(intern "*PHASE_UNKNOWN*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*PHASE_UNKNOWN* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*PHASE_UNKNOWN* 0)
(intern "*PHASE_WAIT_FAST_LIO*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*PHASE_WAIT_FAST_LIO* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*PHASE_WAIT_FAST_LIO* 1)
(intern "*PHASE_WAIT_INS*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*PHASE_WAIT_INS* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*PHASE_WAIT_INS* 2)
(intern "*PHASE_WAIT_MAP*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*PHASE_WAIT_MAP* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*PHASE_WAIT_MAP* 3)
(intern "*PHASE_WAIT_GPS*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*PHASE_WAIT_GPS* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*PHASE_WAIT_GPS* 4)
(intern "*PHASE_WAIT_ALIGNMENT*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*PHASE_WAIT_ALIGNMENT* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*PHASE_WAIT_ALIGNMENT* 5)
(intern "*PHASE_READY_RTK*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*PHASE_READY_RTK* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*PHASE_READY_RTK* 6)
(intern "*PHASE_READY_LIO_LOCKED*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*PHASE_READY_LIO_LOCKED* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*PHASE_READY_LIO_LOCKED* 7)
(intern "*PHASE_READY_LIO_SEEDED*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*PHASE_READY_LIO_SEEDED* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*PHASE_READY_LIO_SEEDED* 8)
(intern "*PHASE_READY_IMU_RESTRICTED*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*PHASE_READY_IMU_RESTRICTED* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*PHASE_READY_IMU_RESTRICTED* 9)
(intern "*PHASE_SWITCHING_MAP*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*PHASE_SWITCHING_MAP* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*PHASE_SWITCHING_MAP* 10)
(intern "*PHASE_RESTARTING_FUSION*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*PHASE_RESTARTING_FUSION* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*PHASE_RESTARTING_FUSION* 11)
(intern "*PHASE_RESTARTING_ALL*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*PHASE_RESTARTING_ALL* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*PHASE_RESTARTING_ALL* 12)
(intern "*PHASE_RECOVERING_LIO*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*PHASE_RECOVERING_LIO* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*PHASE_RECOVERING_LIO* 13)
(intern "*PHASE_FAULT*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*PHASE_FAULT* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*PHASE_FAULT* 14)
(intern "*PHASE_WAIT_IMU_ODOMETRY*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*PHASE_WAIT_IMU_ODOMETRY* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*PHASE_WAIT_IMU_ODOMETRY* 15)
(intern "*PHASE_RESTART_VERIFYING*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*PHASE_RESTART_VERIFYING* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*PHASE_RESTART_VERIFYING* 16)
(intern "*PHASE_WAIT_ALGORITHM*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*PHASE_WAIT_ALGORITHM* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*PHASE_WAIT_ALGORITHM* 17)
(intern "*PHASE_WAIT_TRUSTED_OUTPUT*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*PHASE_WAIT_TRUSTED_OUTPUT* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*PHASE_WAIT_TRUSTED_OUTPUT* 18)
(intern "*SOURCE_NONE*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*SOURCE_NONE* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*SOURCE_NONE* 0)
(intern "*SOURCE_RTK*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*SOURCE_RTK* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*SOURCE_RTK* 1)
(intern "*SOURCE_LIO_LOCKED*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*SOURCE_LIO_LOCKED* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*SOURCE_LIO_LOCKED* 2)
(intern "*SOURCE_LIO_SEEDED*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*SOURCE_LIO_SEEDED* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*SOURCE_LIO_SEEDED* 3)
(intern "*SOURCE_IMU_RESTRICTED*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*SOURCE_IMU_RESTRICTED* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*SOURCE_IMU_RESTRICTED* 4)
(intern "*RESTART_NONE*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*RESTART_NONE* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*RESTART_NONE* 0)
(intern "*RESTART_FUSION*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*RESTART_FUSION* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*RESTART_FUSION* 1)
(intern "*RESTART_LIO*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*RESTART_LIO* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*RESTART_LIO* 2)
(intern "*RESTART_ALL*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*RESTART_ALL* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*RESTART_ALL* 3)
(intern "*FAULT_NONE*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*FAULT_NONE* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*FAULT_NONE* 0)
(intern "*FAULT_MAP_INVALID*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*FAULT_MAP_INVALID* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*FAULT_MAP_INVALID* 1)
(intern "*FAULT_RESTART_FAILED*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*FAULT_RESTART_FAILED* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*FAULT_RESTART_FAILED* 2)
(intern "*FAULT_RESTART_SUPPRESSED*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*FAULT_RESTART_SUPPRESSED* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*FAULT_RESTART_SUPPRESSED* 3)
(intern "*FAULT_POSITION_TIMEOUT*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*FAULT_POSITION_TIMEOUT* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*FAULT_POSITION_TIMEOUT* 4)
(intern "*FAULT_LOW_FREQUENCY*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*FAULT_LOW_FREQUENCY* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*FAULT_LOW_FREQUENCY* 5)
(intern "*FAULT_ALGORITHM*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*FAULT_ALGORITHM* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*FAULT_ALGORITHM* 6)
(intern "*FAULT_RESTART_PARTIAL*" (find-package "UTIL::LOCALIZATIONSTATUS"))
(shadow '*FAULT_RESTART_PARTIAL* (find-package "UTIL::LOCALIZATIONSTATUS"))
(defconstant util::LocalizationStatus::*FAULT_RESTART_PARTIAL* 7)

(defun util::LocalizationStatus-to-symbol (const)
  (cond
        ((= const 0) 'util::LocalizationStatus::*MODE_UNKNOWN*)
        ((= const 1) 'util::LocalizationStatus::*MODE_IDLE*)
        ((= const 2) 'util::LocalizationStatus::*MODE_MAPPING*)
        ((= const 3) 'util::LocalizationStatus::*MODE_LOCALIZATION*)
        ((= const 0) 'util::LocalizationStatus::*WORKFLOW_UNKNOWN*)
        ((= const 1) 'util::LocalizationStatus::*WORKFLOW_MAPPING*)
        ((= const 2) 'util::LocalizationStatus::*WORKFLOW_LOCALIZATION*)
        ((= const 0) 'util::LocalizationStatus::*RUNTIME_UNKNOWN*)
        ((= const 1) 'util::LocalizationStatus::*RUNTIME_CREATE_ORIGIN*)
        ((= const 2) 'util::LocalizationStatus::*RUNTIME_TRACK_SAVED_ORIGIN*)
        ((= const 0) 'util::LocalizationStatus::*PHASE_UNKNOWN*)
        ((= const 1) 'util::LocalizationStatus::*PHASE_WAIT_FAST_LIO*)
        ((= const 2) 'util::LocalizationStatus::*PHASE_WAIT_INS*)
        ((= const 3) 'util::LocalizationStatus::*PHASE_WAIT_MAP*)
        ((= const 4) 'util::LocalizationStatus::*PHASE_WAIT_GPS*)
        ((= const 5) 'util::LocalizationStatus::*PHASE_WAIT_ALIGNMENT*)
        ((= const 6) 'util::LocalizationStatus::*PHASE_READY_RTK*)
        ((= const 7) 'util::LocalizationStatus::*PHASE_READY_LIO_LOCKED*)
        ((= const 8) 'util::LocalizationStatus::*PHASE_READY_LIO_SEEDED*)
        ((= const 9) 'util::LocalizationStatus::*PHASE_READY_IMU_RESTRICTED*)
        ((= const 10) 'util::LocalizationStatus::*PHASE_SWITCHING_MAP*)
        ((= const 11) 'util::LocalizationStatus::*PHASE_RESTARTING_FUSION*)
        ((= const 12) 'util::LocalizationStatus::*PHASE_RESTARTING_ALL*)
        ((= const 13) 'util::LocalizationStatus::*PHASE_RECOVERING_LIO*)
        ((= const 14) 'util::LocalizationStatus::*PHASE_FAULT*)
        ((= const 15) 'util::LocalizationStatus::*PHASE_WAIT_IMU_ODOMETRY*)
        ((= const 16) 'util::LocalizationStatus::*PHASE_RESTART_VERIFYING*)
        ((= const 17) 'util::LocalizationStatus::*PHASE_WAIT_ALGORITHM*)
        ((= const 18) 'util::LocalizationStatus::*PHASE_WAIT_TRUSTED_OUTPUT*)
        ((= const 0) 'util::LocalizationStatus::*SOURCE_NONE*)
        ((= const 1) 'util::LocalizationStatus::*SOURCE_RTK*)
        ((= const 2) 'util::LocalizationStatus::*SOURCE_LIO_LOCKED*)
        ((= const 3) 'util::LocalizationStatus::*SOURCE_LIO_SEEDED*)
        ((= const 4) 'util::LocalizationStatus::*SOURCE_IMU_RESTRICTED*)
        ((= const 0) 'util::LocalizationStatus::*RESTART_NONE*)
        ((= const 1) 'util::LocalizationStatus::*RESTART_FUSION*)
        ((= const 2) 'util::LocalizationStatus::*RESTART_LIO*)
        ((= const 3) 'util::LocalizationStatus::*RESTART_ALL*)
        ((= const 0) 'util::LocalizationStatus::*FAULT_NONE*)
        ((= const 1) 'util::LocalizationStatus::*FAULT_MAP_INVALID*)
        ((= const 2) 'util::LocalizationStatus::*FAULT_RESTART_FAILED*)
        ((= const 3) 'util::LocalizationStatus::*FAULT_RESTART_SUPPRESSED*)
        ((= const 4) 'util::LocalizationStatus::*FAULT_POSITION_TIMEOUT*)
        ((= const 5) 'util::LocalizationStatus::*FAULT_LOW_FREQUENCY*)
        ((= const 6) 'util::LocalizationStatus::*FAULT_ALGORITHM*)
        ((= const 7) 'util::LocalizationStatus::*FAULT_RESTART_PARTIAL*)
        (t nil)))

(defclass util::LocalizationStatus
  :super ros::object
  :slots (_header _reporter _requested_mode _active_mode _workflow_mode _requested_runtime_mode _active_runtime_mode _phase _position_source _position_state _position_valid _ins_status_received _ins_solution_good _ins_initialized _ins_status _ins_vehicle_align _ins_heading_flag _ins_position_status _fast_lio_initialized _fusion_generation_valid _fusion_generation_sec _fusion_generation_nsec _lio_generation_valid _lio_generation_sec _lio_generation_nsec _map_origin_ready _map_origin_committed _alignment_ready _alignment_locked _imu_odometry_ready _gps_good _lio_good _active_map _pending_map _last_created_map _active_map_uuid _pending_map_uuid _map_schema_version _coordinate_frame _origin_checksum _map_valid _algorithm_status_valid _transitioning _restart_in_progress _restart_scope _watchdog_position_timeout _watchdog_low_frequency _automatic_restart_suspended _automatic_restart_count _stop_required _fault_code _reason ))

(defmethod util::LocalizationStatus
  (:init
   (&key
    ((:header __header) (instance std_msgs::Header :init))
    ((:reporter __reporter) "")
    ((:requested_mode __requested_mode) 0)
    ((:active_mode __active_mode) 0)
    ((:workflow_mode __workflow_mode) 0)
    ((:requested_runtime_mode __requested_runtime_mode) 0)
    ((:active_runtime_mode __active_runtime_mode) 0)
    ((:phase __phase) 0)
    ((:position_source __position_source) 0)
    ((:position_state __position_state) 0)
    ((:position_valid __position_valid) nil)
    ((:ins_status_received __ins_status_received) nil)
    ((:ins_solution_good __ins_solution_good) nil)
    ((:ins_initialized __ins_initialized) nil)
    ((:ins_status __ins_status) 0)
    ((:ins_vehicle_align __ins_vehicle_align) 0)
    ((:ins_heading_flag __ins_heading_flag) 0)
    ((:ins_position_status __ins_position_status) 0)
    ((:fast_lio_initialized __fast_lio_initialized) nil)
    ((:fusion_generation_valid __fusion_generation_valid) nil)
    ((:fusion_generation_sec __fusion_generation_sec) 0)
    ((:fusion_generation_nsec __fusion_generation_nsec) 0)
    ((:lio_generation_valid __lio_generation_valid) nil)
    ((:lio_generation_sec __lio_generation_sec) 0)
    ((:lio_generation_nsec __lio_generation_nsec) 0)
    ((:map_origin_ready __map_origin_ready) nil)
    ((:map_origin_committed __map_origin_committed) nil)
    ((:alignment_ready __alignment_ready) nil)
    ((:alignment_locked __alignment_locked) nil)
    ((:imu_odometry_ready __imu_odometry_ready) nil)
    ((:gps_good __gps_good) nil)
    ((:lio_good __lio_good) nil)
    ((:active_map __active_map) "")
    ((:pending_map __pending_map) "")
    ((:last_created_map __last_created_map) "")
    ((:active_map_uuid __active_map_uuid) "")
    ((:pending_map_uuid __pending_map_uuid) "")
    ((:map_schema_version __map_schema_version) 0)
    ((:coordinate_frame __coordinate_frame) "")
    ((:origin_checksum __origin_checksum) "")
    ((:map_valid __map_valid) nil)
    ((:algorithm_status_valid __algorithm_status_valid) nil)
    ((:transitioning __transitioning) nil)
    ((:restart_in_progress __restart_in_progress) nil)
    ((:restart_scope __restart_scope) 0)
    ((:watchdog_position_timeout __watchdog_position_timeout) nil)
    ((:watchdog_low_frequency __watchdog_low_frequency) nil)
    ((:automatic_restart_suspended __automatic_restart_suspended) nil)
    ((:automatic_restart_count __automatic_restart_count) 0)
    ((:stop_required __stop_required) nil)
    ((:fault_code __fault_code) 0)
    ((:reason __reason) "")
    )
   (send-super :init)
   (setq _header __header)
   (setq _reporter (string __reporter))
   (setq _requested_mode (round __requested_mode))
   (setq _active_mode (round __active_mode))
   (setq _workflow_mode (round __workflow_mode))
   (setq _requested_runtime_mode (round __requested_runtime_mode))
   (setq _active_runtime_mode (round __active_runtime_mode))
   (setq _phase (round __phase))
   (setq _position_source (round __position_source))
   (setq _position_state (round __position_state))
   (setq _position_valid __position_valid)
   (setq _ins_status_received __ins_status_received)
   (setq _ins_solution_good __ins_solution_good)
   (setq _ins_initialized __ins_initialized)
   (setq _ins_status (round __ins_status))
   (setq _ins_vehicle_align (round __ins_vehicle_align))
   (setq _ins_heading_flag (round __ins_heading_flag))
   (setq _ins_position_status (round __ins_position_status))
   (setq _fast_lio_initialized __fast_lio_initialized)
   (setq _fusion_generation_valid __fusion_generation_valid)
   (setq _fusion_generation_sec (round __fusion_generation_sec))
   (setq _fusion_generation_nsec (round __fusion_generation_nsec))
   (setq _lio_generation_valid __lio_generation_valid)
   (setq _lio_generation_sec (round __lio_generation_sec))
   (setq _lio_generation_nsec (round __lio_generation_nsec))
   (setq _map_origin_ready __map_origin_ready)
   (setq _map_origin_committed __map_origin_committed)
   (setq _alignment_ready __alignment_ready)
   (setq _alignment_locked __alignment_locked)
   (setq _imu_odometry_ready __imu_odometry_ready)
   (setq _gps_good __gps_good)
   (setq _lio_good __lio_good)
   (setq _active_map (string __active_map))
   (setq _pending_map (string __pending_map))
   (setq _last_created_map (string __last_created_map))
   (setq _active_map_uuid (string __active_map_uuid))
   (setq _pending_map_uuid (string __pending_map_uuid))
   (setq _map_schema_version (round __map_schema_version))
   (setq _coordinate_frame (string __coordinate_frame))
   (setq _origin_checksum (string __origin_checksum))
   (setq _map_valid __map_valid)
   (setq _algorithm_status_valid __algorithm_status_valid)
   (setq _transitioning __transitioning)
   (setq _restart_in_progress __restart_in_progress)
   (setq _restart_scope (round __restart_scope))
   (setq _watchdog_position_timeout __watchdog_position_timeout)
   (setq _watchdog_low_frequency __watchdog_low_frequency)
   (setq _automatic_restart_suspended __automatic_restart_suspended)
   (setq _automatic_restart_count (round __automatic_restart_count))
   (setq _stop_required __stop_required)
   (setq _fault_code (round __fault_code))
   (setq _reason (string __reason))
   self)
  (:header
   (&rest __header)
   (if (keywordp (car __header))
       (send* _header __header)
     (progn
       (if __header (setq _header (car __header)))
       _header)))
  (:reporter
   (&optional __reporter)
   (if __reporter (setq _reporter __reporter)) _reporter)
  (:requested_mode
   (&optional __requested_mode)
   (if __requested_mode (setq _requested_mode __requested_mode)) _requested_mode)
  (:active_mode
   (&optional __active_mode)
   (if __active_mode (setq _active_mode __active_mode)) _active_mode)
  (:workflow_mode
   (&optional __workflow_mode)
   (if __workflow_mode (setq _workflow_mode __workflow_mode)) _workflow_mode)
  (:requested_runtime_mode
   (&optional __requested_runtime_mode)
   (if __requested_runtime_mode (setq _requested_runtime_mode __requested_runtime_mode)) _requested_runtime_mode)
  (:active_runtime_mode
   (&optional __active_runtime_mode)
   (if __active_runtime_mode (setq _active_runtime_mode __active_runtime_mode)) _active_runtime_mode)
  (:phase
   (&optional __phase)
   (if __phase (setq _phase __phase)) _phase)
  (:position_source
   (&optional __position_source)
   (if __position_source (setq _position_source __position_source)) _position_source)
  (:position_state
   (&optional __position_state)
   (if __position_state (setq _position_state __position_state)) _position_state)
  (:position_valid
   (&optional (__position_valid :null))
   (if (not (eq __position_valid :null)) (setq _position_valid __position_valid)) _position_valid)
  (:ins_status_received
   (&optional (__ins_status_received :null))
   (if (not (eq __ins_status_received :null)) (setq _ins_status_received __ins_status_received)) _ins_status_received)
  (:ins_solution_good
   (&optional (__ins_solution_good :null))
   (if (not (eq __ins_solution_good :null)) (setq _ins_solution_good __ins_solution_good)) _ins_solution_good)
  (:ins_initialized
   (&optional (__ins_initialized :null))
   (if (not (eq __ins_initialized :null)) (setq _ins_initialized __ins_initialized)) _ins_initialized)
  (:ins_status
   (&optional __ins_status)
   (if __ins_status (setq _ins_status __ins_status)) _ins_status)
  (:ins_vehicle_align
   (&optional __ins_vehicle_align)
   (if __ins_vehicle_align (setq _ins_vehicle_align __ins_vehicle_align)) _ins_vehicle_align)
  (:ins_heading_flag
   (&optional __ins_heading_flag)
   (if __ins_heading_flag (setq _ins_heading_flag __ins_heading_flag)) _ins_heading_flag)
  (:ins_position_status
   (&optional __ins_position_status)
   (if __ins_position_status (setq _ins_position_status __ins_position_status)) _ins_position_status)
  (:fast_lio_initialized
   (&optional (__fast_lio_initialized :null))
   (if (not (eq __fast_lio_initialized :null)) (setq _fast_lio_initialized __fast_lio_initialized)) _fast_lio_initialized)
  (:fusion_generation_valid
   (&optional (__fusion_generation_valid :null))
   (if (not (eq __fusion_generation_valid :null)) (setq _fusion_generation_valid __fusion_generation_valid)) _fusion_generation_valid)
  (:fusion_generation_sec
   (&optional __fusion_generation_sec)
   (if __fusion_generation_sec (setq _fusion_generation_sec __fusion_generation_sec)) _fusion_generation_sec)
  (:fusion_generation_nsec
   (&optional __fusion_generation_nsec)
   (if __fusion_generation_nsec (setq _fusion_generation_nsec __fusion_generation_nsec)) _fusion_generation_nsec)
  (:lio_generation_valid
   (&optional (__lio_generation_valid :null))
   (if (not (eq __lio_generation_valid :null)) (setq _lio_generation_valid __lio_generation_valid)) _lio_generation_valid)
  (:lio_generation_sec
   (&optional __lio_generation_sec)
   (if __lio_generation_sec (setq _lio_generation_sec __lio_generation_sec)) _lio_generation_sec)
  (:lio_generation_nsec
   (&optional __lio_generation_nsec)
   (if __lio_generation_nsec (setq _lio_generation_nsec __lio_generation_nsec)) _lio_generation_nsec)
  (:map_origin_ready
   (&optional (__map_origin_ready :null))
   (if (not (eq __map_origin_ready :null)) (setq _map_origin_ready __map_origin_ready)) _map_origin_ready)
  (:map_origin_committed
   (&optional (__map_origin_committed :null))
   (if (not (eq __map_origin_committed :null)) (setq _map_origin_committed __map_origin_committed)) _map_origin_committed)
  (:alignment_ready
   (&optional (__alignment_ready :null))
   (if (not (eq __alignment_ready :null)) (setq _alignment_ready __alignment_ready)) _alignment_ready)
  (:alignment_locked
   (&optional (__alignment_locked :null))
   (if (not (eq __alignment_locked :null)) (setq _alignment_locked __alignment_locked)) _alignment_locked)
  (:imu_odometry_ready
   (&optional (__imu_odometry_ready :null))
   (if (not (eq __imu_odometry_ready :null)) (setq _imu_odometry_ready __imu_odometry_ready)) _imu_odometry_ready)
  (:gps_good
   (&optional (__gps_good :null))
   (if (not (eq __gps_good :null)) (setq _gps_good __gps_good)) _gps_good)
  (:lio_good
   (&optional (__lio_good :null))
   (if (not (eq __lio_good :null)) (setq _lio_good __lio_good)) _lio_good)
  (:active_map
   (&optional __active_map)
   (if __active_map (setq _active_map __active_map)) _active_map)
  (:pending_map
   (&optional __pending_map)
   (if __pending_map (setq _pending_map __pending_map)) _pending_map)
  (:last_created_map
   (&optional __last_created_map)
   (if __last_created_map (setq _last_created_map __last_created_map)) _last_created_map)
  (:active_map_uuid
   (&optional __active_map_uuid)
   (if __active_map_uuid (setq _active_map_uuid __active_map_uuid)) _active_map_uuid)
  (:pending_map_uuid
   (&optional __pending_map_uuid)
   (if __pending_map_uuid (setq _pending_map_uuid __pending_map_uuid)) _pending_map_uuid)
  (:map_schema_version
   (&optional __map_schema_version)
   (if __map_schema_version (setq _map_schema_version __map_schema_version)) _map_schema_version)
  (:coordinate_frame
   (&optional __coordinate_frame)
   (if __coordinate_frame (setq _coordinate_frame __coordinate_frame)) _coordinate_frame)
  (:origin_checksum
   (&optional __origin_checksum)
   (if __origin_checksum (setq _origin_checksum __origin_checksum)) _origin_checksum)
  (:map_valid
   (&optional (__map_valid :null))
   (if (not (eq __map_valid :null)) (setq _map_valid __map_valid)) _map_valid)
  (:algorithm_status_valid
   (&optional (__algorithm_status_valid :null))
   (if (not (eq __algorithm_status_valid :null)) (setq _algorithm_status_valid __algorithm_status_valid)) _algorithm_status_valid)
  (:transitioning
   (&optional (__transitioning :null))
   (if (not (eq __transitioning :null)) (setq _transitioning __transitioning)) _transitioning)
  (:restart_in_progress
   (&optional (__restart_in_progress :null))
   (if (not (eq __restart_in_progress :null)) (setq _restart_in_progress __restart_in_progress)) _restart_in_progress)
  (:restart_scope
   (&optional __restart_scope)
   (if __restart_scope (setq _restart_scope __restart_scope)) _restart_scope)
  (:watchdog_position_timeout
   (&optional (__watchdog_position_timeout :null))
   (if (not (eq __watchdog_position_timeout :null)) (setq _watchdog_position_timeout __watchdog_position_timeout)) _watchdog_position_timeout)
  (:watchdog_low_frequency
   (&optional (__watchdog_low_frequency :null))
   (if (not (eq __watchdog_low_frequency :null)) (setq _watchdog_low_frequency __watchdog_low_frequency)) _watchdog_low_frequency)
  (:automatic_restart_suspended
   (&optional (__automatic_restart_suspended :null))
   (if (not (eq __automatic_restart_suspended :null)) (setq _automatic_restart_suspended __automatic_restart_suspended)) _automatic_restart_suspended)
  (:automatic_restart_count
   (&optional __automatic_restart_count)
   (if __automatic_restart_count (setq _automatic_restart_count __automatic_restart_count)) _automatic_restart_count)
  (:stop_required
   (&optional (__stop_required :null))
   (if (not (eq __stop_required :null)) (setq _stop_required __stop_required)) _stop_required)
  (:fault_code
   (&optional __fault_code)
   (if __fault_code (setq _fault_code __fault_code)) _fault_code)
  (:reason
   (&optional __reason)
   (if __reason (setq _reason __reason)) _reason)
  (:serialization-length
   ()
   (+
    ;; std_msgs/Header _header
    (send _header :serialization-length)
    ;; string _reporter
    4 (length _reporter)
    ;; uint8 _requested_mode
    1
    ;; uint8 _active_mode
    1
    ;; uint8 _workflow_mode
    1
    ;; uint8 _requested_runtime_mode
    1
    ;; uint8 _active_runtime_mode
    1
    ;; uint8 _phase
    1
    ;; uint8 _position_source
    1
    ;; uint32 _position_state
    4
    ;; bool _position_valid
    1
    ;; bool _ins_status_received
    1
    ;; bool _ins_solution_good
    1
    ;; bool _ins_initialized
    1
    ;; uint8 _ins_status
    1
    ;; uint8 _ins_vehicle_align
    1
    ;; uint8 _ins_heading_flag
    1
    ;; int32 _ins_position_status
    4
    ;; bool _fast_lio_initialized
    1
    ;; bool _fusion_generation_valid
    1
    ;; uint32 _fusion_generation_sec
    4
    ;; uint32 _fusion_generation_nsec
    4
    ;; bool _lio_generation_valid
    1
    ;; uint32 _lio_generation_sec
    4
    ;; uint32 _lio_generation_nsec
    4
    ;; bool _map_origin_ready
    1
    ;; bool _map_origin_committed
    1
    ;; bool _alignment_ready
    1
    ;; bool _alignment_locked
    1
    ;; bool _imu_odometry_ready
    1
    ;; bool _gps_good
    1
    ;; bool _lio_good
    1
    ;; string _active_map
    4 (length _active_map)
    ;; string _pending_map
    4 (length _pending_map)
    ;; string _last_created_map
    4 (length _last_created_map)
    ;; string _active_map_uuid
    4 (length _active_map_uuid)
    ;; string _pending_map_uuid
    4 (length _pending_map_uuid)
    ;; uint32 _map_schema_version
    4
    ;; string _coordinate_frame
    4 (length _coordinate_frame)
    ;; string _origin_checksum
    4 (length _origin_checksum)
    ;; bool _map_valid
    1
    ;; bool _algorithm_status_valid
    1
    ;; bool _transitioning
    1
    ;; bool _restart_in_progress
    1
    ;; uint8 _restart_scope
    1
    ;; bool _watchdog_position_timeout
    1
    ;; bool _watchdog_low_frequency
    1
    ;; bool _automatic_restart_suspended
    1
    ;; uint32 _automatic_restart_count
    4
    ;; bool _stop_required
    1
    ;; uint16 _fault_code
    2
    ;; string _reason
    4 (length _reason)
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; std_msgs/Header _header
       (send _header :serialize s)
     ;; string _reporter
       (write-long (length _reporter) s) (princ _reporter s)
     ;; uint8 _requested_mode
       (write-byte _requested_mode s)
     ;; uint8 _active_mode
       (write-byte _active_mode s)
     ;; uint8 _workflow_mode
       (write-byte _workflow_mode s)
     ;; uint8 _requested_runtime_mode
       (write-byte _requested_runtime_mode s)
     ;; uint8 _active_runtime_mode
       (write-byte _active_runtime_mode s)
     ;; uint8 _phase
       (write-byte _phase s)
     ;; uint8 _position_source
       (write-byte _position_source s)
     ;; uint32 _position_state
       (write-long _position_state s)
     ;; bool _position_valid
       (if _position_valid (write-byte -1 s) (write-byte 0 s))
     ;; bool _ins_status_received
       (if _ins_status_received (write-byte -1 s) (write-byte 0 s))
     ;; bool _ins_solution_good
       (if _ins_solution_good (write-byte -1 s) (write-byte 0 s))
     ;; bool _ins_initialized
       (if _ins_initialized (write-byte -1 s) (write-byte 0 s))
     ;; uint8 _ins_status
       (write-byte _ins_status s)
     ;; uint8 _ins_vehicle_align
       (write-byte _ins_vehicle_align s)
     ;; uint8 _ins_heading_flag
       (write-byte _ins_heading_flag s)
     ;; int32 _ins_position_status
       (write-long _ins_position_status s)
     ;; bool _fast_lio_initialized
       (if _fast_lio_initialized (write-byte -1 s) (write-byte 0 s))
     ;; bool _fusion_generation_valid
       (if _fusion_generation_valid (write-byte -1 s) (write-byte 0 s))
     ;; uint32 _fusion_generation_sec
       (write-long _fusion_generation_sec s)
     ;; uint32 _fusion_generation_nsec
       (write-long _fusion_generation_nsec s)
     ;; bool _lio_generation_valid
       (if _lio_generation_valid (write-byte -1 s) (write-byte 0 s))
     ;; uint32 _lio_generation_sec
       (write-long _lio_generation_sec s)
     ;; uint32 _lio_generation_nsec
       (write-long _lio_generation_nsec s)
     ;; bool _map_origin_ready
       (if _map_origin_ready (write-byte -1 s) (write-byte 0 s))
     ;; bool _map_origin_committed
       (if _map_origin_committed (write-byte -1 s) (write-byte 0 s))
     ;; bool _alignment_ready
       (if _alignment_ready (write-byte -1 s) (write-byte 0 s))
     ;; bool _alignment_locked
       (if _alignment_locked (write-byte -1 s) (write-byte 0 s))
     ;; bool _imu_odometry_ready
       (if _imu_odometry_ready (write-byte -1 s) (write-byte 0 s))
     ;; bool _gps_good
       (if _gps_good (write-byte -1 s) (write-byte 0 s))
     ;; bool _lio_good
       (if _lio_good (write-byte -1 s) (write-byte 0 s))
     ;; string _active_map
       (write-long (length _active_map) s) (princ _active_map s)
     ;; string _pending_map
       (write-long (length _pending_map) s) (princ _pending_map s)
     ;; string _last_created_map
       (write-long (length _last_created_map) s) (princ _last_created_map s)
     ;; string _active_map_uuid
       (write-long (length _active_map_uuid) s) (princ _active_map_uuid s)
     ;; string _pending_map_uuid
       (write-long (length _pending_map_uuid) s) (princ _pending_map_uuid s)
     ;; uint32 _map_schema_version
       (write-long _map_schema_version s)
     ;; string _coordinate_frame
       (write-long (length _coordinate_frame) s) (princ _coordinate_frame s)
     ;; string _origin_checksum
       (write-long (length _origin_checksum) s) (princ _origin_checksum s)
     ;; bool _map_valid
       (if _map_valid (write-byte -1 s) (write-byte 0 s))
     ;; bool _algorithm_status_valid
       (if _algorithm_status_valid (write-byte -1 s) (write-byte 0 s))
     ;; bool _transitioning
       (if _transitioning (write-byte -1 s) (write-byte 0 s))
     ;; bool _restart_in_progress
       (if _restart_in_progress (write-byte -1 s) (write-byte 0 s))
     ;; uint8 _restart_scope
       (write-byte _restart_scope s)
     ;; bool _watchdog_position_timeout
       (if _watchdog_position_timeout (write-byte -1 s) (write-byte 0 s))
     ;; bool _watchdog_low_frequency
       (if _watchdog_low_frequency (write-byte -1 s) (write-byte 0 s))
     ;; bool _automatic_restart_suspended
       (if _automatic_restart_suspended (write-byte -1 s) (write-byte 0 s))
     ;; uint32 _automatic_restart_count
       (write-long _automatic_restart_count s)
     ;; bool _stop_required
       (if _stop_required (write-byte -1 s) (write-byte 0 s))
     ;; uint16 _fault_code
       (write-word _fault_code s)
     ;; string _reason
       (write-long (length _reason) s) (princ _reason s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; std_msgs/Header _header
     (send _header :deserialize buf ptr-) (incf ptr- (send _header :serialization-length))
   ;; string _reporter
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _reporter (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; uint8 _requested_mode
     (setq _requested_mode (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _active_mode
     (setq _active_mode (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _workflow_mode
     (setq _workflow_mode (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _requested_runtime_mode
     (setq _requested_runtime_mode (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _active_runtime_mode
     (setq _active_runtime_mode (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _phase
     (setq _phase (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _position_source
     (setq _position_source (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint32 _position_state
     (setq _position_state (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; bool _position_valid
     (setq _position_valid (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; bool _ins_status_received
     (setq _ins_status_received (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; bool _ins_solution_good
     (setq _ins_solution_good (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; bool _ins_initialized
     (setq _ins_initialized (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; uint8 _ins_status
     (setq _ins_status (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _ins_vehicle_align
     (setq _ins_vehicle_align (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _ins_heading_flag
     (setq _ins_heading_flag (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; int32 _ins_position_status
     (setq _ins_position_status (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; bool _fast_lio_initialized
     (setq _fast_lio_initialized (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; bool _fusion_generation_valid
     (setq _fusion_generation_valid (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; uint32 _fusion_generation_sec
     (setq _fusion_generation_sec (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; uint32 _fusion_generation_nsec
     (setq _fusion_generation_nsec (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; bool _lio_generation_valid
     (setq _lio_generation_valid (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; uint32 _lio_generation_sec
     (setq _lio_generation_sec (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; uint32 _lio_generation_nsec
     (setq _lio_generation_nsec (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; bool _map_origin_ready
     (setq _map_origin_ready (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; bool _map_origin_committed
     (setq _map_origin_committed (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; bool _alignment_ready
     (setq _alignment_ready (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; bool _alignment_locked
     (setq _alignment_locked (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; bool _imu_odometry_ready
     (setq _imu_odometry_ready (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; bool _gps_good
     (setq _gps_good (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; bool _lio_good
     (setq _lio_good (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; string _active_map
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _active_map (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; string _pending_map
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _pending_map (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; string _last_created_map
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _last_created_map (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; string _active_map_uuid
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _active_map_uuid (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; string _pending_map_uuid
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _pending_map_uuid (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; uint32 _map_schema_version
     (setq _map_schema_version (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; string _coordinate_frame
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _coordinate_frame (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; string _origin_checksum
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _origin_checksum (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; bool _map_valid
     (setq _map_valid (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; bool _algorithm_status_valid
     (setq _algorithm_status_valid (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; bool _transitioning
     (setq _transitioning (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; bool _restart_in_progress
     (setq _restart_in_progress (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; uint8 _restart_scope
     (setq _restart_scope (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; bool _watchdog_position_timeout
     (setq _watchdog_position_timeout (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; bool _watchdog_low_frequency
     (setq _watchdog_low_frequency (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; bool _automatic_restart_suspended
     (setq _automatic_restart_suspended (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; uint32 _automatic_restart_count
     (setq _automatic_restart_count (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; bool _stop_required
     (setq _stop_required (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; uint16 _fault_code
     (setq _fault_code (sys::peek buf ptr- :short)) (incf ptr- 2)
   ;; string _reason
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _reason (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;;
   self)
  )

(setf (get util::LocalizationStatus :md5sum-) "66d83c808f19f476c7a0fb306057009c")
(setf (get util::LocalizationStatus :datatype-) "util/LocalizationStatus")
(setf (get util::LocalizationStatus :definition-)
      "# Unified localization lifecycle status.  fusion_4dof publishes the algorithm
# view on /Mower/localization_algorithm_status; android_manager_4dof publishes
# the authoritative aggregate view on /Mower/localization_status.  Both topics
# are latched.
Header header

uint8 MODE_UNKNOWN=0
uint8 MODE_IDLE=1
uint8 MODE_MAPPING=2
uint8 MODE_LOCALIZATION=3

# The user-visible workflow and the algorithm process runtime are deliberately
# separate. During a boundary-recording workflow, a recovery may restart fusion
# in TRACK_SAVED_ORIGIN without ending the MAPPING workflow.
uint8 WORKFLOW_UNKNOWN=0
uint8 WORKFLOW_MAPPING=1
uint8 WORKFLOW_LOCALIZATION=2

uint8 RUNTIME_UNKNOWN=0
uint8 RUNTIME_CREATE_ORIGIN=1
uint8 RUNTIME_TRACK_SAVED_ORIGIN=2

uint8 PHASE_UNKNOWN=0
uint8 PHASE_WAIT_FAST_LIO=1
uint8 PHASE_WAIT_INS=2
uint8 PHASE_WAIT_MAP=3
uint8 PHASE_WAIT_GPS=4
uint8 PHASE_WAIT_ALIGNMENT=5
uint8 PHASE_READY_RTK=6
uint8 PHASE_READY_LIO_LOCKED=7
uint8 PHASE_READY_LIO_SEEDED=8
uint8 PHASE_READY_IMU_RESTRICTED=9
uint8 PHASE_SWITCHING_MAP=10
uint8 PHASE_RESTARTING_FUSION=11
uint8 PHASE_RESTARTING_ALL=12
uint8 PHASE_RECOVERING_LIO=13
uint8 PHASE_FAULT=14
uint8 PHASE_WAIT_IMU_ODOMETRY=15
uint8 PHASE_RESTART_VERIFYING=16
uint8 PHASE_WAIT_ALGORITHM=17
uint8 PHASE_WAIT_TRUSTED_OUTPUT=18

uint8 SOURCE_NONE=0
uint8 SOURCE_RTK=1
uint8 SOURCE_LIO_LOCKED=2
uint8 SOURCE_LIO_SEEDED=3
uint8 SOURCE_IMU_RESTRICTED=4

uint8 RESTART_NONE=0
uint8 RESTART_FUSION=1
uint8 RESTART_LIO=2
uint8 RESTART_ALL=3

uint16 FAULT_NONE=0
uint16 FAULT_MAP_INVALID=1
uint16 FAULT_RESTART_FAILED=2
uint16 FAULT_RESTART_SUPPRESSED=3
uint16 FAULT_POSITION_TIMEOUT=4
uint16 FAULT_LOW_FREQUENCY=5
uint16 FAULT_ALGORITHM=6
uint16 FAULT_RESTART_PARTIAL=7

# Publisher identity and legacy process-mode fields. requested_mode/active_mode
# are retained for compatibility; use workflow_mode plus active_runtime_mode for
# unambiguous new integrations.
string reporter
uint8 requested_mode
uint8 active_mode
uint8 workflow_mode
uint8 requested_runtime_mode
uint8 active_runtime_mode
uint8 phase

# Current production position semantics.
uint8 position_source
uint32 position_state
bool position_valid

# INS readiness. Both ins_solution_good and the compatibility field
# ins_initialized mean that a fresh receiver status has INS_Status==3.
# ins_position_status remains the 1ant publisher's motion-derived heading
# consistency diagnostic; it is not a localization startup gate.
bool ins_status_received
bool ins_solution_good
bool ins_initialized
uint8 ins_status
uint8 ins_vehicle_align
uint8 ins_heading_flag
int32 ins_position_status

# FAST-LIO and alignment truth owned by fusion_4dof.
bool fast_lio_initialized
# Process-generation identity is wall-clock based and used only for equality,
# never for message-age calculations. It lets the manager distinguish a new
# fusion process from a verification failure in the restart wrapper.
bool fusion_generation_valid
uint32 fusion_generation_sec
uint32 fusion_generation_nsec
bool lio_generation_valid
# Process identity may become valid before static calibration. Consumers must
# use fast_lio_initialized for pose readiness.
uint32 lio_generation_sec
uint32 lio_generation_nsec
bool map_origin_ready
bool map_origin_committed
bool alignment_ready
bool alignment_locked
bool imu_odometry_ready
bool gps_good
bool lio_good

# fusion_4dof owns runtime map identity; android_manager_4dof adds requested
# lifecycle state, last-created identity, restart and watchdog ownership.
string active_map
string pending_map
string last_created_map
string active_map_uuid
string pending_map_uuid
uint32 map_schema_version
string coordinate_frame
string origin_checksum
bool map_valid
bool algorithm_status_valid
bool transitioning
bool restart_in_progress
uint8 restart_scope
bool watchdog_position_timeout
bool watchdog_low_frequency
bool automatic_restart_suspended
uint32 automatic_restart_count
bool stop_required
uint16 fault_code
string reason

================================================================================
MSG: std_msgs/Header
# Standard metadata for higher-level stamped data types.
# This is generally used to communicate timestamped data 
# in a particular coordinate frame.
# 
# sequence ID: consecutively increasing ID 
uint32 seq
#Two-integer timestamp that is expressed as:
# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')
# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')
# time-handling sugar is provided by the client library
time stamp
#Frame this data is associated with
string frame_id

")



(provide :util/LocalizationStatus "66d83c808f19f476c7a0fb306057009c")


