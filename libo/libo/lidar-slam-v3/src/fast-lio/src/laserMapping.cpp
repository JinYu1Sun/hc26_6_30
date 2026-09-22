// This is an advanced implementation of the algorithm described in the
// following paper:
//   J. Zhang and S. Singh. LOAM: Lidar Odometry and Mapping in Real-time.
//     Robotics: Science and Systems Conference (RSS). Berkeley, CA, July 2014.

// Modifier: Livox               dev@livoxtech.com

// Copyright 2013, Ji Zhang, Carnegie Mellon University
// Further contributions copyright (c) 2016, Southwest Research Institute
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from this
//    software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#include <omp.h>
#include <mutex>
#include <math.h>
#include <thread>
#include <fstream>
#include <csignal>
#include <unistd.h>
#include <Python.h>
#include <so3_math.h>
#include <ros/ros.h>
#include <Eigen/Core>
#include "IMU_Processing.hpp"
#include <nav_msgs/Odometry.h>
#include <nav_msgs/Path.h>
#include <visualization_msgs/Marker.h>
#include <pcl_conversions/pcl_conversions.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/io/pcd_io.h>
#include <sensor_msgs/PointCloud2.h>
#include <tf/transform_datatypes.h>
#include <tf/transform_broadcaster.h>
#include <geometry_msgs/Vector3.h>
#include <livox_ros_driver2/CustomMsg.h>
#include "preprocess.h"
#include "gravity_leveling.h"
#include <ikd-Tree/ikd_Tree.h>

// TODO: 新增重包装后的定位输出
#include <util/LIOPose.h>
#include <util/ImuStaticCalibration.h>
#include <geometry_msgs/TransformStamped.h>
#include <std_msgs/Bool.h>
#include <tf/LinearMath/Matrix3x3.h>

#define INIT_TIME (0.1)
#define LASER_POINT_COV (0.001)
#define MAXN (720000)
#define PUBFRAME_PERIOD (20)

/*** Time Log Variables ***/
double kdtree_incremental_time = 0.0, kdtree_search_time = 0.0, kdtree_delete_time = 0.0;
double T1[MAXN], s_plot[MAXN], s_plot2[MAXN], s_plot3[MAXN], s_plot4[MAXN], s_plot5[MAXN], s_plot6[MAXN], s_plot7[MAXN], s_plot8[MAXN], s_plot9[MAXN], s_plot10[MAXN], s_plot11[MAXN];
double match_time = 0, solve_time = 0, solve_const_H_time = 0;
int kdtree_size_st = 0, kdtree_size_end = 0, add_point_size = 0, kdtree_delete_counter = 0;
bool runtime_pos_log = false, pcd_save_en = false, time_sync_en = false, extrinsic_est_en = true, path_en = true;
/**************************/

float res_last[100000] = {0.0}; // 残差，点到面距离平方和
float DET_RANGE = 300.0f;
const float MOV_THRESHOLD = 1.5f;

mutex mtx_buffer;
condition_variable sig_buffer;

string root_dir = ROOT_DIR;
string map_file_path, lid_topic, imu_topic, resolved_imu_topic;

double res_mean_last = 0.05, total_residual = 0.0;
double last_timestamp_lidar = 0, last_timestamp_imu = -1.0;
double gyr_cov = 0.1, acc_cov = 0.1, b_gyr_cov = 0.0001, b_acc_cov = 0.0001;
double imu_acc_scale = 1.0;
double filter_size_corner_min = 0, filter_size_surf_min = 0, filter_size_map_min = 0, fov_deg = 0;
double cube_len = 0, HALF_FOV_COS = 0, FOV_DEG = 0, total_distance = 0, lidar_end_time = 0, first_lidar_time = 0.0;
int effct_feat_num = 0, time_log_counter = 0, scan_count = 0, publish_count = 0;
int iterCount = 0, feats_down_size = 0, NUM_MAX_ITERATIONS = 0, laserCloudValidNum = 0, pcd_save_interval = -1, pcd_index = 0;
bool point_selected_surf[100000] = {0}; // 是否为平面特征点
bool lidar_pushed, flg_first_scan = true, flg_exit = false, flg_EKF_inited;
bool scan_pub_en = false, dense_pub_en = false, scan_body_pub_en = false;

// /Mower/lio_slam_level output contract.  FAST-LIO continues estimating and
// mapping in its original per-process world W.  Exactly once after IMU
// initialization we derive and freeze R_H_W from the estimated gravity, then
// apply that rigid rotation only to the dedicated fusion pose output.
bool lio_gravity_level_en = true;
bool lio_diagnostic_log_en = true;
double lio_diagnostic_log_period_sec = 0.20;
bool lio_level_ready = false;
M3D lio_level_R_H_W(Eye3d);
double lio_level_tilt_rad = 0.0;
ros::Publisher pub_lio_static_initialized;
ros::Publisher pub_lio_static_calibration;
ros::Time lio_process_generation_stamp;

vector<vector<int>> pointSearchInd_surf;
vector<BoxPointType> cub_needrm; // ikd-tree中，地图需要移除的包围盒序列
vector<PointVector> Nearest_Points;
vector<double> extrinT(3, 0.0);
vector<double> extrinR(9, 0.0);
struct TimedLidarFrame
{
    PointCloudXYZI::Ptr cloud;
    double stamp;
};
// Point cloud and timestamp are one queue element. Keeping them in separate
// deques allowed a timestamp to survive a LiDAR rollback buffer clear and made
// every subsequent cloud appear one scan period older.
deque<TimedLidarFrame> lidar_buffer;
deque<sensor_msgs::Imu::ConstPtr> imu_buffer;
constexpr std::size_t kMaxLidarBufferSize = 32;
constexpr std::size_t kMaxImuBufferSize = 2048;

PointCloudXYZI::Ptr featsFromMap(new PointCloudXYZI());
PointCloudXYZI::Ptr feats_undistort(new PointCloudXYZI());
PointCloudXYZI::Ptr feats_down_body(new PointCloudXYZI());  // 畸变纠正后降采样的单帧点云，lidar系
PointCloudXYZI::Ptr feats_down_world(new PointCloudXYZI()); // 畸变纠正后降采样的单帧点云，w系
PointCloudXYZI::Ptr normvec(new PointCloudXYZI(100000, 1)); // 特征点在地图中对应点的，局部平面参数,w系
PointCloudXYZI::Ptr laserCloudOri(new PointCloudXYZI(100000, 1));
PointCloudXYZI::Ptr corr_normvect(new PointCloudXYZI(100000, 1)); // 对应点法相量？
PointCloudXYZI::Ptr _featsArray;                                  // ikd-tree中，map需要移除的点云

pcl::VoxelGrid<PointType> downSizeFilterSurf; // 单帧内降采样使用voxel grid
pcl::VoxelGrid<PointType> downSizeFilterMap;  // 未使用

KD_TREE<PointType> ikdtree;

V3F XAxisPoint_body(LIDAR_SP_LEN, 0.0, 0.0);
V3F XAxisPoint_world(LIDAR_SP_LEN, 0.0, 0.0);
V3D euler_cur;
V3D position_last(Zero3d);
V3D Lidar_T_wrt_IMU(Zero3d); // T lidar to imu (imu = r * lidar + t)
M3D Lidar_R_wrt_IMU(Eye3d);  // R lidar to imu (imu = r * lidar + t)

/*** EKF inputs and output ***/
MeasureGroup Measures;
esekfom::esekf<state_ikfom, 12, input_ikfom> kf; // 状态，噪声维度，输入
state_ikfom state_point;
vect3 pos_lid; // world系下lidar坐标

nav_msgs::Path path;
nav_msgs::Odometry odomAftMapped;
geometry_msgs::Quaternion geoQuat;
geometry_msgs::PoseStamped msg_body_pose;

shared_ptr<Preprocess> p_pre(new Preprocess());
shared_ptr<ImuProcess> p_imu(new ImuProcess());
ros::Publisher pubLaserCloudFiltered;

