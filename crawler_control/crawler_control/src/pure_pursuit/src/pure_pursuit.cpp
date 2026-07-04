#include "geometry.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <pure_pursuit/pure_pursuit.h>
#include <std_msgs/Bool.h>
#include <std_msgs/String.h>
#include <std_msgs/UInt8.h>
#include <vector>
#include <yaml-cpp/yaml.h>

#define VERSION "1.0"
#define SPEED_TARGET_POINT 0
static int ctv = 0; // 1500
ControlCommand command; // 全局控制命令，传递给setControl_main()和command_publish()
ControlError ctlE = {44.4f, 44.4f, 44.4f}; // 添加误差反馈，数据初始化

int ld_i = 4;       // 2
bool turn_flag = 0; // 正式转弯标志位
bool gear_flag = 0; // 档位切换转弯点，第一次遇到1意味着转弯开始？规划距离（50cm）
bool turn_state = 0;                // 转弯状态？->第一次转弯的状态位
bool tem_flag = 0;                  // 时间约束标志
bool initial_approach_mode = false; // cnk 0514 初始接近模式
int approach_count = 0;             // cnk 0514 初始接近模式计数器
bool first_set = false;
bool adjust_error = false;
int count = 0;
double last_distance = std::numeric_limits<double>::max();

void ResetFlag() {
  ld_i = 4;      // 2
  turn_flag = 0; // 正式转弯标志位
  gear_flag = 0; // 档位切换转弯点，第一次遇到1意味着转弯开始？规划距离（50cm）
  turn_state = 0; // 转弯状态？->第一次转弯的状态位
  tem_flag = 0;   // 时间约束标志
}

// 构造函数
PurePursuit::PurePursuit()
    : private_nh_("~"), LOOP_RATE_(20), // 50
      is_planning_result_set_(false), is_carposition_set_(false),
      is_velocity_set_(false), desire_speed_com(0.0) /*,
                                desire_turn_angle(0.0)*/
{
  last_pos_time = ros::Time::now();
  initForROS();
}

// 析构函数
PurePursuit::~PurePursuit() {}

void PurePursuit::setPath_main(void) {
  int i = 0;
  waypose p1; // 临时变量

  // ROS 标准车辆坐标系：x 轴沿前进方向，y 轴沿左侧，z 轴向上
  float theta = car_position.yaw;
  memset(local_waypoints, 0, sizeof(local_waypoints));
  float cosval = cosf(theta);
  float sinval = sinf(theta);

  for (i = 0; i < planning_result.point_num; i++) {
    p1.point.x = planning_result.x[i] - car_position.position_x;
    p1.point.y = planning_result.y[i] - car_position.position_y;
    p1.heading = robot::geometry::NormalizeAngle(
        planning_result.heading[i] - car_position.yaw); // 航向差保持不变
    // 世界坐标 -> ROS 车辆坐标：x 为前进方向，y 为左侧
    local_waypoints[i].point.x = p1.point.x * cosval + p1.point.y * sinval;
    local_waypoints[i].point.y = -p1.point.x * sinval + p1.point.y * cosval;
    local_waypoints[i].heading = p1.heading;
    // if(i<=9){
    // ROS_INFO("D1#localpath# x:%.3f,y:%.3f,heading:%.3f",
    // local_waypoints[i].point.x,local_waypoints[i].point.y,local_waypoints[i].heading);
    // //debug_1
    // }
  }
}

void PurePursuit::updatestatus(void) {
  path_state = updatePathState();
  lat_track_state = updateLatTrackState();

  static int tnum = 0;
  tnum++;
  if (tnum > 3 * LOOP_RATE_) {
    // ROS_INFO("path_state: %d,track_state: %d", path_state, lat_track_state);
    tnum = 0;
  }
}

int PurePursuit::updatePathState(void) {
  float distance;
  if (planning_result.point_num == 0 ||
      planning_result.point_num < look_ahead_index)
    return PATH_FINISHED;

  if (!getNearestIndex(&nearest_waypoint_id)) {
    return PATH_INVALID;
  }
  if (getLatDistance(&distance)) {
    if (fabsf(distance) > kMaxPathDistance) {
      ROS_INFO("PATH_TOO_FAR ~dis~AAAAAAAAAA  %.3f", distance); //
      return PATH_TOO_FAR;
    } else {
      stop_path_flag = 0;
      return PATH_OK;
    }
  }
  ROS_INFO("D2#unmatched PathState , acquiesce PATH_INVALID"); //@ debug_2
  return PATH_INVALID;
}

int PurePursuit::updateLatTrackState(void) {
  if (!getLatDistance(&lat_error)) {
    if (lat_error < kMaxLatError)
      lat_track_state = CONTROL_OK;
    else
      lat_track_state = CONTROL_INIT;
  }
  switch (lat_track_state) {
  case CONTROL_INIT:
    return CONTROL_INIT;
    break;
  case CONTROL_OK:
  case CONTROL_OUT_OF_BOUND:
    if (lat_error < kStoppingLatError)
      return CONTROL_OK;
    else {
      ROS_INFO("CONTROL_OUT_OF_BOUND ~dis~  %.3f", lat_error); //
      return CONTROL_OUT_OF_BOUND;
    }
    break;
  case CONTROL_FINISHED:
    return CONTROL_FINISHED;
    break;
  default:
    return CONTROL_INIT;
  }
}

