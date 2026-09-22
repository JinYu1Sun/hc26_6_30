#include <cmath>
#include <cstddef>
#include <math.h>
#include <deque>
#include <mutex>
#include <thread>
#include <string>
#include <fstream>
#include <csignal>
#include <ros/ros.h>
#include <so3_math.h>
#include <Eigen/Eigen>
#include <common_lib.h>
#include <pcl/common/io.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <condition_variable>
#include <nav_msgs/Odometry.h>
#include <pcl/common/transforms.h>
#include <pcl/kdtree/kdtree_flann.h>
#include <tf/transform_broadcaster.h>
#include <eigen_conversions/eigen_msg.h>
#include <pcl_conversions/pcl_conversions.h>
#include <sensor_msgs/Imu.h>
#include <sensor_msgs/PointCloud2.h>
#include <geometry_msgs/Vector3.h>
#include "use-ikfom.hpp"
#include "imu_static_detector.h"

/// *************Preconfiguration

const bool time_list(PointType &x, PointType &y) { return (x.curvature < y.curvature); };

/// *************IMU Process and undistortion
class ImuProcess
{
public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  ImuProcess();
  ~ImuProcess();

  void Reset();
  void Reset(double start_timestamp, const sensor_msgs::ImuConstPtr &lastimu);
  void set_extrinsic(const V3D &transl, const M3D &rot);
  void set_extrinsic(const V3D &transl);
  void set_extrinsic(const MD(4, 4) & T);
  void set_gyr_cov(const V3D &scaler);
  void set_acc_cov(const V3D &scaler);
  void set_gyr_bias_cov(const V3D &b_g);
  void set_acc_bias_cov(const V3D &b_a);
  bool set_static_init_config(const fast_lio::ImuStaticDetector::Config &config);
  bool IsInitialized() const { return !imu_need_init_; }
  bool GetStaticInitializationStats(
      V3D *mean_acceleration, V3D *mean_angular_velocity,
      V3D *acceleration_variance, V3D *angular_velocity_variance,
      double *duration_sec, std::size_t *sample_count,
      double *max_angular_velocity_norm, double *last_imu_stamp,
      std::string *imu_frame_id) const;
  Eigen::Matrix<double, 12, 12> Q;
  void Process(const MeasureGroup &meas, esekfom::esekf<state_ikfom, 12, input_ikfom> &kf_state, PointCloudXYZI::Ptr pcl_un_);

  ofstream fout_imu;
  V3D cov_acc;
  V3D cov_gyr;
  V3D cov_acc_scale;
  V3D cov_gyr_scale;
  V3D cov_bias_gyr;
  V3D cov_bias_acc;
  double first_lidar_time;

private:
  void IMU_init(const MeasureGroup &meas, esekfom::esekf<state_ikfom, 12, input_ikfom> &kf_state, int &N);
  void UndistortPcl(const MeasureGroup &meas, esekfom::esekf<state_ikfom, 12, input_ikfom> &kf_state, PointCloudXYZI &pcl_in_out);

  PointCloudXYZI::Ptr cur_pcl_un_;
  sensor_msgs::ImuConstPtr last_imu_;
  deque<sensor_msgs::ImuConstPtr> v_imu_;
  vector<Pose6D> IMUpose;
  vector<M3D> v_rot_pcl_;
  M3D Lidar_R_wrt_IMU;
  V3D Lidar_T_wrt_IMU;
  V3D mean_acc;
  V3D mean_gyr;
  V3D angvel_last;
  V3D acc_s_last;
  double start_timestamp_;
  double last_lidar_end_time_;
  int init_iter_num = 1;
  bool b_first_frame_ = true;
  bool imu_need_init_ = true;
  fast_lio::ImuStaticDetector static_detector_;
};

ImuProcess::ImuProcess()
    : first_lidar_time(0.0), start_timestamp_(-1.0),
      last_lidar_end_time_(-1.0), b_first_frame_(true), imu_need_init_(true)
{
  init_iter_num = 1;
  static_detector_.Reset();
  Q = process_noise_cov();
  cov_acc = V3D(0.1, 0.1, 0.1);
  cov_gyr = V3D(0.1, 0.1, 0.1);
  cov_bias_gyr = V3D(0.0001, 0.0001, 0.0001);
  cov_bias_acc = V3D(0.0001, 0.0001, 0.0001);
  mean_acc = V3D(0, 0, -1.0);
  mean_gyr = V3D(0, 0, 0);
  angvel_last = Zero3d;
  Lidar_T_wrt_IMU = Zero3d;
  Lidar_R_wrt_IMU = Eye3d;
  last_imu_.reset(new sensor_msgs::Imu());
}