bool EnsureLioLevelInitialized(const ros::Publisher &pubLevelTransform)
{
    if (lio_level_ready)
        return true;

    // Even identity leveling is not valid before the same quality-gated IMU
    // initialization. This prevents disabling visualization leveling from
    // accidentally bypassing the static-start safety contract.
    if (!p_imu->IsInitialized())
        return false;

    if (!lio_gravity_level_en)
    {
        lio_level_R_H_W.setIdentity();
        lio_level_tilt_rad = 0.0;
    }
    else
    {
        // Do not infer leveling from a vehicle pose.  Only FAST-LIO's explicit
        // gravity state is valid here; this remains correct when initialization
        // happens on a slope because gravity, unlike the initial body attitude,
        // still defines the horizontal plane.
        const V3D gravity_W(state_point.grav[0], state_point.grav[1],
                            state_point.grav[2]);
        double alignment_error = 0.0;
        if (!fast_lio::ComputeGravityLevelRotation(
                gravity_W, &lio_level_R_H_W, 5.0, 15.0,
                &alignment_error, &lio_level_tilt_rad))
        {
            ROS_ERROR_THROTTLE(
                1.0,
                "Cannot initialize fixed LIO gravity leveling: gravity_W="
                "[%.6f %.6f %.6f]. Leveled LIO output remains invalid.",
                gravity_W.x(), gravity_W.y(), gravity_W.z());
            return false;
        }

        ROS_INFO("Fixed LIO gravity leveling initialized for this "
                 "/laserMapping generation: gravity_W=[%.6f %.6f %.6f], "
                 "tilt=%.3f deg, alignment_error=%.3e.",
                 gravity_W.x(), gravity_W.y(), gravity_W.z(),
                 lio_level_tilt_rad * 180.0 / M_PI, alignment_error);
        ROS_INFO("R_H_W=[%.9f %.9f %.9f; %.9f %.9f %.9f; %.9f %.9f %.9f]",
                 lio_level_R_H_W(0, 0), lio_level_R_H_W(0, 1),
                 lio_level_R_H_W(0, 2), lio_level_R_H_W(1, 0),
                 lio_level_R_H_W(1, 1), lio_level_R_H_W(1, 2),
                 lio_level_R_H_W(2, 0), lio_level_R_H_W(2, 1),
                 lio_level_R_H_W(2, 2));
    }

    // Export the exact detector window used by FAST-LIO. Fusion must not run a
    // second, independently timed static detector because the vehicle may be
    // released immediately after this authoritative initialization finishes.
    V3D mean_acc, mean_gyro, acc_variance, gyro_variance;
    double static_duration_sec = 0.0;
    double max_gyro_norm = 0.0;
    double last_static_imu_stamp = 0.0;
    std::size_t static_sample_count = 0;
    std::string static_imu_frame_id;
    if (!p_imu->GetStaticInitializationStats(
            &mean_acc, &mean_gyro, &acc_variance, &gyro_variance,
            &static_duration_sec, &static_sample_count, &max_gyro_norm,
            &last_static_imu_stamp, &static_imu_frame_id))
    {
        ROS_ERROR_THROTTLE(
            1.0, "FAST-LIO initialized but its static-window statistics are "
                 "invalid; keep leveled output and fusion calibration blocked.");
        return false;
    }

    util::ImuStaticCalibration calibration;
    calibration.header.stamp = ros::Time(last_static_imu_stamp);
    calibration.header.frame_id = static_imu_frame_id;
    calibration.valid = true;
    calibration.lio_generation = lio_process_generation_stamp;
    calibration.imu_topic = resolved_imu_topic;
    calibration.duration_sec = static_duration_sec;
    calibration.sample_count =
        static_cast<uint32_t>(static_sample_count);
    for (int i = 0; i < 3; ++i)
    {
        calibration.mean_acceleration_mps2[i] = mean_acc[i];
        calibration.mean_angular_velocity_radps[i] = mean_gyro[i];
        calibration.acceleration_variance[i] = acc_variance[i];
        calibration.angular_velocity_variance[i] = gyro_variance[i];
    }
    calibration.max_angular_velocity_norm_radps = max_gyro_norm;

    lio_level_ready = true; // Freeze for the complete process generation.

    // Latched diagnostic: rotation maps vectors in raw FAST-LIO world W into
    // gravity-level world H.  It is informational; fusion consumes the already
    // transformed pose and does not need to reconstruct this state.
    Eigen::Quaterniond q_H_W(lio_level_R_H_W);
    q_H_W.normalize();
    geometry_msgs::TransformStamped level_transform;
    // This latched transform also carries a per-process generation token.
    // Wall-clock startup time is used instead of the sensor stamp so a replay
    // or a new FAST-LIO process cannot accidentally reuse an old 4DoF state.
    level_transform.header.stamp = lio_process_generation_stamp;
    level_transform.header.frame_id = "lio_level";
    level_transform.child_frame_id = "lio_odom_raw";
    level_transform.transform.translation.x = 0.0;
    level_transform.transform.translation.y = 0.0;
    level_transform.transform.translation.z = 0.0;
    level_transform.transform.rotation.w = q_H_W.w();
    level_transform.transform.rotation.x = q_H_W.x();
    level_transform.transform.rotation.y = q_H_W.y();
    level_transform.transform.rotation.z = q_H_W.z();
    pubLevelTransform.publish(level_transform);
    pub_lio_static_calibration.publish(calibration);
    std_msgs::Bool initialized_msg;
    initialized_msg.data = true;
    pub_lio_static_initialized.publish(initialized_msg);
    return true;
}

void SigHandle(int sig)
{
    flg_exit = true;
    ROS_WARN("catch sig %d", sig);
    sig_buffer.notify_all();
}

inline void dump_lio_state_to_log(FILE *fp)
{
    V3D rot_ang(Log(state_point.rot.toRotationMatrix()));
    fprintf(fp, "%lf ", Measures.lidar_beg_time - first_lidar_time);
    fprintf(fp, "%lf %lf %lf ", rot_ang(0), rot_ang(1), rot_ang(2));                            // Angle
    fprintf(fp, "%lf %lf %lf ", state_point.pos(0), state_point.pos(1), state_point.pos(2));    // Pos
    fprintf(fp, "%lf %lf %lf ", 0.0, 0.0, 0.0);                                                 // omega
    fprintf(fp, "%lf %lf %lf ", state_point.vel(0), state_point.vel(1), state_point.vel(2));    // Vel
    fprintf(fp, "%lf %lf %lf ", 0.0, 0.0, 0.0);                                                 // Acc
    fprintf(fp, "%lf %lf %lf ", state_point.bg(0), state_point.bg(1), state_point.bg(2));       // Bias_g
    fprintf(fp, "%lf %lf %lf ", state_point.ba(0), state_point.ba(1), state_point.ba(2));       // Bias_a
    fprintf(fp, "%lf %lf %lf ", state_point.grav[0], state_point.grav[1], state_point.grav[2]); // Bias_a
    fprintf(fp, "\r\n");
    fflush(fp);
}

void pointBodyToWorld_ikfom(PointType const *const pi, PointType *const po, state_ikfom &s)
{
    V3D p_body(pi->x, pi->y, pi->z);
    V3D p_global(s.rot * (s.offset_R_L_I * p_body + s.offset_T_L_I) + s.pos);

    po->x = p_global(0);
    po->y = p_global(1);
    po->z = p_global(2);
    po->intensity = pi->intensity;
}

// 按当前body(lidar)的状态，将局部点转换到世界系下
void pointBodyToWorld(PointType const *const pi, PointType *const po)
{
    V3D p_body(pi->x, pi->y, pi->z);
    // world <-- imu <-- lidar
    V3D p_global(state_point.rot * (state_point.offset_R_L_I * p_body + state_point.offset_T_L_I) + state_point.pos);

    po->x = p_global(0);
    po->y = p_global(1);
    po->z = p_global(2);
    po->intensity = pi->intensity;
}

template <typename T>
void pointBodyToWorld(const Matrix<T, 3, 1> &pi, Matrix<T, 3, 1> &po)
{
    V3D p_body(pi[0], pi[1], pi[2]);
    V3D p_global(state_point.rot * (state_point.offset_R_L_I * p_body + state_point.offset_T_L_I) + state_point.pos);

    po[0] = p_global(0);
    po[1] = p_global(1);
    po[2] = p_global(2);
}

void RGBpointBodyToWorld(PointType const *const pi, PointType *const po)
{
    V3D p_body(pi->x, pi->y, pi->z);
    V3D p_global(state_point.rot * (state_point.offset_R_L_I * p_body + state_point.offset_T_L_I) + state_point.pos);

    po->x = p_global(0);
    po->y = p_global(1);
    po->z = p_global(2);
    po->intensity = pi->intensity;
}

void RGBpointBodyLidarToIMU(PointType const *const pi, PointType *const po)
{
    V3D p_body_lidar(pi->x, pi->y, pi->z);
    V3D p_body_imu(state_point.offset_R_L_I * p_body_lidar + state_point.offset_T_L_I);

    po->x = p_body_imu(0);
    po->y = p_body_imu(1);
    po->z = p_body_imu(2);
    po->intensity = pi->intensity;
}

void points_cache_collect()
{
    PointVector points_history;
    ikdtree.acquire_removed_points(points_history);
    // for (int i = 0; i < points_history.size(); i++) _featsArray->push_back(points_history[i]);
}

// 根据lidar的FoV分割场景
BoxPointType LocalMap_Points; // ikd-tree中,局部地图的包围盒角点
bool Localmap_Initialized = false;
void lasermap_fov_segment()
{
    cub_needrm.clear(); // 清空需要移除的区域
    kdtree_delete_counter = 0;
    kdtree_delete_time = 0.0;
    pointBodyToWorld(XAxisPoint_body, XAxisPoint_world); // X轴分界点转换到w系下
    V3D pos_LiD = pos_lid;                               // global系lidar位置

    // 初始化局部地图包围盒角点，以为w系下lidar位置为中心
    if (!Localmap_Initialized)
    {
        for (int i = 0; i < 3; i++)
        {
            LocalMap_Points.vertex_min[i] = pos_LiD(i) - cube_len / 2.0;
            LocalMap_Points.vertex_max[i] = pos_LiD(i) + cube_len / 2.0;
        }
        Localmap_Initialized = true;
        return;
    }

    float dist_to_map_edge[3][2]; // 各个方向与局部地图边界的距离
    bool need_move = false;
    for (int i = 0; i < 3; i++)
    {
        dist_to_map_edge[i][0] = fabs(pos_LiD(i) - LocalMap_Points.vertex_min[i]);
        dist_to_map_edge[i][1] = fabs(pos_LiD(i) - LocalMap_Points.vertex_max[i]);

        // 与某个方向上的边界距离太小，标记需要移除need_move
        if (dist_to_map_edge[i][0] <= MOV_THRESHOLD * DET_RANGE || dist_to_map_edge[i][1] <= MOV_THRESHOLD * DET_RANGE)
            need_move = true;
    }

    // 不需要移除则直接返回
    if (!need_move)
        return;

    BoxPointType New_LocalMap_Points, tmp_boxpoints;
    New_LocalMap_Points = LocalMap_Points; // 新的局部地图角点
    float mov_dist = max((cube_len - 2.0 * MOV_THRESHOLD * DET_RANGE) * 0.5 * 0.9, double(DET_RANGE * (MOV_THRESHOLD - 1)));
    for (int i = 0; i < 3; i++)
    {
        tmp_boxpoints = LocalMap_Points;
        // 与包围盒最小值角点距离
        if (dist_to_map_edge[i][0] <= MOV_THRESHOLD * DET_RANGE)
        {
            New_LocalMap_Points.vertex_max[i] -= mov_dist;
            New_LocalMap_Points.vertex_min[i] -= mov_dist;
            tmp_boxpoints.vertex_min[i] = LocalMap_Points.vertex_max[i] - mov_dist;
            cub_needrm.push_back(tmp_boxpoints); // 移除较远包围盒
        }
        else if (dist_to_map_edge[i][1] <= MOV_THRESHOLD * DET_RANGE)
        {
            New_LocalMap_Points.vertex_max[i] += mov_dist;
            New_LocalMap_Points.vertex_min[i] += mov_dist;
            tmp_boxpoints.vertex_max[i] = LocalMap_Points.vertex_min[i] + mov_dist;
            cub_needrm.push_back(tmp_boxpoints);
        }
    }
    LocalMap_Points = New_LocalMap_Points;

    points_cache_collect();
    double delete_begin = omp_get_wtime();
    if (cub_needrm.size() > 0)
        kdtree_delete_counter = ikdtree.Delete_Point_Boxes(cub_needrm);
    kdtree_delete_time = omp_get_wtime() - delete_begin;
}