int PurePursuit::getNearestIndex(int *index) {
  int nearest_index = -1;
  int i, near_i = -1;
  float up_dis = 999.0;
  *index = -1;
  if (planning_result.point_num < 1) {
    return 0;
  }

  for (i = 0; i < planning_result.point_num; i++) {
    // in 1224 修改了最近点逻辑？ 要判断 是否丢点

    // cm >=0
    float dis = robot::geometry::hypotFast(local_waypoints[i].point.x * 100,
                                           local_waypoints[i].point.y * 100);
    if (dis < up_dis) {
      up_dis = dis;
      near_i = i;
    }
    if (dis > 300) {
      // break;
      continue;
    }
    /*if (local_waypoints[i].point.y <= 0.0f) // 局部转换，前向y正值最近的点
    @add continue; nearest_index = i; break;*/
  }
  nearest_dis = up_dis;
  nearest_index = near_i;
  if (!nearest_index) {
    int j = 0;
    for (j; j < nearest_index; j++) {
      if (planning_result.gear[j] != planning_result.gear[nearest_index]) {
        nearest_index = j;
      }
    }
    nearest_index = nearest_index != near_i ? 0 : near_i;
    // ROS_INFO("D33#nearest _index:%d,near_i:%d,x:%.3f,y:%.3f",
    // nearest_index,near_i,
    //    local_waypoints[near_i].point.x,local_waypoints[near_i].point.y);
  }
  // ROS_INFO("D3#nearest _index:%d,dis:%.3f,x:%.3f,y:%.3f",
  // nearest_index,nearest_dis,
  //    local_waypoints[nearest_index].point.x,local_waypoints[nearest_index].point.y);
  //    //debug_3
  if (nearest_index == -1) // not found
  {
    ROS_ERROR_THROTTLE(1, "ERROR ~not found nearest_waypoint~ : %d ",
                       nearest_index);
    return 0;
  }
  if (nearest_dis > kMaxPathDistance) {
    ROS_INFO("PATH_TOO_FAR ~dis~BBBBB  %.3f", nearest_dis); //
    return PATH_TOO_FAR;
  }
  *index = nearest_index;
  return 1;
}

int PurePursuit::getLatDistance(
    float *distance) { //:1224 debug == 0和航向角有关
  *distance = 999.99f;
  if (nearest_waypoint_id > planning_result.point_num)
    return 0;

  if (nearest_waypoint_id ==
      0 /*&& local_waypoints[nearest_waypoint_id].point.x < 0.20*/) {
    // ROS 坐标系下 y 为横向（左侧为正）
    *distance = local_waypoints[nearest_waypoint_id].point.y;
  }
  if (nearest_waypoint_id >= 1) {
    // getDistanceToLine 在 ROS 坐标系下左侧为正，与 y 轴方向一致
    *distance = getDistanceToLine(&local_waypoints[nearest_waypoint_id - 1],
                                  &local_waypoints[nearest_waypoint_id]);
  }
  // ROS_INFO("nearest_waypoint_id : %d",nearest_waypoint_id);
  ctlE.e_pos = *distance;
  return 1;
}
// 计算车辆位置（相对于 (0,0)作为原点）到点 p1 和点 p2
// 连线的垂直距离
float PurePursuit::getDistanceToLine(waypose *p1,
                                     waypose *p2) { // 与车坐标点（0，0）算距离
  float A = p1->point.y - p2->point.y;
  float B = p2->point.x - p1->point.x;
  float C = p1->point.x * p2->point.y - p1->point.y * p2->point.x;

  if (fabsf(A * A + B * B) < 0.01f)
    return sqrtf(powf(p1->point.x, 2.0f) + powf(p1->point.y, 2.0f));
  return (-C / sqrtf(A * A + B * B));
}

int PurePursuit::findGoalPoint(void) {
  if (planning_result.point_num < 1) // :2504 <=改为 <
    return 0;
  if (planning_result.point_num > look_ahead_index) {
    goal = &local_waypoints[look_ahead_index];
    goal_id = look_ahead_index;
    // return 1;
  } else {
    goal = &local_waypoints[planning_result.point_num - 1];
    path_state = PATH_FINISHED;
    goal_id = planning_result.point_num - 1;
  }
  // ROS_INFO("D0goal->y:%.3f,x:%.3f,heading:%.3f,ld_index:%d", goal->point.y,
  // goal->point.x,goal->heading,goal_id); //debug_0
  return 1;
}

int PurePursuit::isGoalValid(void) {
  // 预查范围内保证 横向偏差
  int i;
  float lat_error_temp; // cm
  if (nearest_waypoint_id < 0 || look_ahead_index < 0 ||
      planning_result.point_num < look_ahead_index)
    return 0;

  max_goal_lat_error = 0.0f;

  for (i = nearest_waypoint_id; i < look_ahead_index; i++) {
    if (isinf(desire_ICR))
      // ROS 坐标系下 y 为横向
      lat_error_temp = fabsf(local_waypoints[i].point.y) * 100;
    else
      // ROS 坐标系下 x 为纵向、y 为横向
      lat_error_temp = fabsf(
          sqrtf(powf(local_waypoints[i].point.x * 100, 2.0f) +
                powf(local_waypoints[i].point.y * 100 - desire_ICR, 2.0f)) -
          fabsf(desire_ICR)); // 局部路径圆与行走半径圆差异值

    if (lat_error_temp > max_goal_lat_error)
      max_goal_lat_error = lat_error_temp;

    // ROS_INFO("D6#lat_error_temp %.3f,ICR %.3f", lat_error_temp,desire_ICR);
    // //debug_6
    if (lat_error_temp > kMaxLatError) {
      return 0;
    }
  }
  return 1;
}

/**
 * @brief Pure Pursuit 角速度计算（ROS 车辆坐标系）
 *
 * 几何关系：
 *   车辆以后轴中心为原点，local_x 轴沿前进方向，local_y 轴沿左侧。
 *   预瞄点 goal 在车辆坐标系下的坐标为 (x, y)。
 *
 * 推导：
 *   设车辆沿半径为 R 的圆弧驶向预瞄点，圆心在 y 轴上 (0, R)。
 *   由 x^2 + (R - y)^2 = R^2 可得：
 *     R = (x^2 + y^2) / (2y)
 *   曲率 κ = 1/R = 2y / (x^2 + y^2)
 *
 *   由于 y 轴正方向为左侧，而左转对应负 turn_value，因此取：
 *     κ = -2y / (x^2 + y^2)
 *
 * 角速度：
 *   ω = v * κ
 *   其中 v 为车辆线速度（m/s）。
 *
 * 输出符号：
 *   左转对应负，右转对应正。
 *
 * @param goal 预瞄点在车辆坐标系下的位置
 * @param v_m_s 车辆线速度（m/s）
 * @return 角速度 omega（rad/s）
 */
