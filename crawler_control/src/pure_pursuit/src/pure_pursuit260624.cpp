#include "geometry.h"
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

  float theta = car_position.yaw - M_PI_2;
  memset(local_waypoints, 0, sizeof(local_waypoints));
  float cosval = cosf(theta);
  float sinval = sinf(theta);

  for (i = 0; i < planning_result.point_num; i++) {
    p1.point.x = planning_result.x[i] - car_position.position_x;
    p1.point.y = planning_result.y[i] - car_position.position_y;
    p1.heading = robot::geometry::NormalizeAngle(
        planning_result.heading[i] - car_position.yaw); // 局部位置转换？？
    local_waypoints[i].point.x = p1.point.x * cosval + p1.point.y * sinval;
    local_waypoints[i].point.y = p1.point.x * (-sinval) + p1.point.y * cosval;
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
      0 /*&& local_waypoints[nearest_waypoint_id].point.y < 0.20*/) {
    *distance = local_waypoints[nearest_waypoint_id].point.x;
  }
  if (nearest_waypoint_id >= 1) {
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
      lat_error_temp = fabsf(local_waypoints[i].point.x) * 100;
    else
      lat_error_temp = fabsf(
          sqrtf(powf(local_waypoints[i].point.x * 100 - desire_ICR, 2.0f) +
                powf(local_waypoints[i].point.y * 100, 2.0f)) -
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

void PurePursuit::setControl_main(void) {
  int gear_count = 0;
  desire_speed_com = planning_result.speed[SPEED_TARGET_POINT] * 100.0f;
  int ld_init;
  // if(initial_approach_mode)
  // {     // cnk 0514 初始接近模式
  // ld_init=4;
  // }
  // else{
  ld_init = desire_speed_com < 25 ? ld_i + 2 : ld_i + 5;   //}
  if (nearest_waypoint_id != -1 && !need_turn_for_new_path) // cnk 0511
  {
    bool found = false; // 已经找到了合适预瞄点？
    look_ahead_index = planning_result.point_num - 1;
    // nearest_waypoint_id+1
    for (int i = 0; i < planning_result.point_num; i++) {
      // 优化转弯逻辑：add判断 y < 0.20 ; nearest_dis < 0.2
      if (turn_state) {
        gear_flag = 0;
        turn_state = planning_result.gear[SPEED_TARGET_POINT+1] == 2
                         ? 0
                         : turn_state; // 提前预警转弯
      } else {
        gear_flag = planning_result.gear[SPEED_TARGET_POINT] == 2 ? 1 : 0; //@
      }
      // turn_state = planning_result.point_num == 1 ? 0 : turn_state; //@
      // 最后一个点0只有路径规划上接2
      // 0109 add stop_tem_flag 转弯要求
      if (turn_flag || (gear_flag && local_waypoints[0].point.y <
                                         0.09)) { // TODO 0.09是否合适？cnk
        static ros::Time t1;                      // 定义为静态，防止时间值
        static float init_y = 0, last_y = 0;      // 第一个位置？和累计值？
        tem_flag = 1;
        if (turn_flag) {
          break;
        }
        if (!stop_tem_flag && tem_flag) {
          t1 = ros::Time::now();
          init_y = local_waypoints[0].point.y;
          tem_flag = 0;
        }
        stop_tem_flag = 1; //
        //
        // ROS_INFO("<-------- Turn: stop---------->");

        last_y = local_waypoints[0].point.y;
        auto t2 = ros::Time::now();
        auto det = getDelay(t2, t1); // 4帧的时间差
        auto add_y = fabs(last_y - init_y);
        // ROS_INFO("Ds4#det %.3f,t2 %.3f,t1 %.3f", det,t2.toSec(),t1.toSec());
        // //debug_s ROS_INFO("Dsdelat_Vy =  %.3f", add_y*100 / det);
        // //根据帧间速度判断 cm/s
        if (det >= 0.2) {
          bool addv_flag = add_y * 100 / det < 40 ? 1 : 0; // TODO cnk  20+5
          // ROS_WARN("speed:  %.4f", add_y);
          if (addv_flag) {
            turn_flag = 1;
            first_set = false;
            adjust_error = false;
            stop_tem_flag = 0;
            add_y = 0;
            break; // 跳出for循环
          } else {
            t1 = ros::Time::now();
            init_y = local_waypoints[0].point.y;
            stop_tem_flag = 1; // 速度过高停车      //
            // ROS_INFO("<-------- Speed: stop---------->");
            add_y = 0;
            break;
          }
        } else {
          stop_tem_flag = 1; // det<0.2 停车      //
          // ROS_INFO("<-------- distance: stop---------->");
          break; // 跳出for循环
        }
      }
      if (planning_result.gear[i] == 2) {
        gear_count++; // 记录次数 1
        if (turn_flag == 0 && gear_count == 1) {
          // 找到第一个 gear[i] == 2
          look_ahead_index = i;
          break; // 跳出循环
        } else if (turn_flag == 1 && gear_count == 2) {
          // 找到第二个 gear[i] == 2
          look_ahead_index = i;
          break; // 跳出循环
        }
      }
      //    turn_flag = 0;
    }
    ////////////// cnk 0514
    static ros::Time first_stop_time = ros::Time::now();
    if (stop_tem_flag == 1) {
      // 检查是否超过强制超时时间
      double total_stop_time = (ros::Time::now() - first_stop_time).toSec();

      if (total_stop_time > 0.5) { // 强制0.5秒后继续
        stop_tem_flag = 0;
        turn_flag = true;
        first_set = false;
        adjust_error = false;
        first_stop_time = ros::Time::now(); // 重置时间戳
      }
    } else {
      first_stop_time = ros::Time::now();
    }
    ////////////// cnk 0514
    look_ahead_index = look_ahead_index <= ld_init
                           ? look_ahead_index
                           : ld_init; // 0107 限制最大预瞄距离？
    // ROS_INFO("D44#ld_index %d,gear_flag %d,stop_tem %d,turn_flag %d,t_state
    // %d", look_ahead_index, gear_flag, stop_tem_flag, turn_flag, turn_state);
    // // debug_44
  }
  ROS_INFO("D44#desire_speed_com %f,gear_flag %d,stop_tem %d,turn_flag %d,turn_state %d", desire_speed_com, gear_flag, stop_tem_flag, turn_flag,
  turn_state);
  vir_wheel_track = 1.0 * wheel_track; // 标定 与滑动摩擦有关
  // pl0_heading = planning_result.heading[0];
  ctlE.e_theta = (local_waypoints[0].heading - 0) * 180 /
                 M_PI; // 与期望航向0的差值 单位：度
  // ROS_INFO("D4#point_num %d,V_des %.3f cm/s,ld_init: %d",
  // planning_result.point_num,desire_speed_com,ld_init);  //debug_4

  while (!stop_tem_flag) // 1
  {
    if (need_turn_for_new_path)
      break;

    //  自旋
    static float diff_v = 0;
    // 关键 turn_flag = 1 时 dv = 1500
    // command.drive_value =
    // calculateDriveValue(desire_speed_com,planning_result.speed[look_ahead_index-1],turn_flag,
    // avoid_state_data); look_ahead_index-1 会报段错误
    command.drive_value = calculateDriveValue(
        desire_speed_com, planning_result.speed[look_ahead_index], turn_flag,
        avoid_state_data);
    ctv = desire_speed_com < 30 ? 0 : 0; // 转弯中心值偏移？初始化1501 : 1501
    command.turn_value = ctv;
    if (desire_speed_com <= 30.0 && planning_result.point_num > 1) {
      // ？要转弯？？ delta_heading  符号 正 逆转；负 顺转 |～～ 档位逻辑未加
      float delta_heading = -local_waypoints[0].heading; //@
      // ROS_INFO("D05#near_wp_id %d,delta_heading %.4f,l[0] %.3f,near %.3f",
      // nearest_waypoint_id,delta_heading,
      //          local_waypoints[0].heading,local_waypoints[nearest_waypoint_id].heading);
      //          //debug_05
      // 自旋转向

      // ROS_INFO("start turn--------------------"); //debug_5
      if (turn_flag) {
        ROS_INFO("朝向起始点");
        double dx = planning_result.x[0] - car_position.position_x;
        double dy = planning_result.y[0] - car_position.position_y;
        double distance_to_start = hypot(dx, dy);
        double desired_heading = atan2(dy, dx);
        double heading_error =
            robot::geometry::NormalizeAngle(desired_heading - car_position.yaw);
            ROS_INFO("heading_error=%lf,distance_to_start=%lf",heading_error,distance_to_start);
        if (distance_to_start > 0.05 && !adjust_error) {
          if (fabsf(heading_error) > 0.05 && !first_set) {
            command.drive_value = 0; // 1500
            command.turn_value = (heading_error > 0) ? -10 : 10; // 1270 : 1730
            ROS_INFO("Adjusting heading to target point...");
          } else {
            first_set = true;
            if (distance_to_start > last_distance) {
              count++;
            } else {
              count = 0; // 距离变小，重置
            }

            if (count >= 3) {
              adjust_error = true;
              count = 0;
            }
            if(heading_error>0&&dx>0)
            {
              command.drive_value = 15; // 1600
              command.turn_value = 12; // 1500
              
            }else if(heading_error<0&&dx>0)
            {
              command.drive_value = 15; // 1600
              command.turn_value = 6; // 1500
              
            }else if(heading_error>0&&dx<0)
            {
              command.drive_value = 15; // 1600
              command.turn_value = -6; // 1500
              
            }else
            {
              command.drive_value = 15; // 1600
              command.turn_value = -12; // 1500
              
            }
            last_distance = distance_to_start;
          }
        } else {
          // 对齐完成
          adjust_error = true;
          count = 0;
          ROS_INFO("adjust completed.");
        }

        ROS_WARN("start turn, %.4f,%.4f", planning_result.x[0],
                 planning_result.y[0]);                // 0.1弧度=5.73度
        if (fabsf(local_waypoints[0].heading) <= 0.05) // 0.096
        {
          geometry_msgs::Point point_msg;
          point_msg.z = 999;
          if (avoid_state_data == 1)
            ROS_WARN("********************************");
          path_too_far_pub_.publish(point_msg);
          ROS_WARN("gear =2 should droped,turn finish, %.4f,%.4f",
                   planning_result.x[0], planning_result.y[0]);
          turn_state = 1;
          turn_flag = 0;
          // command.drive_value = 1730; // 1750
          break; // 跳出while循环
        }
        ROS_WARN("delta_heading,%.4f",
                  delta_heading);
        if (delta_heading < 0) {
          command.turn_value = -10; // 1270
          command.drive_value = 0; // 1500
          break;
        } else {
          command.turn_value =10; // 1730
          command.drive_value = 0; // 1500
          break;
        }
      }
    }
    
    if (!turn_flag) {
      if (findGoalPoint()) // 找到预瞄点
      {
        ROS_INFO("goal->heading,ctlE.e_pos,%.4f,%.4f",
                  goal->heading,ctlE.e_pos);
        if (fabsf(goal->heading) <= 0.02 &&
            fabsf(ctlE.e_pos) < 0.06) // 速度预瞄点在正前方
        {
          // 需要速度方向跟踪PID
          command.turn_value = ctv;
          desire_ICR = INFINITY;
        } else // 预瞄点不在正前方
        {
           ROS_INFO("what");
          desire_ICR = (pow(goal->point.x * 100, 2.0f) +
                        pow(goal->point.y * 100, 2.0f)) /
                       (2.0f * goal->point.x * 100); // 负值 为 左转 正值 右转
          command.turn_value = calculateTurnValue(
              gear_flag, local_waypoints[0], goal, goal_id, last_turn_value);
          break;
        }
      } else // 没有找到预瞄点停车
      {
        desire_ICR = INFINITY;
        path_state = PATH_FINISHED;
        ROS_INFO("with out target point");
        break;
      }

      if (isGoalValid()) // 当前转向能够保证预瞄点以内路径横向偏差
      {
        // 加入修正项，在横向误差大时使用近处预瞄点控制横向误差
        look_ahead_index = nearest_waypoint_id + ld_i; // for debug
        // ROS_INFO("nearest_waypoint_id  %d", nearest_waypoint_id);
        for (int i = 0; i < ld_i; i++) {
          if (planning_result.gear[i] != planning_result.gear[i + 1]) {
            look_ahead_index = i;
            break;
          } else {
            look_ahead_index = nearest_waypoint_id + ld_i;
          }
        }

        if (findGoalPoint()) // 更新 预瞄点goal, <= ld_i
        {
          if (fabsf(goal->heading) <= 0.02 &&
              fabsf(ctlE.e_pos) < 0.06) // 假定预瞄点在正前方
          {
            command.turn_value = ctv;
            desire_ICR = INFINITY;
          } else // 预瞄点不在正前方
          {
            float desire_ICR = (pow(goal->point.x * 100, 2.0f) +
                                pow(goal->point.y * 100, 2.0f)) /
                               (2.0f * goal->point.x * 100);
            command.turn_value = calculateTurnValue(
                gear_flag, local_waypoints[0], goal, goal_id, last_turn_value);
            break;
          }
        }
        break; // 跳出while循环
      } else if (
          fabs(goal->point.y < 0.9f) &&
          (goal_id <
           2)) // 当前转向不能够保证预瞄点以内路径横向偏差，预瞄点过近，就这样开吧
      {
        //  1230 判断及fabs
        // ROS_INFO("Continue driving %d,%f,%d,%d", (goal->point.y < 3.0f),
        // goal->point.y, (goal_id < 2), goal_id);
        break;
      } else { // 当前转向不能够保证预瞄点以内路径横向偏差，预瞄距离可以减小
        look_ahead_index--;
        // ROS_INFO("D8#look_ahead_index -- : %d", look_ahead_index);
      }

      // if (look_ahead_index <= 0)
      if (look_ahead_index <= 0) {
        command.turn_value = 0; // 1500
        command.drive_value = 0; // 1500
        desire_ICR = INFINITY;
        // ROS_INFO("look_ahead_index <= 0");
        break;
      }
    }
  }
  // add speed range
  int tv = command.turn_value;
  int dv = command.drive_value;
  if (command.turn_value >= -10 && command.turn_value <= 10) { // 2300
    command.turn_value = command.turn_value;
  } else {
    command.turn_value = ctv;
  }
  if (command.drive_value >= -60 && command.drive_value <= 60) { // 2300
    command.drive_value = command.drive_value;
  } else {
    command.drive_value = 60; // 1900
  }
   ROS_INFO("D9#cmd# t_v:%d,d_v:%d,~~tv %d,dv %d", command.turn_value,
   command.drive_value, tv, dv);

  // 最后 add c_finished
  if (avoid_state_data != 1 || planning_result.pathtype == 0) {
    if (!c_finish && path_state == PATH_FINISHED &&
        fabsf(goal->point.y) < 0.2f && fabsf(vehicle_info.vehicle_speed) > 2) {
      // ROS_INFO("path_state is PATH_FINISHED and goal->point.y = %.3f",
      // goal->point.y);
      std::string filePath =
          "/home/nvidia/Mower/src/location_map/ControlStatus.txt";
      std::ofstream outFile(filePath);
      if (!outFile.is_open()) {
        ROS_WARN("can't open ControlStatus.txt");
        // std::cerr << "can't open ControlStatus.txt" << std::endl;
      }
      float c_length =
          sqrt(pow(goal->point.x, 2.0f) + pow(goal->point.y, 2.0f)) * 100; // cm
      if (c_length < 10) {
        ROS_INFO("~~ control finished , end_length : %.3f cm", c_length);
        c_finish = true;
        path_state = PATH_INIT;
        lat_track_state = CONTROL_FINISHED;
        outFile << c_finish << std::endl;
        outFile.close();
      }
    } else {
      // ROS_INFO("path_state: %d,finish: %d,goal->point.y: %.3f,track_state:
      // %d", path_state, c_finish, goal->point.y,lat_track_state);
      // //-debug: add
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
  ROS_INFO("Tracking: goal_id=%d, look_ahead_index=%d, e_pos=%.4f, e_theta=%.4f",
             (int)goal_id, look_ahead_index, ctlE.e_pos, ctlE.e_theta);
  act();
}


bool already_moving_straight = false;
bool align_to_path_point = false;
double last_distance_to_start = std::numeric_limits<double>::max();
int distance_increase_count = 0;
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

  if (last_pathtype != planning_result.pathtype) {
    ROS_INFO_THROTTLE(2, "New path detected");
    need_turn_for_new_path = true;
    align_to_path_point = true; // 新路径到来，首先需要横向靠近路径起点，align_to_path_point：控制是否执行靠近路径起点的逻辑
  }


  if (need_turn_for_new_path) {
    if (align_to_path_point && planning_result.pathtype == 0) {
      double dx = planning_result.x[0] - car_position.position_x;
      double dy = planning_result.y[0] - car_position.position_y;
      double distance_to_start = hypot(dx, dy);
      double desired_heading = atan2(dy, dx);
      double heading_error =
          robot::geometry::NormalizeAngle(desired_heading - car_position.yaw);
       ROS_INFO("callbackFromPlanningResult");
      ROS_INFO("Align to path point: distance = %.2f, heading_error = %.3f",
               distance_to_start, heading_error);
      if (distance_to_start > 0.03) {
        if (fabsf(heading_error) > 0.03 && !already_moving_straight) {
          command.drive_value = 0; // 1500
          
          command.turn_value = (heading_error > 0) ? -10 : 10; // 1270 : 1730
          distance_increase_count = 0;
          ROS_INFO("Adjusting heading to target point...");
        } else {
          already_moving_straight = true;
          if (distance_to_start > last_distance_to_start) {
            distance_increase_count++;
          } else {
            distance_increase_count = 0; // 距离变小，重置
          }

          if (distance_increase_count >= 3) {
            align_to_path_point = false;
            distance_increase_count = 0;
          }
          command.drive_value = 25; // 1600
          command.turn_value = 0; // 1500
          last_distance_to_start = distance_to_start;
        }
      } else {
        // 调整对齐完成
        align_to_path_point = false;
        distance_increase_count = 0;
        ROS_INFO("Alignment completed.");
      }
    }
    // === 原有的角度调整逻辑 ===
    double heading = robot::geometry::NormalizeAngle(
        planning_result.heading[0] - car_position.yaw);
    if (fabsf(heading) <= 0.10) {
      geometry_msgs::Point point_msg;
      point_msg.z = 999;
      path_too_far_pub_.publish(point_msg);
      ROS_WARN("first gear =2 dropped");
      need_turn_for_new_path = false;
      ResetFlag(); //
    } else {
      command.drive_value = 0; // 1500
      ROS_INFO("heading=%.4f",heading);
      command.turn_value = (heading > 0) ? -10 : 10; // 1270 : 1730

    }
  }

  last_pathtype = planning_result.pathtype;
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

  // add flag to car_stop_status
  if (stop_car_flag || c_finish || (stop_tem_flag && !need_turn_for_new_path) ||
      stop_path_flag) // cnk 0512
  {
    mower_msgs::VehicleCmd quit_cmd;
    quit_cmd.ad_control_enable = 1; // 要不要开
    quit_cmd.mower_height = cmd_msg.mower_height;
    quit_cmd.gear_model = cmd_msg.gear_model;
    quit_cmd.mover_bool = cmd_msg.mover_bool;
    last_turn_value = (last_turn_value < 30 && last_turn_value > -30)
                          ? last_turn_value
                          : 0;
    quit_cmd.turn_value = last_turn_value;
    quit_cmd.drive_value = 0;
    if (c_finish) {
      quit_cmd.ad_control_enable = 0;
      quit_cmd.gear_model = cmd_msg.p_Gear;
      quit_cmd.mover_bool = mover_bool_cfg;
      ROS_INFO("ALL~~~~~~~~~Finished");
    }

    // ROS_INFO("stop car~~~~~,stop_tem %d,stop_car_flag %d,stop_path %d",
    // stop_tem_flag, stop_car_flag, stop_path_flag); ROS_INFO("D10$stop pub $
    // t_v %d,d_v %d,gear %d,ad %d", quit_cmd.turn_value,
    // quit_cmd.drive_value,quit_cmd.gear_model,quit_cmd.ad_control_enable);
    quit_cmd.header.stamp = ros::Time::now(); // 设置时间戳
    pub_Command.publish(quit_cmd);
    // 保障停车
    static int stop_num = 0;
    stop_num++;

    if (stop_num > 10 && stop_path_flag) // TODO @cnk
    {
      stop_num = stop_num > 15 ? 0 : stop_num;
      if (!planning_result.x.empty()) {
        geometry_msgs::Point point_msg;
        point_msg.x = planning_result.x[0];
        point_msg.y = planning_result.y[0];
        path_too_far_pub_.publish(point_msg);
        // ROS_INFO("Published path_too_far_point: x = %.2f, y = %.2f",
        // point_msg.x, point_msg.y);
      } else {
        ROS_WARN(
            "Planning result is empty. Cannot publish path_too_far_point.");
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
    // ROS_INFO("mower_height = %d",cmd_msg.mower_height);
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
  pub_CtlError =
      nh_.advertise<mower_msgs::ControlError>("/mower/control_error", 1);
  path_too_far_pub_ =
      nh_.advertise<geometry_msgs::Point>("/path_too_far_point", 1); //@cnk
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
    return;
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
  static int th_dv = 0; // 1500
  // int th_v = (int)(4.0989*current_speed + 1660); //系数kv
  //最后add 0405
  if (avoid_state_data == 3) {
    th_dv = 0; // 1500
  } else if (avoid_state_data == 2) {
    th_dv =30; // 1730
  } else {
    if (current_speed <= 40) {
      th_dv = 40; // 最终 1730
    }
    //  else if (current_speed < 35) {
    //   th_dv = 30; // 1800
    // } 
    else {
      th_dv = 60; // 1900
    }
  }

  if (flag) {
    return 0; // 1500
  } else {
    return th_dv;
  }
}

// 非标定法 计算转弯值
int calculateTurnValue(bool latest_gear_flag, waypose local_waypose,
                       waypose *c_goal, unsigned char cgoal_id, int last_tv) {
  // 1判断上一转值？ 垂直 直线 曲线
  static bool dev_flag = 0; // 跟踪偏差
  static bool sl_flag = 0;  // 直线
  static bool cu_flag = 0;  // 曲线
  static int t_value = ctv;
  static float h = 0;
  int det_tv = 0;

  int x_sign = local_waypose.point.x > 0 ? 1 : -1; // 1 车在左边
  int h_sign = local_waypose.heading > 0 ? 1 : -1; // 1 向右
   ROS_INFO("x_sign,h_sign:%d,%d", x_sign, h_sign);
  int tmp_value = 0;
  if (fabs(c_goal->heading - h) > 0.02 ||
      fabs(c_goal->point.x - local_waypose.point.x) * 100 > 12) {
    // h_sign ？向左打：向右打
    if (h_sign == 1) {
      if (x_sign == 1) {
        // det_tv = 3; // 小值
        tmp_value =0;
      } else {
        det_tv = -1;
        tmp_value =last_tv + det_tv;
      }

    } else {
      if (x_sign == 1) {
        det_tv = 1;
        tmp_value =last_tv + det_tv;
      } else {
        // det_tv = -3; // 小值
        tmp_value =0;
      }

    }
    t_value = tmp_value;

    if (t_value > 3) {
    t_value = 3;
    } else if (t_value < -3) {
        t_value = -3;
    }
  } else { // 250312 add
        t_value = 0;
  }


  // 优先修正航向偏差 (纠正曲线时)
  ROS_INFO("local_waypose.point.x,local_waypose.heading:%.3f,%.3f", local_waypose.point.x, local_waypose.heading);


    ROS_INFO("CT99#dev_flag %d,last_tv %d,t_value = %d, det_tv = %d",dev_flag, last_tv,t_value,
    det_tv);
    return t_value;

}

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
/* 单元二次函数yq = a * xq^2 + b * xq + c
int quadraticFunction(double a, double b, double c, double x){
    double y = a * x * x + b * x + c;
    return static_cast<int>(y);
}*/

double getDelay(const ros::Time &now, const ros::Time &last) {
  return fabs(last.toSec() - now.toSec());
}