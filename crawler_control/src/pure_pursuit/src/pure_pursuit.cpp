#include "geometry.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <pure_pursuit/pure_pursuit.h>
#include <std_msgs/Bool.h>
#include <std_msgs/String.h>
#include <std_msgs/UInt8.h>
#include <yaml-cpp/yaml.h>

// 相邻路径点间距 [m]
constexpr float kPointSpacing = 0.25f;

// 构造函数
PurePursuit::PurePursuit()
    : private_nh_("~"), LOOP_RATE_(20), // 50
      is_planning_result_set_(false), is_carposition_set_(false),
      is_velocity_set_(false)
{
  last_pos_time = ros::Time::now();
  last_planning_result_time = ros::Time::now();
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
    p1.heading = robot::geometry::NormalizeAngle(planning_result.heading[i] - car_position.yaw); // 航向差保持不变
    // 世界坐标 -> ROS 车辆坐标：x 为前进方向，y 为左侧
    local_waypoints[i].point.x = p1.point.x * cosval + p1.point.y * sinval;
    local_waypoints[i].point.y = -p1.point.x * sinval + p1.point.y * cosval;
    local_waypoints[i].heading = p1.heading;
  }
}

void PurePursuit::updatestatus(void) {
  stop_path_flag = updatePathState();
}

int PurePursuit::updatePathState(void) {
  float distance;
  if (planning_result.point_num == 0)
    return 1;

  if (!getNearestIndex(&nearest_waypoint_id)) {
    return 1;
  }
  if (getLatDistance(&distance)) {
    if (fabsf(distance) > kMaxPathDistance) {
      return 1;
    } else {
      return 0;
    }
  }
  return 0;
}