static float computePurePursuitOmega(const waypose *goal, float v_m_s) {
  // 预瞄距离的平方 L_d^2 = x^2 + y^2
  float ld_sq = goal->point.x * goal->point.x + goal->point.y * goal->point.y;
  
  // 防止除零，预瞄点过近时视为直行
  if (ld_sq < 1e-6f) {
    return 0.0f;
  }

  // ROS 坐标系下 y 为横向（左侧为正），左转对应负 omega，
  // 因此曲率 κ = -2y / L_d^2
  float curvature = -2.0f * goal->point.y / ld_sq;
  ROS_INFO("[setControl_main] ld_sq: %.3f, long_x: %.3f, lat_y: %.3f", ld_sq, goal->point.x, goal->point.y);
  //角速度修正权重
  const float omega_weight = 0.5f;
  // 角速度 ω = v * κ
  float omega = omega_weight * v_m_s * curvature;

  // 根据差速车运动学限制最大角速度
  // 理论上差速车可以原地旋转（R -> 0），但实际受左右轮最大速度差限制
  // ω_max = Δv_max / L，其中 L = 0.75m 为轮距
  // 这里先按固定 1.5 rad/s 限制，需根据实际电机/轮速差能力标定
  const float kMaxOmega = 0.471*0.75f;

  if (omega > kMaxOmega) {
    omega = kMaxOmega;
  } else if (omega < -kMaxOmega) {
    omega = -kMaxOmega;
  }

  return omega;
}