void standard_pcl_cbk(const sensor_msgs::PointCloud2::ConstPtr &msg)
{
    std::lock_guard<std::mutex> lock(mtx_buffer);
    scan_count++;
    double preprocess_start_time = omp_get_wtime();
    if (msg->header.stamp.toSec() < last_timestamp_lidar)
    {
        // Do not feed a regressed sensor epoch into the running EKF. An
        // isolated out-of-order packet is harmlessly dropped; a persistent
        // clock rollback stops fresh LIO output and lets the lifecycle manager
        // perform a process-generation reset.
        ROS_ERROR("lidar timestamp regressed (current=%.9f last=%.9f); drop frame",
                  msg->header.stamp.toSec(), last_timestamp_lidar);
        return;
    }
    if (lidar_buffer.size() >= kMaxLidarBufferSize)
    {
        ROS_ERROR_THROTTLE(1.0, "LiDAR processing buffer is full; drop incoming frame");
        return;
    }

    PointCloudXYZI::Ptr ptr(new PointCloudXYZI());
    p_pre->process(msg, ptr);
    lidar_buffer.push_back({ptr, msg->header.stamp.toSec()});
    last_timestamp_lidar = msg->header.stamp.toSec();
    s_plot11[scan_count] = omp_get_wtime() - preprocess_start_time;
    sig_buffer.notify_all();
}

double timediff_lidar_wrt_imu = 0.0;
bool timediff_set_flg = false; // 标记是否已经进行了时间补偿
void livox_pcl_cbk(const livox_ros_driver2::CustomMsg::ConstPtr &msg)
{
    std::unique_lock<std::mutex> lock(mtx_buffer);
    double preprocess_start_time = omp_get_wtime();
    scan_count++;
    if (msg->header.stamp.toSec() < last_timestamp_lidar)
    {
        ROS_ERROR("lidar timestamp regressed (current=%.9f last=%.9f); drop frame",
                  msg->header.stamp.toSec(), last_timestamp_lidar);
        return;
    }
    if (lidar_buffer.size() >= kMaxLidarBufferSize)
    {
        ROS_ERROR_THROTTLE(1.0, "LiDAR processing buffer is full; drop incoming frame");
        return;
    }
    last_timestamp_lidar = msg->header.stamp.toSec();

    if (!time_sync_en && abs(last_timestamp_imu - last_timestamp_lidar) > 10.0 && !imu_buffer.empty() && !lidar_buffer.empty())
    {
        printf("IMU and LiDAR not Synced, IMU time: %lf, lidar header time: %lf \n", last_timestamp_imu, last_timestamp_lidar);
    }

    if (time_sync_en && !timediff_set_flg && abs(last_timestamp_lidar - last_timestamp_imu) > 1 && !imu_buffer.empty())
    {
        timediff_set_flg = true;
        timediff_lidar_wrt_imu = last_timestamp_lidar + 0.1 - last_timestamp_imu; //????
        printf("Self sync IMU and LiDAR, time diff is %.10lf \n", timediff_lidar_wrt_imu);
    }

    PointCloudXYZI::Ptr ptr(new PointCloudXYZI());

    // 特征提取或间隔采样
    p_pre->process(msg, ptr);
    lidar_buffer.push_back({ptr, last_timestamp_lidar}); // 点云和帧头时间原子入队

    s_plot11[scan_count] = omp_get_wtime() - preprocess_start_time;
    lock.unlock();

    // 原始帧头和 frame_id 原样保留；点顺序仍是 Livox 驱动输入顺序。
    sensor_msgs::PointCloud2 filtered_msg;
    pcl::toROSMsg(p_pre->pl_filtered, filtered_msg);
    filtered_msg.header = msg->header;
    pubLaserCloudFiltered.publish(filtered_msg);

    sig_buffer.notify_all();
}

void imu_cbk(const sensor_msgs::Imu::ConstPtr &msg_in)
{
    publish_count++;
    // cout<<"IMU got at: "<<msg_in->header.stamp.toSec()<<endl;
    sensor_msgs::Imu::Ptr msg(new sensor_msgs::Imu(*msg_in));

    // Livox MID360 reports acceleration in g. Convert only FAST-LIO's private
    // message copy to SI; keep the original /livox/imu topic unchanged.
    msg->linear_acceleration.x *= imu_acc_scale;
    msg->linear_acceleration.y *= imu_acc_scale;
    msg->linear_acceleration.z *= imu_acc_scale;

    // lidar 和 imu时间差过大，且开启 时间同步, 纠正当前输入imu的时间
    if (abs(timediff_lidar_wrt_imu) > 0.1 && time_sync_en)
    {
        // 对输入imu时间，纠正为 时间差 + 原始时间
        msg->header.stamp =
            ros::Time().fromSec(timediff_lidar_wrt_imu + msg_in->header.stamp.toSec());
    }

    double timestamp = msg->header.stamp.toSec();

    std::lock_guard<std::mutex> lock(mtx_buffer);

    if (timestamp < last_timestamp_imu)
    {
        // Clearing only imu_buffer is not a complete estimator epoch reset:
        // ImuProcess retains last_imu_/last_lidar_end_time_ and the EKF/map
        // retain the old time base. Drop the regressed packet instead.
        ROS_WARN("imu timestamp regressed (current=%.9f last=%.9f); drop sample",
                 timestamp, last_timestamp_imu);
        return;
    }
    if (imu_buffer.size() >= kMaxImuBufferSize)
    {
        ROS_ERROR_THROTTLE(1.0, "IMU processing buffer is full; drop incoming sample");
        return;
    }

    last_timestamp_imu = timestamp; // update imu time

    imu_buffer.push_back(msg);
    sig_buffer.notify_all();
}

double lidar_mean_scantime = 0.0;
int scan_num = 0;
bool sync_packages(MeasureGroup &meas)
{
    if (lidar_buffer.empty() || imu_buffer.empty())
    {
        return false;
    }

    /*** push a lidar scan ***/
    if (!lidar_pushed)
    {
        meas.lidar = lidar_buffer.front().cloud;          // lidar指针指向最旧的lidar数据
        meas.lidar_beg_time = lidar_buffer.front().stamp; // 与点云原子配对的帧头时间

        // 更新结束时刻的时间
        if (meas.lidar->points.size() <= 1) // time too little 时间太短，点数不足
        {
            lidar_end_time = meas.lidar_beg_time + lidar_mean_scantime; // 记录lidar结束时间为 起始时间 + 单帧扫描时间
            ROS_WARN("Too few input point cloud!\n");
        }
        else if (meas.lidar->points.back().curvature / double(1000) < 0.5 * lidar_mean_scantime) // 最后一个点的时间 小于 单帧扫描时间的一半
        {
            lidar_end_time = meas.lidar_beg_time + lidar_mean_scantime; // 记录lidar结束时间为 起始时间 + 单帧扫描时间
        }
        else
        {
            scan_num++;
            lidar_end_time = meas.lidar_beg_time + meas.lidar->points.back().curvature / double(1000); // 结束时间设置为 起始时间 + 最后一个点的时间（相对）
            // 动态更新每帧lidar数据平均扫描时间
            lidar_mean_scantime += (meas.lidar->points.back().curvature / double(1000) - lidar_mean_scantime) / scan_num;
        }

        meas.lidar_end_time = lidar_end_time;

        lidar_pushed = true;
    }

    if (last_timestamp_imu < lidar_end_time)
    {
        return false;
    }

    /*** push imu data, and pop from imu buffer ***/
    double imu_time = imu_buffer.front()->header.stamp.toSec(); // 最旧IMU时间
    meas.imu.clear();
    while ((!imu_buffer.empty()) && (imu_time < lidar_end_time)) // 记录imu数据，imu时间小于当前帧lidar结束时间
    {
        imu_time = imu_buffer.front()->header.stamp.toSec();
        if (imu_time > lidar_end_time)
            break;
        meas.imu.push_back(imu_buffer.front()); // 记录当前lidar帧内的imu数据到meas.imu
        imu_buffer.pop_front();
    }

    lidar_buffer.pop_front();
    lidar_pushed = false;
    return true;
}