ImuProcess::~ImuProcess() {}

void ImuProcess::Reset()
{
  // ROS_WARN("Reset ImuProcess");
  mean_acc = V3D(0, 0, -1.0);
  mean_gyr = V3D(0, 0, 0);
  angvel_last = Zero3d;
  imu_need_init_ = true;
  start_timestamp_ = -1;
  last_lidar_end_time_ = -1.0;
  init_iter_num = 1;
  static_detector_.Reset();
  cov_acc.setZero();
  cov_gyr.setZero();
  v_imu_.clear();
  IMUpose.clear();
  last_imu_.reset(new sensor_msgs::Imu());
  cur_pcl_un_.reset(new PointCloudXYZI());
}

void ImuProcess::set_extrinsic(const MD(4, 4) & T)
{
  Lidar_T_wrt_IMU = T.block<3, 1>(0, 3);
  Lidar_R_wrt_IMU = T.block<3, 3>(0, 0);
}

void ImuProcess::set_extrinsic(const V3D &transl)
{
  Lidar_T_wrt_IMU = transl;
  Lidar_R_wrt_IMU.setIdentity();
}

void ImuProcess::set_extrinsic(const V3D &transl, const M3D &rot)
{
  Lidar_T_wrt_IMU = transl;
  Lidar_R_wrt_IMU = rot;
}

void ImuProcess::set_gyr_cov(const V3D &scaler)
{
  cov_gyr_scale = scaler;
}

void ImuProcess::set_acc_cov(const V3D &scaler)
{
  cov_acc_scale = scaler;
}

void ImuProcess::set_gyr_bias_cov(const V3D &b_g)
{
  cov_bias_gyr = b_g;
}

void ImuProcess::set_acc_bias_cov(const V3D &b_a)
{
  cov_bias_acc = b_a;
}

bool ImuProcess::set_static_init_config(
    const fast_lio::ImuStaticDetector::Config &config)
{
  return static_detector_.Configure(config);
}

bool ImuProcess::GetStaticInitializationStats(
    V3D *mean_acceleration, V3D *mean_angular_velocity,
    V3D *acceleration_variance, V3D *angular_velocity_variance,
    double *duration_sec, std::size_t *sample_count,
    double *max_angular_velocity_norm, double *last_imu_stamp,
    std::string *imu_frame_id) const
{
  if (imu_need_init_ || !static_detector_.confirmed() ||
      mean_acceleration == nullptr || mean_angular_velocity == nullptr ||
      acceleration_variance == nullptr ||
      angular_velocity_variance == nullptr || duration_sec == nullptr ||
      sample_count == nullptr || max_angular_velocity_norm == nullptr ||
      last_imu_stamp == nullptr || imu_frame_id == nullptr ||
      last_imu_ == nullptr)
    return false;

  *mean_acceleration = static_detector_.mean_acc();
  *mean_angular_velocity = static_detector_.mean_gyro();
  *acceleration_variance = static_detector_.acc_variance();
  *angular_velocity_variance = static_detector_.gyro_variance();
  *duration_sec = static_detector_.elapsed_sec();
  *sample_count = static_detector_.sample_count();
  *max_angular_velocity_norm = static_detector_.max_gyro_norm_seen();
  *last_imu_stamp = static_detector_.last_stamp();
  *imu_frame_id = last_imu_->header.frame_id;
  return mean_acceleration->allFinite() &&
         mean_angular_velocity->allFinite() &&
         acceleration_variance->allFinite() &&
         angular_velocity_variance->allFinite() &&
         std::isfinite(*duration_sec) && *duration_sec > 0.0 &&
         *sample_count >= 2 && std::isfinite(*max_angular_velocity_norm) &&
         std::isfinite(*last_imu_stamp) && *last_imu_stamp > 0.0;
}