void PurePursuit::setControl_main(void) {
  // ========== 0. 默认输出与安全初始化 ==========
  // 每一周期先输出安全的默认命令：直行、停车
  // 这样即使后续路径异常，也能保证车辆不乱动
  command.turn_value = 0;
  command.drive_value = 0;
  desire_ICR = INFINITY;

  ROS_INFO("[setControl_main] start: nearest=%d, point_num=%d, turn_phase=%d",
           nearest_waypoint_id, planning_result.point_num,
           static_cast<int>(turn_phase_));

  // ========== 1. 路径有效性检查 ==========
  // point_num < 1：没有路径点
  // nearest_waypoint_id 越界：最近点索引无效
  if (planning_result.point_num < 1 || nearest_waypoint_id < 0 ||
      nearest_waypoint_id >= planning_result.point_num) {
    look_ahead_index = 0;
    return;
  }

  // 紧急停车模式：障碍物在 3 个点以内，直接停车且停止转向
  // 注意：drive_value 在 calculateDriveValue 中已为 0，这里只需把 turn_value 也清零
  if (avoid_state_data == 3) {
    ROS_WARN("[setControl_main] emergency stop: avoid_state_data=3, drive=0, turn=0");
    // command.turn_value = 0;
    return;
  }

  // 选择控制基准点
  // 如果第一个点是 gear=2 转弯点，强制从第 0 个点开始跟踪，避免跳过起点转弯点
  // 否则使用全局最近点
  int control_base_idx = nearest_waypoint_id;
  if (planning_result.point_num > 0 && planning_result.gear[0] == 2) {
    control_base_idx = 0;
    ROS_INFO("[setControl_main] first point is gear=2, force tracking from index 0");
  }

  // ========== 2. 纵向控制 ==========
  // drive_value 表示车辆线速度，使用 planning_result 中最近路径点的规划速度
  // 注意：速度基准始终用 nearest_waypoint_id，不能因起点是 gear=2 就用 speed[0]（通常为 0）
  // planning_result.speed 单位：m/s
  // drive_value 单位为 cm/s，例如 0.5 m/s -> 50
  float nearest_speed_m_s = planning_result.speed[nearest_waypoint_id];
  command.drive_value =
      calculateDriveValue(nearest_speed_m_s, 0.0f, false, avoid_state_data);

  // 车辆线速度（m/s），用于角速度计算
  float v_m_s = nearest_speed_m_s;

  // 当前航向误差（度），用于控制误差发布与调试
  ctlE.e_theta = local_waypoints[0].heading * 180.0f / M_PI;

  // ========== 3. 基于距离选择目标预瞄索引 ==========
  // 已知相邻路径点间距约为 0.25m
  // 预瞄距离根据车速动态选择：L_d = 1.5 * v + 0.3（m）
  // 低速时预瞄近，高速时预瞄远，避免跟踪过远导致偏差
  const float weight_l = 2.0f; //预瞄点权重
  const float kMaxLookAheadDistM = 4.0f;  // 最大预瞄距离 2.0m（约 20 个点）
  const float kMinLookAheadDistM = 1.0f;  // 最小预瞄距离 1.0m（约 4 个点）
  float lookahead_dist_m = weight_l * v_m_s + 0.3f;
  if (lookahead_dist_m > kMaxLookAheadDistM) {
    lookahead_dist_m = kMaxLookAheadDistM;
  }
  if (lookahead_dist_m < kMinLookAheadDistM) {
    lookahead_dist_m = kMinLookAheadDistM;
  }

  // 已知相邻路径点沿路径间距约为 0.25m
  // 直接用预瞄距离除以点间距得到预瞄索引偏移量
  // 用 round() 而不是 int()，避免 1.4 倍间距被截断为 1 倍
  const float kPointSpacing = 0.25f;  // m
  int target_idx = control_base_idx +
                   static_cast<int>(std::round(lookahead_dist_m / kPointSpacing));
  if (target_idx >= planning_result.point_num) {
    target_idx = planning_result.point_num - 1;
  }

 

  // ========== 4. 检测前方 gear=2 转弯点 ==========
  // 在 [control_base_idx, target_idx] 范围内搜索第一个 gear == 2 的点
  // 如果存在，强制将其作为预瞄点，并进入转弯接近状态
  int gear2_idx = -1;
  for (int i = control_base_idx; i <= target_idx; ++i) {
    if (planning_result.gear[i] == 2) {
      gear2_idx = i;
      break;
    }
  }

  if (gear2_idx != -1) {
    
    target_idx = gear2_idx;
    look_ahead_index = target_idx;
    if (turn_phase_ == TurnPhase::NONE) {
      turn_phase_ = TurnPhase::APPROACH;
      turn_target_idx_ = gear2_idx;
    }
  }
 ROS_INFO("[setControl_main] lookahead: v=%.3f m/s, gear2_idx=%d, target_idx=%d",
           v_m_s, gear2_idx, target_idx);


  // ========== 5. 转弯状态机 ==========
  // 5.1 APPROACH：接近转弯点
  if (turn_phase_ == TurnPhase::APPROACH) {
    // 计算车辆到转弯目标点的直线距离
    float dist_to_turn = std::hypot(local_waypoints[turn_target_idx_].point.x,
                                    local_waypoints[turn_target_idx_].point.y);

    // 接近转弯点时提前减速，避免到达 gear=2 点才突然停车
    // 注意：gear=2 点本身的规划速度通常为 0，不能作为接近速度
    // 这里使用固定接近速度 0.30 m/s，在 0.5m 内线性减速到 0.15 m/s
    const float kMaxApproachSpeed = 0.30f;  // m/s
    const float kDecelStartDist = 0.5f;     // m
    float approach_speed = kMaxApproachSpeed;

    if (dist_to_turn < kDecelStartDist) {
      approach_speed *= (dist_to_turn / kDecelStartDist);
      if (approach_speed < 0.15f) {
        approach_speed = 0.15f;  // 保留最小速度，避免完全停滞
      }
    }

    command.drive_value =
        calculateDriveValue(approach_speed, 0.0f, false, avoid_state_data);

    // 进入转弯执行状态的条件（满足任一即可）：
    // 1. 距离转弯点足够近（< 0.05m）：精确触发，确保车辆基本到达转弯点
    // 2. 最近点索引已经超过转弯点，且车辆仍在转弯点附近（< 0.10m）：
    //    兜底，防止定位/最近点跳变导致错过，但 10cm 已足够覆盖一个点距内的波动
    // ROS 坐标系下 x 为前进方向
    if (dist_to_turn < 0.1f || local_waypoints[turn_target_idx_].point.x < 0.05f ||
        (nearest_waypoint_id > turn_target_idx_ && dist_to_turn < 0.10f)) {
      turn_phase_ = TurnPhase::EXECUTING;
    }
  }

  // 5.2 EXECUTING：执行原地转弯
  if (turn_phase_ == TurnPhase::EXECUTING) {
    std::lock_guard<std::mutex> lock(turn_completed_mutex_);
    command.drive_value = 0;  // 停车转弯

    // heading_error 为车辆航向与最近路径点切线方向的夹角
    // 当 heading_error ≈ 0 时，车辆已对准下一段路径
    float heading_error = local_waypoints[0].heading;
    ROS_INFO( "[setControl_main] turn executing: heading_error=%.3f rad",
                        heading_error);

    turn_completed_msg_.request.x = planning_result.x[look_ahead_index];
    turn_completed_msg_.request.y = planning_result.y[look_ahead_index];
    turn_completed_msg_.response.turn_finish = false;
    if (std::fabs(heading_error) <= 0.10f) {
      ROS_INFO("[setControl_main] turn completed: heading_error=%.3f rad, publish /turn_completed",
               heading_error);
      // 转弯完成，发布 /turn_completed 信号通知上层 planner 丢弃该 gear=2 点
      turn_completed_msg_.response.turn_finish = true;

      // 恢复普通跟踪状态
      turn_phase_ = TurnPhase::NONE;
      turn_target_idx_ = -1;
    } else {
      // heading_error > 0：路径方向在车辆左侧，需要左转（负值）
      // heading_error < 0：路径方向在车辆右侧，需要右转（正值）
      command.turn_value = (heading_error > 0.0f) ? -10 : 10;
      turn_completed_msg_.response.turn_finish = false;
      return;
    }
  }

  // ========== 6. 正常 Pure Pursuit 跟踪 ==========
  look_ahead_index = target_idx;
  if (!findGoalPoint()) {
    return;
  }

  // 使用 Pure Pursuit 计算角速度（rad/s）
  // 单片机端对转速乘以 π 并放大 100 倍，因此 omega -> turn_value 需乘 100/π
  float omega = computePurePursuitOmega(goal, v_m_s);

  command.turn_value = static_cast<int>(omega * 100.0f / M_PI);

  // ========== 7. 输出限幅 ==========
  // turn_value 限制在 [-20, 20]，对应角速度 [-0.2*π, 0.2*π] rad/s
  // drive_value 限制在 [0, 150]，需根据车辆最大速度调整
  command.turn_value = std::max(-20, std::min(20, command.turn_value));
  command.drive_value = std::max(0, std::min(150, command.drive_value));

  ROS_INFO("[setControl_main] final cmd: drive=%d (cm/s), turn=%d, ld_idx=%d, nearest=%d, base=%d, goal(x,y)=(%.3f,%.3f), omega=%.3f rad/s",
           command.drive_value, command.turn_value,
           look_ahead_index, nearest_waypoint_id, control_base_idx,
           planning_result.x[look_ahead_index], planning_result.y[look_ahead_index], omega);

  // ========== 8. 终点检测 ==========

  if (avoid_state_data != 1) {
    if (!c_finish && path_state == PATH_FINISHED &&
        // ROS 坐标系下 x 为前进方向
        fabsf(goal->point.x) < 0.2f && fabsf(vehicle_info.vehicle_speed) > 2) {
      std::string filePath =
          "/home/nvidia/cralwer_control/logs/ControlStatus.txt";
      std::ofstream outFile(filePath);
      if (!outFile.is_open()) {
        ROS_WARN("can't open ControlStatus.txt");
      }
      float c_length =
          sqrt(pow(goal->point.x, 2.0f) + pow(goal->point.y, 2.0f)) * 100; // cm
      if (c_length < 10) {
        ROS_INFO("[setControl_main] control finished: end_length=%.3f cm", c_length);
        c_finish = true;
        path_state = PATH_INIT;
        lat_track_state = CONTROL_FINISHED;
        outFile << c_finish << std::endl;
        outFile.close();
      }
    } else {
      c_finish = false;
    }
  }
}