int process_increments = 0;
void map_incremental()
{
    PointVector PointToAdd;            // 需要加入到ikd-tree中的点云
    PointVector PointNoNeedDownsample; // 加入ikd-tree时，不需要降采样的点云
    PointToAdd.reserve(feats_down_size);
    PointNoNeedDownsample.reserve(feats_down_size);

    // 根据点与所在包围盒中心点的距离，分类是否需要降采样
    for (int i = 0; i < feats_down_size; i++)
    {
        /* transform to world frame */
        pointBodyToWorld(&(feats_down_body->points[i]), &(feats_down_world->points[i]));
        /* decide if need add to map */
        if (!Nearest_Points[i].empty() && flg_EKF_inited)
        {
            const PointVector &points_near = Nearest_Points[i];
            bool need_add = true;
            BoxPointType Box_of_Point;
            PointType downsample_result, mid_point;
            mid_point.x = floor(feats_down_world->points[i].x / filter_size_map_min) * filter_size_map_min + 0.5 * filter_size_map_min;
            mid_point.y = floor(feats_down_world->points[i].y / filter_size_map_min) * filter_size_map_min + 0.5 * filter_size_map_min;
            mid_point.z = floor(feats_down_world->points[i].z / filter_size_map_min) * filter_size_map_min + 0.5 * filter_size_map_min;
            float dist = calc_dist(feats_down_world->points[i], mid_point); // 当前点与box中心的距离

            // 判断最近点在x、y、z三个方向上，与中心的距离，判断是否加入时需要降采样
            if (fabs(points_near[0].x - mid_point.x) > 0.5 * filter_size_map_min && fabs(points_near[0].y - mid_point.y) > 0.5 * filter_size_map_min && fabs(points_near[0].z - mid_point.z) > 0.5 * filter_size_map_min)
            {
                PointNoNeedDownsample.push_back(feats_down_world->points[i]); // 若三个方向距离都大于地图珊格半轴长，无需降采样
                continue;
            }

            // 判断当前点的 NUM_MATCH_POINTS 个邻近点与包围盒中心的范围
            for (int readd_i = 0; readd_i < NUM_MATCH_POINTS; readd_i++)
            {
                if (points_near.size() < NUM_MATCH_POINTS)
                    break;
                if (calc_dist(points_near[readd_i], mid_point) < dist) // 如果邻近点到中心的距离 小于 当前点到中心的距离，则不需要添加当前点
                {
                    need_add = false;
                    break;
                }
            }
            if (need_add)
                PointToAdd.push_back(feats_down_world->points[i]);
        }
        else
        {
            PointToAdd.push_back(feats_down_world->points[i]);
        }
    }

    double st_time = omp_get_wtime();
    add_point_size = ikdtree.Add_Points(PointToAdd, true); // 加入点时需要降采样
    ikdtree.Add_Points(PointNoNeedDownsample, false);      // 加入点时不需要降采样
    add_point_size = PointToAdd.size() + PointNoNeedDownsample.size();
    kdtree_incremental_time = omp_get_wtime() - st_time;
}

PointCloudXYZI::Ptr pcl_wait_pub(new PointCloudXYZI(500000, 1));
PointCloudXYZI::Ptr pcl_wait_save(new PointCloudXYZI());

PointCloudXYZI::Ptr pcl_init_cloud(new PointCloudXYZI());

// Publish the preprocessed scan after motion compensation. UndistortPcl()
// expresses every point in the LiDAR frame at lidar_end_time, so no additional
// extrinsic or world-frame transform is applied here.
void publish_deskewed_cloud(const ros::Publisher &pubLaserCloudDeskewed)
{
    sensor_msgs::PointCloud2 cloud_msg;
    pcl::toROSMsg(*feats_undistort, cloud_msg);
    cloud_msg.header.stamp = ros::Time().fromSec(lidar_end_time);
    cloud_msg.header.frame_id = "livox_frame";
    pubLaserCloudDeskewed.publish(cloud_msg);
}

void publish_frame_world(const ros::Publisher &pubLaserCloudFull)
{
    if (scan_pub_en)
    {
        PointCloudXYZI::Ptr laserCloudFullRes(dense_pub_en ? feats_undistort : feats_down_body);
        int size = laserCloudFullRes->points.size();
        PointCloudXYZI::Ptr laserCloudWorld(new PointCloudXYZI(size, 1));

        for (int i = 0; i < size; i++)
        {
            RGBpointBodyToWorld(&laserCloudFullRes->points[i],
                                &laserCloudWorld->points[i]);
        }

        sensor_msgs::PointCloud2 laserCloudmsg;
        pcl::toROSMsg(*laserCloudWorld, laserCloudmsg);
        laserCloudmsg.header.stamp = ros::Time().fromSec(lidar_end_time);
        laserCloudmsg.header.frame_id = "odom";
        pubLaserCloudFull.publish(laserCloudmsg);
        publish_count -= PUBFRAME_PERIOD;
    }

    /**************** save map ****************/
    /* 1. make sure you have enough memories
    /* 2. noted that pcd save will influence the real-time performences **/
    if (pcd_save_en)
    {
        int size = feats_undistort->points.size();
        PointCloudXYZI::Ptr laserCloudWorld(new PointCloudXYZI(size, 1));

        for (int i = 0; i < size; i++)
        {
            RGBpointBodyToWorld(&feats_undistort->points[i],
                                &laserCloudWorld->points[i]);
        }
        *pcl_wait_save += *laserCloudWorld;

        static int scan_wait_num = 0;
        scan_wait_num++;
        if (pcl_wait_save->size() > 0 && pcd_save_interval > 0 && scan_wait_num >= pcd_save_interval)
        {
            pcd_index++;
            string all_points_dir(string(string(ROOT_DIR) + "PCD/scans_") + to_string(pcd_index) + string(".pcd"));
            pcl::PCDWriter pcd_writer;
            cout << "current scan saved to /PCD/" << all_points_dir << endl;
            pcd_writer.writeBinary(all_points_dir, *pcl_wait_save);
            pcl_wait_save->clear();
            scan_wait_num = 0;
        }
    }
}

void publish_frame_body(const ros::Publisher &pubLaserCloudFull_body)
{
    int size = feats_undistort->points.size();
    PointCloudXYZI::Ptr laserCloudIMUBody(new PointCloudXYZI(size, 1));

    for (int i = 0; i < size; i++)
    {
        RGBpointBodyLidarToIMU(&feats_undistort->points[i],
                               &laserCloudIMUBody->points[i]);
    }

    sensor_msgs::PointCloud2 laserCloudmsg;
    pcl::toROSMsg(*laserCloudIMUBody, laserCloudmsg);
    laserCloudmsg.header.stamp = ros::Time().fromSec(lidar_end_time);
    laserCloudmsg.header.frame_id = "laser";
    pubLaserCloudFull_body.publish(laserCloudmsg);
    publish_count -= PUBFRAME_PERIOD;
}

void publish_effect_world(const ros::Publisher &pubLaserCloudEffect)
{
    PointCloudXYZI::Ptr laserCloudWorld(
        new PointCloudXYZI(effct_feat_num, 1));
    for (int i = 0; i < effct_feat_num; i++)
    {
        RGBpointBodyToWorld(&laserCloudOri->points[i],
                            &laserCloudWorld->points[i]);
    }
    sensor_msgs::PointCloud2 laserCloudFullRes3;
    pcl::toROSMsg(*laserCloudWorld, laserCloudFullRes3);
    laserCloudFullRes3.header.stamp = ros::Time().fromSec(lidar_end_time);
    laserCloudFullRes3.header.frame_id = "odom";
    pubLaserCloudEffect.publish(laserCloudFullRes3);
}

void publish_map(const ros::Publisher &pubLaserCloudMap)
{
    sensor_msgs::PointCloud2 laserCloudMap;
    pcl::toROSMsg(*featsFromMap, laserCloudMap);
    laserCloudMap.header.stamp = ros::Time().fromSec(lidar_end_time);
    laserCloudMap.header.frame_id = "odom";
    pubLaserCloudMap.publish(laserCloudMap);
}

template <typename T>
void set_posestamp(T &out)
{
    out.pose.position.x = state_point.pos(0);
    out.pose.position.y = state_point.pos(1);
    out.pose.position.z = state_point.pos(2);
    out.pose.orientation.x = geoQuat.x;
    out.pose.orientation.y = geoQuat.y;
    out.pose.orientation.z = geoQuat.z;
    out.pose.orientation.w = geoQuat.w;
}