void ImuProcess::IMU_init(const MeasureGroup &meas, esekfom::esekf<state_ikfom, 12, input_ikfom> &kf_state, int &N)
{
  /** 1. initializing the gravity, gyro bias, acc and gyro covariance
   ** 2. normalize the acceleration measurenments to unit gravity **/

  V3D cur_acc, cur_gyr;

  if (b_first_frame_)
  {
    Reset();
    b_first_frame_ = false;
    first_lidar_time = meas.lidar_beg_time;
  }

  for (const auto &imu : meas.imu)
  {
    const auto &imu_acc = imu->linear_acceleration;
    const auto &gyr_acc = imu->angular_velocity;
    cur_acc << imu_acc.x, imu_acc.y, imu_acc.z;
    cur_gyr << gyr_acc.x, gyr_acc.y, gyr_acc.z;
    static_detector_.AddSample(imu->header.stamp.toSec(), cur_acc, cur_gyr);
  }

  N = static_cast<int>(static_detector_.sample_count()) + 1;
  if (static_detector_.sample_count() == 0)
  {
    last_imu_ = meas.imu.back();
    return;
  }
  mean_acc = static_detector_.mean_acc();
  mean_gyr = static_detector_.mean_gyro();
  cov_acc = static_detector_.acc_variance();
  cov_gyr = static_detector_.gyro_variance();

  if (!mean_acc.allFinite() || mean_acc.norm() < 1e-6)
  {
    static_detector_.Reset();
    last_imu_ = meas.imu.back();
    return;
  }
  state_ikfom init_state = kf_state.get_x();
  init_state.grav = S2(-mean_acc / mean_acc.norm() * G_m_s2);

  // state_inout.rot = Eye3d; // Exp(mean_acc.cross(V3D(0, 0, -1 / scale_gravity)));
  init_state.bg = mean_gyr;
  init_state.offset_T_L_I = Lidar_T_wrt_IMU;
  init_state.offset_R_L_I = Lidar_R_wrt_IMU;
  kf_state.change_x(init_state);

  esekfom::esekf<state_ikfom, 12, input_ikfom>::cov init_P = kf_state.get_P();
  init_P.setIdentity();
  init_P(6, 6) = init_P(7, 7) = init_P(8, 8) = 0.00001;
  init_P(9, 9) = init_P(10, 10) = init_P(11, 11) = 0.00001;
  init_P(15, 15) = init_P(16, 16) = init_P(17, 17) = 0.0001;
  init_P(18, 18) = init_P(19, 19) = init_P(20, 20) = 0.001;
  init_P(21, 21) = init_P(22, 22) = 0.00001;
  kf_state.change_P(init_P);
  last_imu_ = meas.imu.back();
  // Initialization consumes this scan's IMU interval without undistorting its
  // point cloud. Seed the first post-initialization propagation boundary with
  // the last consumed LiDAR end time; leaving it uninitialized can generate an
  // arbitrary first dt and corrupt the freshly estimated gravity/bias state.
  last_lidar_end_time_ = meas.lidar_end_time;
}