void PurePursuit::act(void) {
  switch (path_state) {
  case PATH_OK:
    switch (lat_track_state) {
    case CONTROL_INIT:
    case CONTROL_OK:
      stopping_req_by_lat_control = 0;
      break;
    case CONTROL_OUT_OF_BOUND:
    case CONTROL_FINISHED:
    default:
      stopping_req_by_lat_control = 1;
      break;
    }
    break;
  case PATH_INVALID:
    stopping_req_by_lat_control = 1; //@cnk
    ROS_WARN("PATH_INVALID.");
    break;
  case PATH_TOO_FAR:
    stop_path_flag = 1; // add
    // ROS_INFO("PATH_TOO_FAR ~stop_ =  %d", stop_path_flag); //
    break;
  case PATH_INIT:
  case PATH_FINISHED:
  default:
    stopping_req_by_lat_control = 1;
    ;
    break;
  }
}
void PurePursuit::latcontrol_algorithm(void) // 20ms
{
  setPath_main();
  updatestatus();
  setControl_main();
  
  act();
}


// 注意：新路径对齐逻辑已删除，差速车可直接用 Pure Pursuit 处理 gear=2 点
void PurePursuit::callbackFromPlanningResult(
    const util::LocalPathConstPtr &msg_planning_result) {
  planning_result = *msg_planning_result;
  error_status = (planning_result.point_num < 1)
                     ? 1
                     : 0; // planning_result.point_num > 50 ||
  if (error_status) {
    is_planning_result_set_ = false;
    ROS_ERROR_THROTTLE(1, "~ point_num is wrong : %d ",
                       planning_result.point_num);
    return;
  }

  // 不再通过 pathtype 或路径起点变化做新路径对齐
  // 差速车转弯半径小，避障/恢复路径的第一个 gear=2 点直接由 Pure Pursuit 处理
  is_planning_result_set_ = true;
}

/////////////////////////////////////////////////////////////////////
void PurePursuit::callbackFromSpeedInfo(
    const util::LocalPoseConstPtr &msg_speed_info) {
  vehicle_info = *msg_speed_info;
  int v_speed = vehicle_info.vehicle_speed; // int32 cm/s
  is_velocity_set_ = true;
}

void PurePursuit::callbackFromPos(
    const util::PositionConstPtr &msg_carposition) {
  car_position = *msg_carposition;
  //is_carposition_set_ = true;
  // 如果超过 1 秒没有收到新位置，认为位置信息丢失
  
   last_pos_time = ros::Time::now();
  
}

void PurePursuit::callbackFromDirectControl(
    const mower_msgs::Direct_ControlConstPtr &msg_direct_control) {
  direct_control = *msg_direct_control;
  ROS_INFO("direct_control");
}

/* task-stop */
void PurePursuit::callbackFromTaskstop(const std_msgs::Bool &task_msg) {
  stop_car_flag = task_msg.data;
}
/* Avoidstate */
void PurePursuit::callbackFromAvoidstate(const std_msgs::UInt8 &avoid_msg) {
  avoid_state_data = avoid_msg.data;
}
void PurePursuit::callbackFromSignal(const std_msgs::StringConstPtr &string) {

  std::cout<<"string->data:"<<string->data<<std::endl;
  if (string->data == "rise") {
    mower_height_cfg = 1;
    ROS_INFO("mower_height = 1");
  }
  else if (string->data == "stop") {
    mower_height_cfg = 0;
    ROS_INFO("mower_height = 0");
  }
  else if (string->data == "falling") {
    mower_height_cfg = 2;
    ROS_INFO("mower_height = 2");
  }
  else if (string->data == "open") {
    mover_bool_cfg = 1;
    ROS_INFO("mover_bool_cfg = 1");
  }
  else if (string->data == "close") {
    mover_bool_cfg = 0;
    ROS_INFO("mover_bool_cfg = 0");
  }
}

bool PurePursuit::callbackTurnCompleted(pure_pursuit::TurnCompletedRequest &req, pure_pursuit::TurnCompletedResponse &res)
{
  std::lock_guard<std::mutex> lock(turn_completed_mutex_);
  if (req.x == turn_completed_msg_.request.x && req.y == turn_completed_msg_.request.y)
  {
    if (turn_completed_msg_.response.turn_finish)
    {
      ROS_INFO("turn success!");
      res.turn_finish = true;
      return true;
    }
    else
    {
      ROS_INFO("turn not success!");
      res.turn_finish = false;
      return true;
    }
  }
  else
  {
    ROS_INFO("turn point error!");
    return false;
  }
}