// TODO: 增加置信度判断阈值.
double position_threshold = 0.5;
double orientation_threshold = 0.05;
bool first_ctr_flag = false;
// TODO: 增加了综合定位信息的输出.
void publish_odometry(const ros::Publisher &pubOdomAftMapped,
                      const ros::Publisher &pubRawPosition,
                      const ros::Publisher &pubLevelPosition,
                      const ros::Publisher &pubLevelTransform)
{
    odomAftMapped.header.frame_id = "odom";
    odomAftMapped.child_frame_id = "laser";
    odomAftMapped.header.stamp = ros::Time().fromSec(lidar_end_time); // ros::Time().fromSec(lidar_end_time);
    set_posestamp(odomAftMapped.pose);
    pubOdomAftMapped.publish(odomAftMapped);
    auto P = kf.get_P();
    for (int i = 0; i < 6; i++)
    {
        int k = i < 3 ? i + 3 : i - 3;
        odomAftMapped.pose.covariance[i * 6 + 0] = P(k, 3);
        odomAftMapped.pose.covariance[i * 6 + 1] = P(k, 4);
        odomAftMapped.pose.covariance[i * 6 + 2] = P(k, 5);
        odomAftMapped.pose.covariance[i * 6 + 3] = P(k, 0);
        odomAftMapped.pose.covariance[i * 6 + 4] = P(k, 1);
        odomAftMapped.pose.covariance[i * 6 + 5] = P(k, 2);
    }

    // TODO: 计算定位和姿态的置信度并发布.
    // ===============================================================================================
    // 提取位置的协方差矩阵
    Eigen::Matrix<double, 3, 3> position_covariance;
    position_covariance << odomAftMapped.pose.covariance[0], odomAftMapped.pose.covariance[1], odomAftMapped.pose.covariance[2],
        odomAftMapped.pose.covariance[6], odomAftMapped.pose.covariance[7], odomAftMapped.pose.covariance[8],
        odomAftMapped.pose.covariance[12], odomAftMapped.pose.covariance[13], odomAftMapped.pose.covariance[14];
    Eigen::Vector3d position_std = position_covariance.diagonal().array().sqrt();

    // 提取姿态的协方差矩阵
    Eigen::Matrix<double, 3, 3> orientation_covariance;
    orientation_covariance << odomAftMapped.pose.covariance[21], odomAftMapped.pose.covariance[22], odomAftMapped.pose.covariance[23],
        odomAftMapped.pose.covariance[27], odomAftMapped.pose.covariance[28], odomAftMapped.pose.covariance[29],
        odomAftMapped.pose.covariance[33], odomAftMapped.pose.covariance[34], odomAftMapped.pose.covariance[35];
    Eigen::Vector3d orientation_std = orientation_covariance.diagonal().array().sqrt();

    // 计算各个方向上的均值.
    double position_ = (position_std.x() + position_std.y() + position_std.z()) / 3;
    double orientation_ = (orientation_std.x() + orientation_std.y() + orientation_std.z()) / 3;

    bool position_state;
    if (position_ <= position_threshold && orientation_ <= orientation_threshold)
    {
        position_state = true;
    }
    else
    {
        position_state = false;
        ROS_WARN("Positioning is unreliable");
    }
    // ===============================================================================================

    static tf::TransformBroadcaster br;
    tf::Transform transform;
    tf::Quaternion q;
    transform.setOrigin(tf::Vector3(odomAftMapped.pose.pose.position.x,
                                    odomAftMapped.pose.pose.position.y,
                                    odomAftMapped.pose.pose.position.z));
    q.setW(odomAftMapped.pose.pose.orientation.w);
    q.setX(odomAftMapped.pose.pose.orientation.x);
    q.setY(odomAftMapped.pose.pose.orientation.y);
    q.setZ(odomAftMapped.pose.pose.orientation.z);
    transform.setRotation(q);
    br.sendTransform(tf::StampedTransform(transform, odomAftMapped.header.stamp, "odom", "laser"));

    // ==============================================
    double roll, pitch, yaw;
    tf::Matrix3x3(q).getRPY(roll, pitch, yaw);
    // 车身纵向偏移0.637
    // TODO:
    // double car_direction = 0.637 * cos(yaw);  // + 0.01 * sin(yaw);
    // double car_transverse = 0.637 * sin(yaw); // - 0.01 * cos(yaw);
    // if (!first_ctr_flag)
    // {
    //     car_direction = 0.;
    //     car_transverse = 0.;
    //     first_ctr_flag = true;
    // }
    // Preserve the historical raw topic for diagnostics and existing health
    // monitoring.  It remains the IMU state pose expressed in FAST-LIO world W.
    util::LIOPose raw_lio_pose_msg;
    raw_lio_pose_msg.header.frame_id = "odom";
    raw_lio_pose_msg.header.stamp = odomAftMapped.header.stamp;
    raw_lio_pose_msg.position_x = odomAftMapped.pose.pose.position.x;
    raw_lio_pose_msg.position_y = odomAftMapped.pose.pose.position.y;
    raw_lio_pose_msg.position_z = odomAftMapped.pose.pose.position.z;
    raw_lio_pose_msg.q_w = q.w();
    raw_lio_pose_msg.q_x = q.x();
    raw_lio_pose_msg.q_y = q.y();
    raw_lio_pose_msg.q_z = q.z();
    raw_lio_pose_msg.lio_state = position_state;
    pubRawPosition.publish(raw_lio_pose_msg);

    // Dedicated production input for fusion_se2.  This is a pure left world
    // rotation; no base_link extrinsic or anchor is applied here.  fusion_se2
    // subsequently performs T_H_B = T_H_I * T_I_B and estimates the remaining
    // H->ENU yaw plus x/y/z translation.  It deliberately does not re-anchor
    // on its own first callback, so a fusion-only restart stays continuous.
    if (!EnsureLioLevelInitialized(pubLevelTransform))
    {
        util::LIOPose invalid_level_pose;
        invalid_level_pose.header.frame_id = "lio_level";
        invalid_level_pose.header.stamp = odomAftMapped.header.stamp;
        invalid_level_pose.q_w = 1.0;
        invalid_level_pose.lio_state = false;
        pubLevelPosition.publish(invalid_level_pose);
        return;
    }

    const V3D position_H_I = lio_level_R_H_W * state_point.pos;
    const M3D rotation_H_I =
        lio_level_R_H_W * state_point.rot.toRotationMatrix();
    Eigen::Quaterniond q_H_I(rotation_H_I);
    q_H_I.normalize();

    util::LIOPose level_lio_pose_msg;
    level_lio_pose_msg.header.frame_id = "lio_level";
    level_lio_pose_msg.header.stamp = odomAftMapped.header.stamp;
    level_lio_pose_msg.position_x = position_H_I.x();
    level_lio_pose_msg.position_y = position_H_I.y();
    level_lio_pose_msg.position_z = position_H_I.z();
    level_lio_pose_msg.q_w = q_H_I.w();
    level_lio_pose_msg.q_x = q_H_I.x();
    level_lio_pose_msg.q_y = q_H_I.y();
    level_lio_pose_msg.q_z = q_H_I.z();
    level_lio_pose_msg.lio_state = position_state;
    pubLevelPosition.publish(level_lio_pose_msg);
    if (lio_diagnostic_log_en)
    {
        static bool diagnostic_origin_set = false;
        static V3D diagnostic_origin_W(Zero3d);
        static V3D diagnostic_origin_H(Zero3d);
        if (!diagnostic_origin_set)
        {
            diagnostic_origin_W = state_point.pos;
            diagnostic_origin_H = position_H_I;
            diagnostic_origin_set = true;
        }
        const V3D displacement_W = state_point.pos - diagnostic_origin_W;
        const V3D displacement_H = position_H_I - diagnostic_origin_H;
        tf::Quaternion q_h(q_H_I.x(), q_H_I.y(), q_H_I.z(), q_H_I.w());
        double roll_h = 0.0, pitch_h = 0.0, yaw_h = 0.0;
        tf::Matrix3x3(q_h).getRPY(roll_h, pitch_h, yaw_h);
        ROS_INFO_THROTTLE(
            lio_diagnostic_log_period_sec,
            "[LIO4D-FAST] stamp=%.9f gen=%u.%09u state=%d "
            "w_xyz=(%.6f,%.6f,%.6f) h_xyz=(%.6f,%.6f,%.6f) "
            "w_dxyz=(%.6f,%.6f,%.6f) h_dxyz=(%.6f,%.6f,%.6f) "
            "w_dist_xy_3d=(%.6f,%.6f) h_dist_xy_3d=(%.6f,%.6f) "
            "w_q_wxyz=(%.9f,%.9f,%.9f,%.9f) "
            "h_q_wxyz=(%.9f,%.9f,%.9f,%.9f) "
            "w_rpy=(%.7f,%.7f,%.7f) h_rpy=(%.7f,%.7f,%.7f) "
            "grav_w=(%.7f,%.7f,%.7f) bg=(%.8f,%.8f,%.8f) "
            "ba=(%.8f,%.8f,%.8f) vel_w=(%.6f,%.6f,%.6f) "
            "features=%d residual_mean=%.7f",
            odomAftMapped.header.stamp.toSec(),
            lio_process_generation_stamp.sec,
            lio_process_generation_stamp.nsec, position_state ? 1 : 0,
            state_point.pos.x(), state_point.pos.y(), state_point.pos.z(),
            position_H_I.x(), position_H_I.y(), position_H_I.z(),
            displacement_W.x(), displacement_W.y(), displacement_W.z(),
            displacement_H.x(), displacement_H.y(), displacement_H.z(),
            std::hypot(displacement_W.x(), displacement_W.y()),
            displacement_W.norm(),
            std::hypot(displacement_H.x(), displacement_H.y()),
            displacement_H.norm(), q.w(), q.x(), q.y(), q.z(), q_H_I.w(),
            q_H_I.x(), q_H_I.y(), q_H_I.z(), roll, pitch, yaw, roll_h,
            pitch_h, yaw_h, state_point.grav[0], state_point.grav[1],
            state_point.grav[2], state_point.bg[0], state_point.bg[1],
            state_point.bg[2], state_point.ba[0], state_point.ba[1],
            state_point.ba[2], state_point.vel.x(), state_point.vel.y(),
            state_point.vel.z(), effct_feat_num, res_mean_last);
    }
    // ==============================================
}

void publish_path(const ros::Publisher pubPath)
{
    set_posestamp(msg_body_pose);
    msg_body_pose.header.stamp = ros::Time().fromSec(lidar_end_time);
    msg_body_pose.header.frame_id = "odom";

    /*** if path is too large, the rvis will crash ***/
    static int jjj = 0;
    jjj++;
    if (jjj % 10 == 0)
    {
        path.poses.push_back(msg_body_pose);
        pubPath.publish(path);
    }
}