void ImuProcess::UndistortPcl(const MeasureGroup &meas, esekfom::esekf<state_ikfom, 12, input_ikfom> &kf_state, PointCloudXYZI &pcl_out)
{
  /*** add the imu of the last frame-tail to the of current frame-head ***/
  auto v_imu = meas.imu;
  v_imu.push_front(last_imu_);
  const double &imu_beg_time = v_imu.front()->header.stamp.toSec();
  const double &imu_end_time = v_imu.back()->header.stamp.toSec();
  const double &pcl_beg_time = meas.lidar_beg_time;
  const double &pcl_end_time = meas.lidar_end_time;

  /*** sort point clouds by offset time ***/
  pcl_out = *(meas.lidar);
  sort(pcl_out.points.begin(), pcl_out.points.end(), time_list);
  // cout<<"[ IMU Process ]: Process lidar from "<<pcl_beg_time<<" to "<<pcl_end_time<<", " \
  //          <<meas.imu.size()<<" imu msgs from "<<imu_beg_time<<" to "<<imu_end_time<<endl;

  /*** Initialize IMU pose ***/
  state_ikfom imu_state = kf_state.get_x();
  IMUpose.clear();
  IMUpose.push_back(set_pose6d(0.0, acc_s_last, angvel_last, imu_state.vel, imu_state.pos, imu_state.rot.toRotationMatrix()));

  /*** forward propagation at each imu point ***/
  V3D angvel_avr, acc_avr, acc_imu, vel_imu, pos_imu;
  M3D R_imu;

  double dt = 0;

  input_ikfom in;
  for (auto it_imu = v_imu.begin(); it_imu < (v_imu.end() - 1); it_imu++)
  {
    auto &&head = *(it_imu);
    auto &&tail = *(it_imu + 1);

    if (tail->header.stamp.toSec() < last_lidar_end_time_)
      continue;

    angvel_avr << 0.5 * (head->angular_velocity.x + tail->angular_velocity.x),
        0.5 * (head->angular_velocity.y + tail->angular_velocity.y),
        0.5 * (head->angular_velocity.z + tail->angular_velocity.z);
    acc_avr << 0.5 * (head->linear_acceleration.x + tail->linear_acceleration.x),
        0.5 * (head->linear_acceleration.y + tail->linear_acceleration.y),
        0.5 * (head->linear_acceleration.z + tail->linear_acceleration.z);

    // fout_imu << setw(10) << head->header.stamp.toSec() - first_lidar_time << " " << angvel_avr.transpose() << " " << acc_avr.transpose() << endl;

    acc_avr = acc_avr * G_m_s2 / mean_acc.norm(); // - state_inout.ba;

    if (head->header.stamp.toSec() < last_lidar_end_time_)
    {
      dt = tail->header.stamp.toSec() - last_lidar_end_time_;
      // dt = tail->header.stamp.toSec() - pcl_beg_time;
    }
    else
    {
      dt = tail->header.stamp.toSec() - head->header.stamp.toSec();
    }

    in.acc = acc_avr;
    in.gyro = angvel_avr;
    Q.block<3, 3>(0, 0).diagonal() = cov_gyr;
    Q.block<3, 3>(3, 3).diagonal() = cov_acc;
    Q.block<3, 3>(6, 6).diagonal() = cov_bias_gyr;
    Q.block<3, 3>(9, 9).diagonal() = cov_bias_acc;
    kf_state.predict(dt, Q, in);

    /* save the poses at each IMU measurements */
    imu_state = kf_state.get_x();
    angvel_last = angvel_avr - imu_state.bg;
    acc_s_last = imu_state.rot * (acc_avr - imu_state.ba);
    for (int i = 0; i < 3; i++)
    {
      acc_s_last[i] += imu_state.grav[i];
    }
    double &&offs_t = tail->header.stamp.toSec() - pcl_beg_time;
    IMUpose.push_back(set_pose6d(offs_t, acc_s_last, angvel_last, imu_state.vel, imu_state.pos, imu_state.rot.toRotationMatrix()));
  }

  /*** calculated the pos and attitude prediction at the frame-end ***/
  double note = pcl_end_time > imu_end_time ? 1.0 : -1.0;
  dt = note * (pcl_end_time - imu_end_time);
  kf_state.predict(dt, Q, in);

  imu_state = kf_state.get_x();
  last_imu_ = meas.imu.back();
  last_lidar_end_time_ = pcl_end_time;

  /*** undistort each lidar point (backward propagation) ***/
  if (pcl_out.points.begin() == pcl_out.points.end())
    return;
  auto it_pcl = pcl_out.points.end() - 1;
  for (auto it_kp = IMUpose.end() - 1; it_kp != IMUpose.begin(); it_kp--)
  {
    auto head = it_kp - 1;
    auto tail = it_kp;
    R_imu << MAT_FROM_ARRAY(head->rot);
    // cout<<"head imu acc: "<<acc_imu.transpose()<<endl;
    vel_imu << VEC_FROM_ARRAY(head->vel);
    pos_imu << VEC_FROM_ARRAY(head->pos);
    acc_imu << VEC_FROM_ARRAY(tail->acc);
    angvel_avr << VEC_FROM_ARRAY(tail->gyr);

    for (; it_pcl->curvature / double(1000) > head->offset_time; it_pcl--)
    {
      dt = it_pcl->curvature / double(1000) - head->offset_time;

      /* Transform to the 'end' frame, using only the rotation
       * Note: Compensation direction is INVERSE of Frame's moving direction
       * So if we want to compensate a point at timestamp-i to the frame-e
       * P_compensate = R_imu_e ^ T * (R_i * P_i + T_ei) where T_ei is represented in global frame */
      M3D R_i(R_imu * Exp(angvel_avr, dt));

      V3D P_i(it_pcl->x, it_pcl->y, it_pcl->z);
      V3D T_ei(pos_imu + vel_imu * dt + 0.5 * acc_imu * dt * dt - imu_state.pos);
      V3D P_compensate = imu_state.offset_R_L_I.conjugate() * (imu_state.rot.conjugate() * (R_i * (imu_state.offset_R_L_I * P_i + imu_state.offset_T_L_I) + T_ei) - imu_state.offset_T_L_I); // not accurate!

      // save Undistorted points and their rotation
      it_pcl->x = P_compensate(0);
      it_pcl->y = P_compensate(1);
      it_pcl->z = P_compensate(2);

      if (it_pcl == pcl_out.points.begin())
        break;
    }
  }
}