void PurePursuit::command_publish() {
  mower_msgs::VehicleCmd cmd_msg;
  cmd_msg.turn_value = command.turn_value;
  cmd_msg.drive_value = command.drive_value;
  cmd_msg.ad_control_enable = 1;

  if (planning_result.gear[SPEED_TARGET_POINT] != 0) {
    cmd_msg.mover_bool = mover_bool_cfg;     // 通过yaml 配置
    cmd_msg.mower_height = mower_height_cfg; //
    if (planning_result.gear[SPEED_TARGET_POINT] == 1) {
      if (planning_result.speed[SPEED_TARGET_POINT] != 0) {
        cmd_msg.gear_model = cmd_msg.D_Gear; // D: 3(1)  N: 2(2)  p: 0
      } else {
        cmd_msg.gear_model = cmd_msg.p_Gear; // gear=1,but speed=0;
        cmd_msg.mover_bool = mover_bool_cfg;
      }
    } else if (planning_result.gear[SPEED_TARGET_POINT] == 2) {
      cmd_msg.gear_model = cmd_msg.N_Gear;
    } else {
      cmd_msg.gear_model = cmd_msg.R_Gear;
    }
  } else {
    cmd_msg.gear_model = cmd_msg.p_Gear;
    cmd_msg.mover_bool = mover_bool_cfg;
    cmd_msg.turn_value = 0; // 1500
    cmd_msg.drive_value = 0; // 1500
  }
  // @规划最后一个点问题：停车逻辑还需要验证？？
  if (cmd_msg.gear_model == 0) {
    cmd_msg.mover_bool = mover_bool_cfg;
    cmd_msg.turn_value = 0; // 1500
    cmd_msg.drive_value = 0; // 1500
    cmd_msg.ad_control_enable = 0;
    ROS_INFO("$$bb stop ~~ gear =0");
  }

  // 当前由 TurnPhase 状态机替代，
  //     不再被设置，此处保留条件但注释掉，避免误导。
  if (stop_car_flag || c_finish ||  stop_path_flag) // cnk 0512
  {
    mower_msgs::VehicleCmd quit_cmd;
    quit_cmd.ad_control_enable = 1; // 要不要开
    quit_cmd.mower_height = cmd_msg.mower_height;
    quit_cmd.gear_model = cmd_msg.gear_model;
    quit_cmd.mover_bool = cmd_msg.mover_bool;
    // 对 last_turn_value 做安全限幅，防止初始值异常（如 1501）直接下发
    // 正常跟踪时 turn_value 已限幅在 [-20, 20]，此处兜底保护
    quit_cmd.turn_value = (last_turn_value < 20 && last_turn_value > -20)
                              ? last_turn_value
                              : 0;
    quit_cmd.drive_value = 0;
    if (c_finish) {
      quit_cmd.ad_control_enable = 0;
      quit_cmd.gear_model = cmd_msg.p_Gear;
      quit_cmd.mover_bool = mover_bool_cfg;
      ROS_INFO("ALL~~~~~~~~~Finished");
    }

    quit_cmd.header.stamp = ros::Time::now(); // 设置时间戳
    pub_Command.publish(quit_cmd);
    // 保障停车
    static int stop_num = 0;
    stop_num++;

    if (stop_num > 10 && stop_path_flag) // TODO @cnk
    {
      stop_num = stop_num > 15 ? 0 : stop_num;
      if (!planning_result.x.empty()) {
        // std_msgs::Bool turn_completed_msg;
        // turn_completed_msg.data = true;
        // turn_completed_.publish(turn_completed_msg); // 成员名保持历史命名，实际发布 /turn_completed
        // ROS_INFO("Published turn_completed: true");
      } else {
        ROS_WARN(
            "Planning result is empty. Cannot publish turn_completed.");
      }
    }
  } else {
    last_turn_value = cmd_msg.turn_value;    //: 1225 add
    cmd_msg.header.stamp = ros::Time::now(); // 设置时间戳
    
    // 添加调试日志，特别关注状态切换时的控制命令
    static int last_avoid_state = -1;
    if (avoid_state_data != last_avoid_state) {
      ROS_INFO("State changed: %d -> %d, cmd: turn=%d, drive=%d, gear=%d", 
               last_avoid_state, avoid_state_data, 
               cmd_msg.turn_value, cmd_msg.drive_value, cmd_msg.gear_model);
      last_avoid_state = avoid_state_data;
    }
    pub_Command.publish(cmd_msg);

     ROS_INFO("D10$cmd pub $ t_v %d,d_v %d,gear %d,ad %d", cmd_msg.turn_value,
     cmd_msg.drive_value, cmd_msg.gear_model,cmd_msg.ad_control_enable);
    // //@Debug10
  }
}

void PurePursuit::controlError_publish() {
  mower_msgs::ControlError ctlE_msg;
  ctlE_msg.header.stamp = ros::Time::now(); // 添加时间戳
  ctlE_msg.e_pos = ctlE.e_pos;
  ctlE_msg.e_theta = ctlE.e_theta;
  // float du = ctlE.e_theta; //角度 degree
  ctlE_msg.e_lat = 44.4; // 暂未赋值
  // ROS_INFO("CE$ pub $ e_pos %.4f,e_theta_deg %.4f", ctlE_msg.e_pos,
  // ctlE_msg.e_theta);
  pub_CtlError.publish(ctlE_msg);
}