// 构造H矩阵
void h_share_model(state_ikfom &s, esekfom::dyn_share_datastruct<double> &ekfom_data)
{
    double match_start = omp_get_wtime();
    laserCloudOri->clear();
    corr_normvect->clear();
    total_residual = 0.0;

/** closest surface search and residual computation **/
#ifdef MP_EN
    omp_set_num_threads(MP_PROC_NUM);
#pragma omp parallel for
#endif
    for (int i = 0; i < feats_down_size; i++) // 判断每个点的对应邻域是否符合平面点的假设
    {
        PointType &point_body = feats_down_body->points[i];   // lidar系下坐标
        PointType &point_world = feats_down_world->points[i]; // lidar数据点在world系下坐标

        /* transform to world frame */
        V3D p_body(point_body.x, point_body.y, point_body.z);                     // lidar系下坐标
        V3D p_global(s.rot * (s.offset_R_L_I * p_body + s.offset_T_L_I) + s.pos); // w系下坐标
        point_world.x = p_global(0);
        point_world.y = p_global(1);
        point_world.z = p_global(2);
        point_world.intensity = point_body.intensity;

        vector<float> pointSearchSqDis(NUM_MATCH_POINTS);

        auto &points_near = Nearest_Points[i];

        if (ekfom_data.converge)
        {
            /** Find the closest surfaces in the map **/
            // world系下从ikdtree找5个最近点用于平面拟合
            ikdtree.Nearest_Search(point_world, NUM_MATCH_POINTS, points_near, pointSearchSqDis);
            // 最近点数大于NUM_MATCH_POINTS，且最大距离小于等于5,point_selected_surf设置为true
            point_selected_surf[i] = points_near.size() < NUM_MATCH_POINTS ? false : pointSearchSqDis[NUM_MATCH_POINTS - 1] > 5 ? false
                                                                                                                                : true;
        }

        // 不符合平面特征
        if (!point_selected_surf[i])
            continue;

        VF(4)
        pabcd;
        point_selected_surf[i] = false; // 二次筛选平面点
        // 拟合局部平面，返回：是否有内点大于距离阈值
        if (esti_plane(pabcd, points_near, 0.1f))
        {
            // plane distance
            float pd2 = pabcd(0) * point_world.x + pabcd(1) * point_world.y + pabcd(2) * point_world.z + pabcd(3);
            float s = 1 - 0.9 * fabs(pd2) / sqrt(p_body.norm()); // 筛选条件 1 - 0.9 * （点到平面距离 / 点到lidar原点距离）

            if (s > 0.9)
            {
                point_selected_surf[i] = true;
                normvec->points[i].x = pabcd(0);
                normvec->points[i].y = pabcd(1);
                normvec->points[i].z = pabcd(2);
                normvec->points[i].intensity = pd2; // 以intensity记录点到面残差
                res_last[i] = abs(pd2);             // 残差，距离
            }
        }
    }

    effct_feat_num = 0; // 有效匹配点数

    for (int i = 0; i < feats_down_size; i++)
    {
        if (point_selected_surf[i])
        {
            laserCloudOri->points[effct_feat_num] = feats_down_body->points[i]; // body系 平面特征点
            corr_normvect->points[effct_feat_num] = normvec->points[i];         // world系 平面参数
            total_residual += res_last[i];                                      // 残差和
            effct_feat_num++;
        }
    }

    if (effct_feat_num < 1)
    {
        ekfom_data.valid = false;
        ROS_WARN("No Effective Points! \n");
        return;
    }

    res_mean_last = total_residual / effct_feat_num; // 残差均值 （距离）
    match_time += omp_get_wtime() - match_start;
    double solve_start_ = omp_get_wtime();

    /*** Computation of Measuremnt Jacobian matrix H and measurents vector ***/
    ekfom_data.h_x = MatrixXd::Zero(effct_feat_num, 12); // 定义H维度
    ekfom_data.h.resize(effct_feat_num);                 // 有效方程个数

    for (int i = 0; i < effct_feat_num; i++)
    {
        const PointType &laser_p = laserCloudOri->points[i]; // lidar系 平面特征点
        V3D point_this_be(laser_p.x, laser_p.y, laser_p.z);
        M3D point_be_crossmat;
        point_be_crossmat << SKEW_SYM_MATRX(point_this_be);
        V3D point_this = s.offset_R_L_I * point_this_be + s.offset_T_L_I; // 当前状态imu系下 点坐标
        M3D point_crossmat;
        point_crossmat << SKEW_SYM_MATRX(point_this); // 当前状态imu系下 点坐标反对称矩阵

        /*** get the normal vector of closest surface/corner ***/
        const PointType &norm_p = corr_normvect->points[i];
        V3D norm_vec(norm_p.x, norm_p.y, norm_p.z); // 对应局部法相量, world系下

        /*** calculate the Measuremnt Jacobian matrix H ***/
        V3D C(s.rot.conjugate() * norm_vec); // 将对应局部法相量旋转到imu系下 corr_normal_I
        V3D A(point_crossmat * C);           // 残差对角度求导系数 P(IMU)^ [R(imu <-- w) * normal_w]
        // 添加数据到矩阵
        if (extrinsic_est_en)
        {
            // B = lidar_p^ R(L <-- I) * corr_normal_I
            // B = lidar_p^ R(L <-- I) * R(I <-- W) * normal_W
            V3D B(point_be_crossmat * s.offset_R_L_I.conjugate() * C); // s.rot.conjugate()*norm_vec);
            ekfom_data.h_x.block<1, 12>(i, 0) << norm_p.x, norm_p.y, norm_p.z, VEC_FROM_ARRAY(A), VEC_FROM_ARRAY(B), VEC_FROM_ARRAY(C);
        }
        else
        {
            ekfom_data.h_x.block<1, 12>(i, 0) << norm_p.x, norm_p.y, norm_p.z, VEC_FROM_ARRAY(A), 0.0, 0.0, 0.0, 0.0, 0.0, 0.0;
        }

        /*** Measuremnt: distance to the closest surface/corner ***/
        ekfom_data.h(i) = -norm_p.intensity;
    }
    solve_time += omp_get_wtime() - solve_start_;
}

// =============================================
// TODO: 异常定位发布，确保进入回调
void ErrorPublish(const ros::Publisher &pubPosition,
                  const std::string &frame_id)
{
    util::LIOPose lio_pose_msg;
    lio_pose_msg.header.frame_id = frame_id;
    lio_pose_msg.header.stamp = odomAftMapped.header.stamp;
    lio_pose_msg.position_x = 0.;
    lio_pose_msg.position_y = 0.;
    lio_pose_msg.position_z = 0.;
    lio_pose_msg.q_w = 1.0;
    lio_pose_msg.q_x = 0.;
    lio_pose_msg.q_y = 0.;
    lio_pose_msg.q_z = 0.;
    lio_pose_msg.lio_state = false;
    pubPosition.publish(lio_pose_msg);
}
// ==============================================