void ImuProcess::Process(const MeasureGroup &meas, esekfom::esekf<state_ikfom, 12, input_ikfom> &kf_state, PointCloudXYZI::Ptr cur_pcl_un_)
{
  double t1, t2, t3;
  t1 = omp_get_wtime();

  if (meas.imu.empty())
  {
    return;
  };
  ROS_ASSERT(meas.lidar != nullptr);

  if (imu_need_init_)
  {
    /// The very first lidar frame
    IMU_init(meas, kf_state, init_iter_num);

    last_imu_ = meas.imu.back();

    if (static_detector_.confirmed())
    {
      const V3D measured_acc_variance = cov_acc;
      const V3D measured_gyro_variance = cov_gyr;
      cov_acc *= pow(G_m_s2 / mean_acc.norm(), 2);
      imu_need_init_ = false;

      cov_acc = cov_acc_scale;
      cov_gyr = cov_gyr_scale;
      ROS_INFO("IMU static initialization confirmed: duration=%.3fs samples=%zu "
               "gyro_mean_norm=%.6frad/s gyro_max=%.6frad/s "
               "acc_norm=%.6fm/s^2 acc_var=[%.8f %.8f %.8f] "
               "gyro_var=[%.8f %.8f %.8f].",
               static_detector_.elapsed_sec(), static_detector_.sample_count(),
               mean_gyr.norm(), static_detector_.max_gyro_norm_seen(),
               mean_acc.norm(), measured_acc_variance.x(),
               measured_acc_variance.y(), measured_acc_variance.z(),
               measured_gyro_variance.x(), measured_gyro_variance.y(),
               measured_gyro_variance.z());
      // ROS_INFO("IMU Initial Done: Gravity: %.4f %.4f %.4f %.4f; state.bias_g: %.4f %.4f %.4f; acc covarience: %.8f %.8f %.8f; gry covarience: %.8f %.8f %.8f",\
      //          imu_state.grav[0], imu_state.grav[1], imu_state.grav[2], mean_acc.norm(), cov_bias_gyr[0], cov_bias_gyr[1], cov_bias_gyr[2], cov_acc[0], cov_acc[1], cov_acc[2], cov_gyr[0], cov_gyr[1], cov_gyr[2]);
      fout_imu.open(DEBUG_FILE_DIR("imu.txt"), ios::out);
    }
    else
    {
      const V3D acc_var = static_detector_.acc_variance();
      ROS_WARN_THROTTLE(
          1.0,
          "Waiting for continuous static IMU initialization: %.2f/%.2fs, "
          "samples=%zu, gyro_mean=%.4f<=%.4f rad/s, gyro_max=%.4f<=%.4f "
          "rad/s, acc_norm=%.3f in [%.3f, %.3f] m/s^2, "
          "acc_var_max=%.5f, acc_var_gate=%s (limit=%.5f), "
          "last_reject=%s. Keep vehicle stopped; "
          "leveled pose invalid.",
          static_detector_.elapsed_sec(),
          static_detector_.config().confirmation_time_sec,
          static_detector_.sample_count(), mean_gyr.norm(),
          static_detector_.config().max_gyro_mean_norm,
          static_detector_.max_gyro_norm_seen(),
          static_detector_.config().max_gyro_norm, mean_acc.norm(),
          static_detector_.config().min_acc_norm,
          static_detector_.config().max_acc_norm, acc_var.maxCoeff(),
          static_detector_.config().max_acc_variance > 0.0 ? "enabled" :
                                                               "disabled",
          static_detector_.config().max_acc_variance,
          static_detector_.last_reject_reason_name());
    }

    return;
  }

  UndistortPcl(meas, kf_state, *cur_pcl_un_);

  t2 = omp_get_wtime();
  t3 = omp_get_wtime();

  // cout<<"[ IMU Process ]: Time: "<<t3 - t1<<endl;
}