void PurePursuit::initForROS() {
  // ros parameter settings

  // sub_topics_params
  nh_.param("sub_planning_result_topic", sub_planning_result_topic,
            std::string("/lawn_mower/global_path")); // 局部规划
  nh_.param("sub_speed_info_topic", sub_speed_info_topic,
            std::string("/nanobot/localpose")); // 厘米级定位的发布
  nh_.param("sub_pos_topic", sub_pos_topic,
            std::string("/Mower/position")); // 定位信息
  nh_.param("sub_direct_control_topic", sub_direct_control_topic,
            std::string("/mower/direct_control")); // APP控制
  nh_.param("sub_stop_car_topic", sub_stop_car_topic,
            std::string("/mower/stop_car")); // 视觉停车
  nh_.param("sub_avoidstate_topic", sub_avoidstate_topic,
            std::string("/lawn_mower/avoid_state")); // 状态数据

  // setup subscriber
  sub1_ = nh_.subscribe(sub_planning_result_topic, 1,
                        &PurePursuit::callbackFromPlanningResult, this);
  sub2_ = nh_.subscribe(sub_speed_info_topic, 1,
                        &PurePursuit::callbackFromSpeedInfo, this);
  sub4_ = nh_.subscribe(sub_pos_topic, 5, &PurePursuit::callbackFromPos, this);
  sub5_ = nh_.subscribe(sub_direct_control_topic, 1,
                        &PurePursuit::callbackFromDirectControl, this);
  substop = nh_.subscribe(sub_stop_car_topic, 1,
                          &PurePursuit::callbackFromTaskstop, this);
  sub_avoid = nh_.subscribe(sub_avoidstate_topic, 1,
                            &PurePursuit::callbackFromAvoidstate, this);
  sub_signal_ = nh_.subscribe("/signal", 1, 
                            &PurePursuit::callbackFromSignal, this);

  // setup publisher
  pub_Command = nh_.advertise<mower_msgs::VehicleCmd>("/vehicle/cmd", 1);
  pub_Controlok = nh_.advertise<mower_msgs::ControlOk>("/mower/control_ok", 1);
  pub_CtlError = nh_.advertise<mower_msgs::ControlError>("/mower/control_error", 1);
  // turn_completed_ = nh_.advertise<std_msgs::Bool>("/turn_completed", 1); //@cnk

  turn_completed_srv_ = nh_.advertiseService("/turn_completed", &PurePursuit::callbackTurnCompleted, this);
}

//
int reset_cnt = 0;
int reset_pos_cnt = 0;
void PurePursuit::run() {
  ROS_INFO_STREAM("control node is ready!");
  ROS_INFO_STREAM("hl_control version: " << getVersion());
  // 读取 YAML 配置文件
  try {
    YAML::Node config = YAML::LoadFile("/home/nvidia/crawler_control/src/"
                                       "pure_pursuit/chassis.yaml"); // 注意路径
    chassis_type = config["chassis_type"].as<int>();
    wheel_track = config["control_params"]["wheel_track"].as<float>();
    // wheel_base = config["control_params"]["wheel_base"].as<float>();
    mover_bool_cfg = config["control_params"]["mover_bool"].as<int>();
    mower_height_cfg = config["control_params"]["mower_height"].as<int>();
    std::cout << "Chassis Type: " << chassis_type << std::endl;
    std::cout << "Wheel Track: " << wheel_track << std::endl;
    std::cout << "Basic Config ## m_bool: " << mover_bool_cfg
              << ", --m_height: " << mower_height_cfg << std::endl;
    // std::cout << "Wheel Base: " << wheel_base << std::endl;
  } catch (const YAML::BadFile &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    // return;
  }

  mower_msgs::ControlOk Control_Ok_msg;
  ros::Rate loop_rate(LOOP_RATE_); // init: 50

  // 添加状态切换过渡变量
  static bool was_reversing = false;
  static bool first_stop = false;
  static ros::Time reverse_stop_time;
  
  while (ros::ok()) {
    ros::spinOnce();
    if ((ros::Time::now() - last_pos_time).toSec() > 1.0) {
        is_carposition_set_ = false;
    }else{
        is_carposition_set_ = true;
    }
    if(avoid_state_data==4){
      reset_pos_cnt++;
      if (reset_pos_cnt == 20) {
        reset_pos_cnt = 0;
        is_carposition_set_ = false;
      }
    }

    Control_Ok_msg.is_control_ok = true;
    std::cout<<"is_carposition_set_ = "<<is_carposition_set_<<std::endl;
    if(!is_carposition_set_ || stop_car_flag)
    {
        mower_msgs::VehicleCmd stop_cmd;
        stop_cmd.turn_value = 0; // 1500
        stop_cmd.drive_value = 0; // 1500停车
        stop_cmd.ad_control_enable = 1;
        stop_cmd.gear_model = 1;
        stop_cmd.mover_bool = mover_bool_cfg;
        stop_cmd.mower_height = mower_height_cfg;
        stop_cmd.header.stamp = ros::Time::now();
        pub_Command.publish(stop_cmd);
        ROS_INFO("NO position or stop_car");
        loop_rate.sleep();
        continue;
    }
    // 添加反向状态切换处理
    if (was_reversing && !first_stop && avoid_state_data != 4) {
      reverse_stop_time = ros::Time::now();
      first_stop = true;
      ROS_INFO("Transitioning from reverse to forward...");
    }
    
    if(avoid_state_data==4 && !stop_car_flag)
    {
      ROS_INFO("avoid_state_data==4~~~~~~~~~~~~~~~~~~~~");
      mower_msgs::VehicleCmd cmd_msg;
      cmd_msg.turn_value = 0; // 1500
      cmd_msg.drive_value = -15; // 1300
      cmd_msg.ad_control_enable = 1;
      cmd_msg.gear_model = 1; // gear=1,but speed=0;
      cmd_msg.mover_bool = mover_bool_cfg;
      cmd_msg.mower_height = mower_height_cfg;
      cmd_msg.header.stamp = ros::Time::now(); // 添加时间戳？？
      pub_Command.publish(cmd_msg);
      was_reversing = true;
      loop_rate.sleep();
      continue;
    }
    
    // 状态切换后的过渡处理？？
    if (was_reversing && avoid_state_data != 4) {
      double transition_time = (ros::Time::now() - reverse_stop_time).toSec();
      
      // 先短暂停车过渡，然后完全停止？？
      if (transition_time < 5) { // 添加0.3秒，避免全速切换
        mower_msgs::VehicleCmd stop_cmd;
        stop_cmd.turn_value = 0; // 1500
        stop_cmd.drive_value = 0; // 1500停车
        stop_cmd.ad_control_enable = 1;
        stop_cmd.gear_model = 1;
        stop_cmd.mover_bool = mover_bool_cfg;
        stop_cmd.mower_height = mower_height_cfg;
        stop_cmd.header.stamp = ros::Time::now();
        pub_Command.publish(stop_cmd);
        ROS_INFO("Stopping after reverse, transition_time: %.3f, avoid_state: %d", 
                 transition_time, avoid_state_data);
        loop_rate.sleep();
        continue;
      } else {
        // 过渡完成，恢复正常控制
        was_reversing = false;
        first_stop = false;
        ROS_INFO("Reverse transition completed, resuming normal control, avoid_state: %d", 
                 avoid_state_data);
      }
    }

    /*** 信号处理，正常控制 ***/
    // //驾驶模式时修改
    if (is_planning_result_set_ && is_carposition_set_ && !was_reversing) // 驾驶模式时修改？不添加过渡状态检查？

    {
      latcontrol_algorithm();
      command_publish();
      controlError_publish();
      
    } else {
      // 如果在过渡期间，发送停车命令
      if (was_reversing) {
        mower_msgs::VehicleCmd stop_cmd;
        stop_cmd.turn_value = 0; // 1500
        stop_cmd.drive_value = 0; // 1500
        stop_cmd.ad_control_enable = 1;
        stop_cmd.gear_model = 1;
        stop_cmd.mover_bool = mover_bool_cfg;
        stop_cmd.mower_height = mower_height_cfg;
        stop_cmd.header.stamp = ros::Time::now();
        pub_Command.publish(stop_cmd);
        ROS_INFO("Transition period - sending stop command");
      }
      
      static int scount = 0;
      scount++;
      if (scount > 3 * LOOP_RATE_) {
        if (!is_planning_result_set_) {
          ROS_INFO(" $$ no planning");
        }

        scount = 0;
      }
    }

    /****** 打印信息 ******/
    static int tnum = 0;
    tnum++;
    if (tnum > 3 * LOOP_RATE_) {
      auto now = ros::Time::now();
      tnum = 0;
    }

    pub_Controlok.publish(Control_Ok_msg);
    loop_rate.sleep();
  }
}