int main(int argc, char **argv)
{
    ros::init(argc, argv, "laserMapping");
    ros::NodeHandle nh;
    const ros::WallTime generation_wall_time = ros::WallTime::now();
    lio_process_generation_stamp =
        ros::Time(generation_wall_time.sec, generation_wall_time.nsec);

    nh.param<bool>("publish/path_en", path_en, true);
    nh.param<bool>("publish/scan_publish_en", scan_pub_en, true);
    nh.param<bool>("publish/dense_publish_en", dense_pub_en, true);
    nh.param<bool>("publish/scan_bodyframe_pub_en", scan_body_pub_en, true);
    nh.param<int>("max_iteration", NUM_MAX_ITERATIONS, 4);
    nh.param<string>("map_file_path", map_file_path, "");
    nh.param<string>("common/lid_topic", lid_topic, "/livox/lidar");
    nh.param<string>("common/imu_topic", imu_topic, "/livox/imu");
    resolved_imu_topic = nh.resolveName(imu_topic);
    nh.param<bool>("common/time_sync_en", time_sync_en, false);
    nh.param<double>("filter_size_corner", filter_size_corner_min, 0.5);
    nh.param<double>("filter_size_surf", filter_size_surf_min, 0.5);
    nh.param<double>("filter_size_map", filter_size_map_min, 0.5);
    nh.param<double>("cube_side_length", cube_len, 200);
    nh.param<float>("mapping/det_range", DET_RANGE, 300.f);
    nh.param<double>("mapping/fov_degree", fov_deg, 180);
    nh.param<double>("mapping/gyr_cov", gyr_cov, 0.1);
    nh.param<double>("mapping/acc_cov", acc_cov, 0.1);
    nh.param<double>("mapping/b_gyr_cov", b_gyr_cov, 0.0001);
    nh.param<double>("mapping/b_acc_cov", b_acc_cov, 0.0001);
    nh.param<double>("common/imu_acc_scale", imu_acc_scale, 1.0);
    if (!std::isfinite(imu_acc_scale) || imu_acc_scale <= 0.0)
    {
        ROS_FATAL("common/imu_acc_scale must be finite and > 0, got %.6f.",
                  imu_acc_scale);
        return EXIT_FAILURE;
    }
    fast_lio::ImuStaticDetector::Config imu_static_config;
    int imu_init_min_samples =
        static_cast<int>(imu_static_config.min_samples);
    nh.param<double>("mapping/imu_init_static_duration",
                     imu_static_config.confirmation_time_sec, 2.0);
    nh.param<double>("mapping/imu_init_max_gyro_mean_norm",
                     imu_static_config.max_gyro_mean_norm, 0.08);
    nh.param<double>("mapping/imu_init_max_gyro_norm",
                     imu_static_config.max_gyro_norm, 0.20);
    nh.param<double>("mapping/imu_init_min_acc_norm",
                     imu_static_config.min_acc_norm, 8.0);
    nh.param<double>("mapping/imu_init_max_acc_norm",
                     imu_static_config.max_acc_norm, 11.5);
    nh.param<double>("mapping/imu_init_max_acc_variance",
                     imu_static_config.max_acc_variance, 0.0);
    nh.param<double>("mapping/imu_init_max_sample_gap",
                     imu_static_config.max_sample_gap_sec, 0.10);
    nh.param<int>("mapping/imu_init_min_samples", imu_init_min_samples, 50);
    if (imu_init_min_samples < 2)
    {
        ROS_FATAL("mapping/imu_init_min_samples must be >= 2, got %d.",
                  imu_init_min_samples);
        return EXIT_FAILURE;
    }
    imu_static_config.min_samples =
        static_cast<std::size_t>(imu_init_min_samples);
    nh.param<double>("preprocess/blind", p_pre->blind, 0.01);
    nh.param<int>("preprocess/lidar_type", p_pre->lidar_type, AVIA);
    nh.param<int>("preprocess/scan_line", p_pre->N_SCANS, 16);
    nh.param<int>("preprocess/scan_rate", p_pre->SCAN_RATE, 10);
    nh.param<int>("point_filter_num", p_pre->point_filter_num, 2);
    nh.param<bool>("feature_extract_enable", p_pre->feature_enabled, false);
    nh.param<bool>("runtime_pos_log_enable", runtime_pos_log, 0);
    nh.param<bool>("publish/lio_gravity_level_en", lio_gravity_level_en, true);
    nh.param<bool>("publish/lio_diagnostic_log_en", lio_diagnostic_log_en,
                   true);
    nh.param<double>("publish/lio_diagnostic_log_period_sec",
                     lio_diagnostic_log_period_sec, 0.20);
    if (!std::isfinite(lio_diagnostic_log_period_sec) ||
        lio_diagnostic_log_period_sec <= 0.0)
    {
        ROS_WARN("publish/lio_diagnostic_log_period_sec must be > 0; "
                 "force to 0.20s.");
        lio_diagnostic_log_period_sec = 0.20;
    }
    nh.param<bool>("mapping/extrinsic_est_en", extrinsic_est_en, true);
    nh.param<bool>("pcd_save/pcd_save_en", pcd_save_en, false);
    nh.param<int>("pcd_save/interval", pcd_save_interval, -1);
    nh.param<vector<double>>("mapping/extrinsic_T", extrinT, vector<double>());
    nh.param<vector<double>>("mapping/extrinsic_R", extrinR, vector<double>());
    cout << "p_pre->lidar_type " << p_pre->lidar_type << endl;

    path.header.stamp = ros::Time::now();
    path.header.frame_id = "odom";

    /*** variables definition ***/
    int effect_feat_num = 0, frame_num = 0;
    double deltaT, deltaR, aver_time_consu = 0, aver_time_icp = 0, aver_time_match = 0, aver_time_incre = 0, aver_time_solve = 0, aver_time_const_H_time = 0;
    bool flg_EKF_converged, EKF_stop_flg = 0;

    FOV_DEG = (fov_deg + 10.0) > 179.9 ? 179.9 : (fov_deg + 10.0);
    HALF_FOV_COS = cos((FOV_DEG) * 0.5 * PI_M / 180.0);

    _featsArray.reset(new PointCloudXYZI());

    memset(point_selected_surf, true, sizeof(point_selected_surf));
    std::fill(res_last, res_last + 100000, -1000.0f);
    downSizeFilterSurf.setLeafSize(filter_size_surf_min, filter_size_surf_min, filter_size_surf_min);
    downSizeFilterMap.setLeafSize(filter_size_map_min, filter_size_map_min, filter_size_map_min);
    // memset(point_selected_surf, true, sizeof(point_selected_surf)); // 重复？
    std::fill(res_last, res_last + 100000, -1000.0f);

    // 设置imu和lidar外参和imu参数等
    Lidar_T_wrt_IMU << VEC_FROM_ARRAY(extrinT);
    Lidar_R_wrt_IMU << MAT_FROM_ARRAY(extrinR);
    p_imu->set_extrinsic(Lidar_T_wrt_IMU, Lidar_R_wrt_IMU);
    p_imu->set_gyr_cov(V3D(gyr_cov, gyr_cov, gyr_cov));
    p_imu->set_acc_cov(V3D(acc_cov, acc_cov, acc_cov)); // 加速度协方差
    p_imu->set_gyr_bias_cov(V3D(b_gyr_cov, b_gyr_cov, b_gyr_cov));
    p_imu->set_acc_bias_cov(V3D(b_acc_cov, b_acc_cov, b_acc_cov));
    if (!p_imu->set_static_init_config(imu_static_config))
    {
        ROS_FATAL("Invalid FAST-LIO static IMU initialization parameters. "
                  "Check duration/gyro/acceleration/variance/gap limits.");
        return EXIT_FAILURE;
    }
    ROS_INFO("Static IMU initialization: continuous %.2fs, samples>=%zu, "
             "gyro mean<=%.4f rad/s, gyro max<=%.4f rad/s, "
             "acc norm=[%.3f, %.3f] m/s^2, acc variance gate=%s "
             "(limit=%.5f per axis), sample gap<=%.3fs.",
             imu_static_config.confirmation_time_sec,
             imu_static_config.min_samples,
             imu_static_config.max_gyro_mean_norm,
             imu_static_config.max_gyro_norm,
             imu_static_config.min_acc_norm,
             imu_static_config.max_acc_norm,
             imu_static_config.max_acc_variance > 0.0 ? "enabled" : "disabled",
             imu_static_config.max_acc_variance,
             imu_static_config.max_sample_gap_sec);

    double epsi[23] = {0.001};
    fill(epsi, epsi + 23, 0.001);
    /// 初始化，其中h_share_model定义了·平面搜索和残差计算
    kf.init_dyn_share(get_f, df_dx, df_dw, h_share_model, NUM_MAX_ITERATIONS, epsi);

    /*** debug record ***/
    FILE *fp;
    string pos_log_dir = root_dir + "/Log/pos_log.txt";
    fp = fopen(pos_log_dir.c_str(), "w");

    ofstream fout_pre, fout_out, fout_dbg;
    fout_pre.open(DEBUG_FILE_DIR("mat_pre.txt"), ios::out);
    fout_out.open(DEBUG_FILE_DIR("mat_out.txt"), ios::out);
    fout_dbg.open(DEBUG_FILE_DIR("dbg.txt"), ios::out);
    if (fout_pre && fout_out)
        cout << "~~~~" << ROOT_DIR << " file opened" << endl;
    else
        cout << "~~~~" << ROOT_DIR << " doesn't exist" << endl;

    /*** ROS subscribe initialization ***/
    // Bound transport queues so overload drops old work instead of replaying
    // seconds of stale sensor data into a real-time localization pipeline.
    ros::Subscriber sub_pcl = p_pre->lidar_type == AVIA ? nh.subscribe(lid_topic, 32, livox_pcl_cbk) : nh.subscribe(lid_topic, 32, standard_pcl_cbk);
    ros::Subscriber sub_imu = nh.subscribe(imu_topic, 2048, imu_cbk);
    pubLaserCloudFiltered = nh.advertise<sensor_msgs::PointCloud2>("/lidar_points_filtered", 1);
    ros::Publisher pubLaserCloudDeskewed = nh.advertise<sensor_msgs::PointCloud2>("/lidar_points_deskew", 10);
    ros::Publisher pubLaserCloudFull = nh.advertise<sensor_msgs::PointCloud2>("/cloud_registered", 10);
    ros::Publisher pubLaserCloudFull_body = nh.advertise<sensor_msgs::PointCloud2>("/cloud_registered_body", 10);
    ros::Publisher pubLaserCloudEffect = nh.advertise<sensor_msgs::PointCloud2>("/cloud_effected", 10);
    ros::Publisher pubLaserCloudMap = nh.advertise<sensor_msgs::PointCloud2>("/Laser_map", 2);
    ros::Publisher pubOdomAftMapped = nh.advertise<nav_msgs::Odometry>("/Odometry", 20);
    ros::Publisher pubPath = nh.advertise<nav_msgs::Path>("/path", 2);
    // TODO: 自定义消息话题输出
    ros::Publisher pubRawPosition =
        nh.advertise<util::LIOPose>("/Mower/lio_slam", 20);
    ros::Publisher pubLevelPosition =
        nh.advertise<util::LIOPose>("/Mower/lio_slam_level", 20);
    ros::Publisher pubLevelTransform =
        nh.advertise<geometry_msgs::TransformStamped>(
            "/Mower/lio_level_transform", 1, true);
    pub_lio_static_initialized =
        nh.advertise<std_msgs::Bool>("/Mower/lio_static_initialized", 1, true);
    pub_lio_static_calibration = nh.advertise<util::ImuStaticCalibration>(
        "/Mower/lio_static_calibration", 1, true);
    std_msgs::Bool initial_static_state;
    initial_static_state.data = false;
    pub_lio_static_initialized.publish(initial_static_state);
    util::ImuStaticCalibration initial_calibration;
    initial_calibration.valid = false;
    initial_calibration.lio_generation = lio_process_generation_stamp;
    initial_calibration.imu_topic = resolved_imu_topic;
    pub_lio_static_calibration.publish(initial_calibration);
    ROS_INFO("LIO gravity-level output %s: raw=/Mower/lio_slam, "
             "level=/Mower/lio_slam_level, transform=/Mower/lio_level_transform, "
             "static_status=/Mower/lio_static_initialized, "
             "static_calibration=/Mower/lio_static_calibration",
             lio_gravity_level_en ? "enabled" : "disabled(identity)");
    //------------------------------------------------------------------------------------------------------
    signal(SIGINT, SigHandle);
    ros::Rate rate(5000);
    bool status = ros::ok();
    while (status)
    {
        if (flg_exit)
            break;
        ros::spinOnce();

        /// 在Measure内，储存当前lidar数据及lidar扫描时间内对应的imu数据序列
        if (sync_packages(Measures))
        {
            // 第一帧lidar数据
            if (flg_first_scan)
            {
                first_lidar_time = Measures.lidar_beg_time; // 记录第一帧绝对时间
                p_imu->first_lidar_time = first_lidar_time; // 记录第一帧绝对时间
                flg_first_scan = false;
                continue;
            }

            double t0, t1, t2, t3, t4, t5, match_start, solve_start, svd_time;

            match_time = 0;
            kdtree_search_time = 0.0;
            solve_time = 0;
            solve_const_H_time = 0;
            svd_time = 0;
            t0 = omp_get_wtime();

            // 根据imu数据序列和lidar数据，向前传播纠正点云的畸变, 此前已经完成间隔采样或特征提取
            //  feats_undistort 为畸变纠正之后的点云,lidar系
            p_imu->Process(Measures, kf, feats_undistort);
            state_point = kf.get_x();                                               // 前向传播后body的状态预测值
            pos_lid = state_point.pos + state_point.rot * state_point.offset_T_L_I; // global系 lidar位置

            if (feats_undistort->empty() || (feats_undistort == NULL))
            {
                // TODO: 增加异常处理机制，保证数据异常的时候LIO定位能够进入到回调函数中.
                ErrorPublish(pubRawPosition, "odom");
                ErrorPublish(pubLevelPosition, "lio_level");

                // ROS_WARN("No point, skip this scan!\n");
                continue;
            }

            // feats_undistort is still the dense preprocessed scan here: it has
            // been deskewed to the scan-end LiDAR frame but not voxel-filtered,
            // transformed to the world frame, or modified by map matching.
            publish_deskewed_cloud(pubLaserCloudDeskewed);

            // 检查当前lidar数据时间，与最早lidar数据时间是否足够
            flg_EKF_inited = (Measures.lidar_beg_time - first_lidar_time) < INIT_TIME ? false : true;
            /*** Segment the map in lidar FOV ***/
            lasermap_fov_segment(); // 根据lidar在W系下的位置，重新确定局部地图的包围盒角点，移除远端的点

            /*** downsample the feature points in a scan ***/
            downSizeFilterSurf.setInputCloud(feats_undistort);
            downSizeFilterSurf.filter(*feats_down_body);
            t1 = omp_get_wtime();
            feats_down_size = feats_down_body->points.size(); // 当前帧降采样后点数

            /*** initialize the map kdtree ***/
            if (ikdtree.Root_Node == nullptr)
            {
                if (feats_down_size > 5)
                {
                    ikdtree.set_downsample_param(filter_size_map_min);
                    feats_down_world->resize(feats_down_size);
                    for (int i = 0; i < feats_down_size; i++)
                    {
                        pointBodyToWorld(&(feats_down_body->points[i]), &(feats_down_world->points[i])); // point转到world系下
                    }
                    // world系下对当前帧降采样后的点云，初始化lkd-tree
                    ikdtree.Build(feats_down_world->points);
                }
                continue;
            }
            int featsFromMapNum = ikdtree.validnum();
            kdtree_size_st = ikdtree.size();

            // cout<<"[ mapping ]: In num: "<<feats_undistort->points.size()<<" downsamp "<<feats_down_size<<" Map num: "<<featsFromMapNum<<"effect num:"<<effct_feat_num<<endl;

            /*** ICP and iterated Kalman filter update ***/
            if (feats_down_size < 5)
            {
                // TODO: 增加异常处理机制，保证数据异常的时候LIO定位能够进入到回调函数中.
                ErrorPublish(pubRawPosition, "odom");
                ErrorPublish(pubLevelPosition, "lio_level");

                // ROS_WARN("No point, skip this scan!\n");
                continue;
            }

            normvec->resize(feats_down_size);
            feats_down_world->resize(feats_down_size);

            // lidar --> imu
            V3D ext_euler = SO3ToEuler(state_point.offset_R_L_I);
            fout_pre << setw(20) << Measures.lidar_beg_time - first_lidar_time << " " << euler_cur.transpose() << " " << state_point.pos.transpose() << " " << ext_euler.transpose() << " " << state_point.offset_T_L_I.transpose() << " " << state_point.vel.transpose()
                     << " " << state_point.bg.transpose() << " " << state_point.ba.transpose() << " " << state_point.grav << endl;

            if (0) // If you need to see map point, change to "if(1)"
            {
                PointVector().swap(ikdtree.PCL_Storage);
                ikdtree.flatten(ikdtree.Root_Node, ikdtree.PCL_Storage, NOT_RECORD);
                featsFromMap->clear();
                featsFromMap->points = ikdtree.PCL_Storage;
            }

            pointSearchInd_surf.resize(feats_down_size);
            Nearest_Points.resize(feats_down_size);
            int rematch_num = 0;
            bool nearest_search_en = true; //

            t2 = omp_get_wtime();

            /*** iterated state estimation ***/
            double t_update_start = omp_get_wtime();
            double solve_H_time = 0;
            kf.update_iterated_dyn_share_modified(LASER_POINT_COV, solve_H_time); // 预测、更新
            state_point = kf.get_x();
            euler_cur = SO3ToEuler(state_point.rot);
            pos_lid = state_point.pos + state_point.rot * state_point.offset_T_L_I; // world系下lidar坐标
            geoQuat.x = state_point.rot.coeffs()[0];                                // world系下当前imu的姿态四元数
            geoQuat.y = state_point.rot.coeffs()[1];
            geoQuat.z = state_point.rot.coeffs()[2];
            geoQuat.w = state_point.rot.coeffs()[3];

            double t_update_end = omp_get_wtime();

            /******* Publish odometry *******/
            publish_odometry(pubOdomAftMapped, pubRawPosition,
                             pubLevelPosition, pubLevelTransform);

            /*** add the feature points to map kdtree ***/
            t3 = omp_get_wtime();
            map_incremental();
            t5 = omp_get_wtime();

            /******* Publish points *******/
            if (path_en)
                publish_path(pubPath);
            if (scan_pub_en || pcd_save_en)
                publish_frame_world(pubLaserCloudFull);
            if (scan_pub_en && scan_body_pub_en)
                publish_frame_body(pubLaserCloudFull_body);
            // publish_effect_world(pubLaserCloudEffect);
            // publish_map(pubLaserCloudMap);

            /*** Debug variables ***/
            if (runtime_pos_log)
            {
                frame_num++;
                kdtree_size_end = ikdtree.size();
                aver_time_consu = aver_time_consu * (frame_num - 1) / frame_num + (t5 - t0) / frame_num;
                aver_time_icp = aver_time_icp * (frame_num - 1) / frame_num + (t_update_end - t_update_start) / frame_num;
                aver_time_match = aver_time_match * (frame_num - 1) / frame_num + (match_time) / frame_num;
                aver_time_incre = aver_time_incre * (frame_num - 1) / frame_num + (kdtree_incremental_time) / frame_num;
                aver_time_solve = aver_time_solve * (frame_num - 1) / frame_num + (solve_time + solve_H_time) / frame_num;
                aver_time_const_H_time = aver_time_const_H_time * (frame_num - 1) / frame_num + solve_time / frame_num;
                T1[time_log_counter] = Measures.lidar_beg_time;
                s_plot[time_log_counter] = t5 - t0;
                s_plot2[time_log_counter] = feats_undistort->points.size();
                s_plot3[time_log_counter] = kdtree_incremental_time;
                s_plot4[time_log_counter] = kdtree_search_time;
                s_plot5[time_log_counter] = kdtree_delete_counter;
                s_plot6[time_log_counter] = kdtree_delete_time;
                s_plot7[time_log_counter] = kdtree_size_st;
                s_plot8[time_log_counter] = kdtree_size_end;
                s_plot9[time_log_counter] = aver_time_consu;
                s_plot10[time_log_counter] = add_point_size;
                time_log_counter++;
                printf("[ mapping ]: time: IMU + Map + Input Downsample: %0.6f ave match: %0.6f ave solve: %0.6f  ave ICP: %0.6f  map incre: %0.6f ave total: %0.6f icp: %0.6f construct H: %0.6f \n", t1 - t0, aver_time_match, aver_time_solve, t3 - t1, t5 - t3, aver_time_consu, aver_time_icp, aver_time_const_H_time);
                ext_euler = SO3ToEuler(state_point.offset_R_L_I);
                fout_out << setw(20) << Measures.lidar_beg_time - first_lidar_time << " " << euler_cur.transpose() << " " << state_point.pos.transpose() << " " << ext_euler.transpose() << " " << state_point.offset_T_L_I.transpose() << " " << state_point.vel.transpose()
                         << " " << state_point.bg.transpose() << " " << state_point.ba.transpose() << " " << state_point.grav << " " << feats_undistort->points.size() << endl;
                dump_lio_state_to_log(fp);
            }
        }

        status = ros::ok();
        rate.sleep();
    }

    /**************** save map ****************/
    /* 1. make sure you have enough memories
    /* 2. pcd save will largely influence the real-time performences **/
    if (pcl_wait_save->size() > 0 && pcd_save_en)
    {
        string file_name = string("scans.pcd");
        string all_points_dir(string(string(ROOT_DIR) + "PCD/") + file_name);
        pcl::PCDWriter pcd_writer;
        cout << "current scan saved to /PCD/" << file_name << endl;
        pcd_writer.writeBinary(all_points_dir, *pcl_wait_save);
    }

    fout_out.close();
    fout_pre.close();

    if (runtime_pos_log)
    {
        vector<double> t, s_vec, s_vec2, s_vec3, s_vec4, s_vec5, s_vec6, s_vec7;
        FILE *fp2;
        string log_dir = root_dir + "/Log/fast_lio_time_log.csv";
        fp2 = fopen(log_dir.c_str(), "w");
        fprintf(fp2, "time_stamp, total time, scan point size, incremental time, search time, delete size, delete time, tree size st, tree size end, add point size, preprocess time\n");
        for (int i = 0; i < time_log_counter; i++)
        {
            fprintf(fp2, "%0.8f,%0.8f,%d,%0.8f,%0.8f,%d,%0.8f,%d,%d,%d,%0.8f\n", T1[i], s_plot[i], int(s_plot2[i]), s_plot3[i], s_plot4[i], int(s_plot5[i]), s_plot6[i], int(s_plot7[i]), int(s_plot8[i]), int(s_plot10[i]), s_plot11[i]);
            t.push_back(T1[i]);
            s_vec.push_back(s_plot9[i]);
            s_vec2.push_back(s_plot3[i] + s_plot6[i]);
            s_vec3.push_back(s_plot4[i]);
            s_vec5.push_back(s_plot[i]);
        }
        fclose(fp2);
    }

    return 0;
}