int PurePursuit::getNearestIndex(int *index) {
  int nearest_index = -1;
  int i;
  float nearest_dis = 999.0;
  *index = -1;

  if (planning_result.point_num < 1) {  
    return 0;
  }

  // 第一轮：heading 一致的前方点
  for (int i = 0; i < planning_result.point_num; i++) {
      if (i==0&&planning_result.gear[i] == 2) 
      {
        float dx_comp = planning_result.x[i] - last_completed_turn_x_;
        float dy_comp = planning_result.y[i] - last_completed_turn_y_;
        if(std::hypot(dx_comp, dy_comp)>0.01f)
        {
          nearest_index = i;
          *index = nearest_index;
          return 1;// gear=2 转弯点就是最近点
        }else{
          *index = nearest_index+1;
          return 1;// gear=2 转弯点就是最近点
        }
      }
      if (fabsf(local_waypoints[i].heading) >= M_PI / 2) continue;
      

      float dis = robot::geometry::hypotFast(local_waypoints[i].point.x * 100,local_waypoints[i].point.y * 100);
      if (dis > 300.0f) continue;

      if (dis < nearest_dis) {
          nearest_dis = dis;
          nearest_index = i;
      }
  }

  if (nearest_index == -1) {
    for (int i = 0; i < planning_result.point_num; i++) {
        // if (local_waypoints[i].point.x <= 0.0f) continue;
        if(planning_result.gear[i] == 2) {
          float dx_comp = planning_result.x[i] - last_completed_turn_x_;
          float dy_comp = planning_result.y[i] - last_completed_turn_y_;
          if(std::hypot(dx_comp, dy_comp)>0.01f)
          {
            nearest_index = i;
            *index = nearest_index;
            return 1;// gear=2 转弯点就是最近点
          }else{
            *index = nearest_index+1;
            return 1;// gear=2 转弯点就是最近点
          }
        }
        float dis = robot::geometry::hypotFast(
            local_waypoints[i].point.x * 100,
            local_waypoints[i].point.y * 100);
        if (dis > 300.0f) continue;

        if (dis < nearest_dis) {
            nearest_dis = dis;
            nearest_index = i;
        }
    }
  }

  if (nearest_index == -1) // not found
  {
    ROS_ERROR_THROTTLE(1, "ERROR ~not found nearest_waypoint~ : %d ", nearest_index);
    return 0;
  }

  if (nearest_dis > kMaxPathDistance) {
    ROS_INFO("PATH_TOO_FAR ~dis~BBBBB  %.3f", nearest_dis); //
    return 0;
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

int PurePursuit::calculateLookAheadIndex(int base_idx, float speed_m_s) {
    const float weight_l = 2.0f;
    const float kMaxLookAheadDistM = 4.0f;
    const float kMinLookAheadDistM = 1.0f;

    float lookahead_dist_m = weight_l * speed_m_s + 0.3f;
    if (lookahead_dist_m > kMaxLookAheadDistM) {
        lookahead_dist_m = kMaxLookAheadDistM;
    } else if (lookahead_dist_m < kMinLookAheadDistM) {
        lookahead_dist_m = kMinLookAheadDistM;
    }

    int target_idx = base_idx + static_cast<int>(std::round(lookahead_dist_m / kPointSpacing));
    if (target_idx >= planning_result.point_num) {
        target_idx = planning_result.point_num - 1;
    }
    if (target_idx < 0) {
        target_idx = 0;
    }

    return target_idx;
}

int PurePursuit::findNextTurnPoint(int base_idx, int search_end_idx) {
    for (int i = base_idx; i <= search_end_idx; ++i) {
        if (planning_result.gear[i] == 2) {
            return i;
        }
    }
    return -1;
}
void PurePursuit::updateStateMachine() {
  ROS_INFO("[setControl_main] start: nearest=%d, point_num=%d, turn_phase=%d",
           nearest_waypoint_id, planning_result.point_num,
           static_cast<int>(vehicle_state_));

  // ========== 1. 路径有效性检查 ==========
  if (stop_path_flag || stop_car_flag ||avoid_state_data == 3) {
    vehicle_state_ = VehicleState::STOP;
    look_ahead_index = 0;
    return;
  }

  if (avoid_state_data == 4) {
    vehicle_state_ = VehicleState::REVERSE;
    look_ahead_index = 0;
    return;
  }

  int control_base_idx = nearest_waypoint_id;
  nearest_speed_m_s = planning_result.speed[nearest_waypoint_id]+0.1f;
  command.drive_value =calculateDriveValue(nearest_speed_m_s, 0.0f, false, avoid_state_data);

  // 计算预瞄索引
  look_ahead_index = calculateLookAheadIndex(control_base_idx, nearest_speed_m_s);

  // 搜索前方转弯点
  int gear2_idx = findNextTurnPoint(control_base_idx, look_ahead_index);

  // 一旦处于转弯状态机，强制预瞄到 latch 的转弯点
  if (gear2_idx!= -1) 
  {
    look_ahead_index = gear2_idx;
    if (vehicle_state_ == VehicleState::TRACKING&&fabsf(local_waypoints[look_ahead_index].point.x) < 0.05f&&fabsf(local_waypoints[look_ahead_index].point.y) < 0.05f) {
      vehicle_state_ = VehicleState::EXECUTING;
    }
  }else{
    // 如果 gear=2 点已经被规划器删除，或者车辆已经离开转弯点，则退出转弯状态机
    vehicle_state_ = VehicleState::TRACKING;
  } 
}
 /*
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
  const float omega_weight = 0.8f;
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
  switch (vehicle_state_)
  {
    case VehicleState::STOP:
      ROS_INFO("Vehicle is STOP, waiting for command.");
      command.turn_value = 0;
      command.drive_value = 0;
      break;

    case VehicleState::REVERSE:
      ROS_INFO("Vehicle is REVERSE, waiting for command.");
      command.turn_value = -15;
      command.drive_value = 0;
      break;

    case VehicleState::TRACKING:
      ROS_INFO("Vehicle is TRACKING, tracking.");
      // ========== 6. 正常 Pure Pursuit 跟踪 ==========
      // 使用 Pure Pursuit 计算角速度（rad/s）
      // 单片机端对转速乘以 π 并放大 100 倍，因此 omega -> turn_value 需乘 100/π
      float omega = computePurePursuitOmega(&local_waypoints[look_ahead_index], nearest_speed_m_s);

      command.turn_value = static_cast<int>(omega * 100.0f / M_PI);

      // ========== 7. 输出限幅 ==========
      // turn_value 限制在 [-20, 20]，对应角速度 [-0.2*π, 0.2*π] rad/s
      // drive_value 限制在 [0, 150]，需根据车辆最大速度调整
      command.turn_value = std::max(-20, std::min(20, command.turn_value));
      command.drive_value = std::max(0, std::min(150, command.drive_value));

      ROS_INFO("[setControl_main] final cmd: drive=%d (cm/s), turn=%d, ld_idx=%d, nearest=%d,  goal(x,y)=(%.3f,%.3f), omega=%.3f rad/s",
              command.drive_value, command.turn_value,
              look_ahead_index, nearest_waypoint_id, 
              planning_result.x[look_ahead_index], planning_result.y[look_ahead_index], omega);
      break;

    case VehicleState::EXECUTING:
      std::lock_guard<std::mutex> lock(turn_completed_mutex_);
      command.drive_value = 0;  // 停车转弯

      // heading_error：车辆航向与转弯后下一段路径方向的夹角
      // 由于 gear=2 点自身的 heading 可能是入弯方向，不能直接用，
      // 所以几何计算从转弯点指向下一个点的方向作为出弯目标。
      float heading_error = 0.0f;
      int next_idx = look_ahead_index + 1;
      if (next_idx < planning_result.point_num) {
        float dx = local_waypoints[next_idx].point.x - local_waypoints[look_ahead_index].point.x;
        float dy = local_waypoints[next_idx].point.y - local_waypoints[look_ahead_index].point.y;
        heading_error = atan2f(dy, dx);
      } else if (look_ahead_index < planning_result.point_num) {
        heading_error = local_waypoints[look_ahead_index].heading;
      }
      ROS_INFO("[setControl_main] turn executing: heading_error=%.3f rad", heading_error);

      turn_completed_msg_.request.x = planning_result.x[look_ahead_index];
      turn_completed_msg_.request.y = planning_result.y[look_ahead_index];
      turn_completed_msg_.response.turn_finish = false;

      if (std::fabs(heading_error) <= 0.10f) {
        ROS_INFO("[setControl_main] turn completed: heading_error=%.3f rad, publish /turn_completed",
                heading_error);
        // 转弯完成，发布 /turn_completed 信号通知上层 planner 丢弃该 gear=2 点
        turn_completed_msg_.response.turn_finish = true;
        last_completed_turn_x_ = planning_result.x[look_ahead_index];
        last_completed_turn_y_ = planning_result.y[look_ahead_index];
        
        // 恢复普通跟踪状态
        vehicle_state_ = VehicleState::TRACKING;
        look_ahead_index = 0;
      } else {
        // heading_error > 0：路径方向在车辆左侧，需要左转（负值）
        // heading_error < 0：路径方向在车辆右侧，需要右转（正值）
        command.turn_value = (heading_error > 0.0f) ? -10 : 10;
        turn_completed_msg_.response.turn_finish = false;
        return;
      }
      break;

    default:
      break;
  }
 }

void PurePursuit::latcontrol_algorithm(void) {
  setPath_main();
  updatestatus();
  updateStateMachine();
  setControl_main();
}

void PurePursuit::callbackFromPlanningResult(const util::LocalPathConstPtr &msg_planning_result) {
  planning_result = *msg_planning_result;
  last_planning_result_time = ros::Time::now();
}

void PurePursuit::callbackFromSpeedInfo(const util::LocalPoseConstPtr &msg_speed_info) {
  vehicle_info = *msg_speed_info;
  is_velocity_set_ = true;
}

void PurePursuit::callbackFromPos(const util::PositionConstPtr &msg_carposition) {
  car_position = *msg_carposition;
  last_pos_time = ros::Time::now();
}

void PurePursuit::callbackFromDirectControl(const mower_msgs::Direct_ControlConstPtr &msg_direct_control) {
  (void)msg_direct_control;
  ROS_INFO("direct_control");
}

/* task-stop */
void PurePursuit::callbackFromTaskstop(const std_msgs::Bool &task_msg) {
  Task_stop = task_msg.data;
}
/* Avoidstate */
void PurePursuit::callbackFromAvoidstate(const std_msgs::UInt8 &avoid_msg) {
  avoid_state_data = avoid_msg.data;
}
void PurePursuit::callbackFromSignal(const std_msgs::StringConstPtr &string) {
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
  cmd_msg.header.stamp = ros::Time::now();
  if(cmd_msg.drive_value < 0.0f)
  {
    cmd_msg.mower_height = 0;
    cmd_msg.mover_bool = 0;
    cmd_msg.gear_model = cmd_msg.R_Gear;
  }else if(cmd_msg.drive_value==0.0f&&cmd_msg.turn_value==0) 
  {
    cmd_msg.mower_height = 0;
    cmd_msg.mover_bool = 0;
    cmd_msg.gear_model = cmd_msg.P_Gear;
  }else if (cmd_msg.drive_value > 0.0f)
  {
    cmd_msg.mower_height = mower_height_cfg;
    cmd_msg.mover_bool = mover_bool_cfg;
    cmd_msg.gear_model = cmd_msg.D_Gear;
  }else if(cmd_msg.drive_value==0.0f&&cmd_msg.turn_value!=0)
  {
    cmd_msg.mower_height = mower_height_cfg;
    cmd_msg.mover_bool = mover_bool_cfg;
    cmd_msg.gear_model = cmd_msg.N_Gear;
  }
  pub_Command.publish(cmd_msg);
}

void PurePursuit::initForROS() {

  // setup subscriber
  sub1_ = nh_.subscribe("/lawn_mower/global_path", 1,
                        &PurePursuit::callbackFromPlanningResult, this);
  sub2_ = nh_.subscribe("/nanobot/localpose", 1,
                        &PurePursuit::callbackFromSpeedInfo, this);
  sub4_ = nh_.subscribe("/Mower/position", 5, &PurePursuit::callbackFromPos, this);
  sub5_ = nh_.subscribe("/mower/direct_control", 1,
                        &PurePursuit::callbackFromDirectControl, this);
  substop = nh_.subscribe("/mower/stop_car", 1,
                          &PurePursuit::callbackFromTaskstop, this);
  sub_avoid = nh_.subscribe("/lawn_mower/avoid_state", 1,
                            &PurePursuit::callbackFromAvoidstate, this);
  sub_signal_ = nh_.subscribe("/signal", 1, 
                            &PurePursuit::callbackFromSignal, this);

  // setup publisher
  pub_Command = nh_.advertise<mower_msgs::VehicleCmd>("/vehicle/cmd", 1);
  pub_Controlok = nh_.advertise<mower_msgs::ControlOk>("/mower/control_ok", 1);
  turn_completed_srv_ = nh_.advertiseService("/turn_completed", &PurePursuit::callbackTurnCompleted, this);
}

void PurePursuit::run() {
  ROS_INFO_STREAM("control node is ready!");
  // 读取 YAML 配置文件
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
    if ((ros::Time::now() - last_planning_result_time).toSec() > 1.0) {
        is_planning_result_set_ = false;
    }else{
      if (planning_result.point_num < 1) {
        is_planning_result_set_ = false;
        ROS_ERROR_THROTTLE(1, "~ point_num is wrong : %d ",planning_result.point_num);
      }else{
        is_planning_result_set_ = true;
      }
    }
    if(!is_carposition_set_||!is_planning_result_set_||Task_stop||!is_velocity_set_){
      Control_Ok_msg.is_control_ok = false;
      stop_car_flag =true;
    }else{
      Control_Ok_msg.is_control_ok = true;
      stop_car_flag =false;
    }

   
    /*** 信号处理，正常控制 ***/
    // //驾驶模式时修改

      latcontrol_algorithm();
      command_publish();

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