// 纵向控制  current_speed 现指最近点 规划值
// desire_speed 这里desire_speed没有用上，且会报段错误
int calculateDriveValue(float current_speed, float desire_speed, bool flag,
                        uint8_t avoid_state_data) {
  if (flag) {
    return 0; // 停车/转弯状态
  }
  if (avoid_state_data == 3) {
    return 0; // 紧急停车
  }
  if (avoid_state_data == 2) {
    return 30; // 避障降速
  }

  // current_speed 单位 m/s
  // drive_value 单位为 cm/s，例如 0.5 m/s -> 50
  // 底层接收后可直接作为左右轮目标速度的平均值
  int dv = static_cast<int>(current_speed * 100.0f);
  if (dv < 0) {
    dv = 0;
  } else if (dv > 150) {
    dv = 150;  // 对应 1.5 m/s，需根据车辆最大速度调整
  }
  return dv;
}

// 非标定法 计算转弯值
// int calculateTurnValue(bool latest_gear_flag, waypose local_waypose,
//                        waypose *c_goal, unsigned char cgoal_id, int last_tv) {
//   // 1判断上一转值？ 垂直 直线 曲线
//   static bool dev_flag = 0; // 跟踪偏差
//   static bool sl_flag = 0;  // 直线
//   static bool cu_flag = 0;  // 曲线
//   static int t_value = ctv;
//   static float h = 0;
//   int det_tv = 0;

//   int x_sign = local_waypose.point.x > 0 ? 1 : -1; // 1 车在左边
//   int h_sign = local_waypose.heading > 0 ? 1 : -1; // 1 向右
//    ROS_INFO("x_sign,h_sign:%d,%d", x_sign, h_sign);
//   int tmp_value = 0;
//   if (fabs(c_goal->heading - h) > 0.02 ||
//       fabs(c_goal->point.x - local_waypose.point.x) * 100 > 12) {
//     // h_sign ？向左打：向右打
//     if (h_sign == 1) {
//       if (x_sign == 1) {
//         // det_tv = 3; // 小值
//         tmp_value =0;
//       } else {
//         det_tv = -1;
//         tmp_value =last_tv + det_tv;
//       }

//     } else {
//       if (x_sign == 1) {
//         det_tv = 1;
//         tmp_value =last_tv + det_tv;
//       } else {
//         // det_tv = -3; // 小值
//         tmp_value =0;
//       }

//     }
//     t_value = tmp_value;

//     if (t_value > 3) {
//     t_value = 3;
//     } else if (t_value < -3) {
//         t_value = -3;
//     }
//   } else { // 250312 add
//         t_value = 0;
//   }


//   // 优先修正航向偏差 (纠正曲线时)
//   ROS_INFO("local_waypose.point.x,local_waypose.heading:%.3f,%.3f", local_waypose.point.x, local_waypose.heading);
// 
// 
//     ROS_INFO("CT99#dev_flag %d,last_tv %d,t_value = %d, det_tv = %d",dev_flag, last_tv,t_value,
//     det_tv);
//     return t_value;
// 
// }

// transtime()
std::string timestampToDate(const ros::Time &time) {
  time_t seconds = time.sec + 8 * 60 * 60; // hoursOffset 8
  struct tm *timeinfo = gmtime(&seconds);  // 使用gmtime获取UTC时间

  std::stringstream ss;
  ss << std::put_time(timeinfo, "%Y-%m-%d %H:%M:%S"); // 格式化输出年月日时分秒
  return ss.str();
}

// version
std::string getVersion() {
  std::stringstream strs;
  std::string ss;
  /*
      250305起，高速电机——开发log；3月12日，调整；27日 去除标定区;0411
     定位话题统一; 0430 avoid_data = 1处理
  */
  strs << VERSION << "_2025/05/06_1400";
  ss = strs.str();
  return ss;
}

double getDelay(const ros::Time &now, const ros::Time &last) {
  return fabs(last.toSec() - now.toSec());
}