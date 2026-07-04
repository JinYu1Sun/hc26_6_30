#include <ros/ros.h>
#include <std_msgs/Bool.h>
#include <std_msgs/UInt8.h>
#include <util/Position.h>
#include <util/LocalPath.h>
#include <util/VehicleControl.h>
#include <util/DiagnosticResult.h>
#include <xmlrpcpp/XmlRpcValue.h>
#include <deque>
#include <cmath>  // 为M_PI添加数学库

class VehicleDiagnostic
{
public:
    VehicleDiagnostic(ros::NodeHandle& nh) : nh_(nh)
    {
        // Initialize status
        has_position_ = false;
        stop_car_command_ = false;
        avoid_status_ = 0;
        has_global_path_ = false;
        has_vehicle_cmd_ = false;
        can_buffer_full_ = false;
        
        vehicle_speed_ = 0.0;
        last_cmd_time_ = ros::Time::now();
        last_position_time_ = ros::Time(0);
        last_path_time_ = ros::Time(0);
        last_vehicle_cmd_time_ = ros::Time(0);
        
        // Position tracking for stop detection - using sliding window approach
        position_history_.clear();
        position_frames_for_stop_detection_ = 5;  // 检测停止需要的帧数
        
        // Subscribe to relevant topics
        position_sub_ = nh_.subscribe("/Mower/position", 1, &VehicleDiagnostic::positionCallback, this);
        stop_car_sub_ = nh_.subscribe("/mower/stop_car", 1, &VehicleDiagnostic::stopCarCallback, this);
        avoid_status_sub_ = nh_.subscribe("/lawn_mower/avoid_state", 1, &VehicleDiagnostic::avoidStatusCallback, this);
        global_path_sub_ = nh_.subscribe("/lawn_mower/global_path", 1, &VehicleDiagnostic::globalPathCallback, this);
        //vehicle_cmd_sub_ = nh_.subscribe("/vehicle/cmd", 1, &VehicleDiagnostic::vehicleCmdCallback, this);
        
        // Publish diagnostic results
        diagnostic_pub_ = nh_.advertise<util::DiagnosticResult>("/vehicle/diagnostic", 1);
        
        // Timer for periodic vehicle status check
        diagnostic_timer_ = nh_.createTimer(ros::Duration(1.0), &VehicleDiagnostic::diagnosticTimerCallback, this);
        
        // Parameter configuration
        nh_.param("timeout_position", timeout_position_, 2.0);      // Position data timeout
        nh_.param("timeout_path", timeout_path_, 2.0);              // Path data timeout
        nh_.param("timeout_vehicle_cmd", timeout_vehicle_cmd_, 3.0); // Vehicle command timeout
        nh_.param("min_speed_threshold", min_speed_threshold_, 0.1); // Minimum speed threshold
        nh_.param("position_frames_for_stop_detection", position_frames_for_stop_detection_, 6); // 检测稳定性需要的帧数
        nh_.param("position_movement_threshold", position_movement_threshold_, 0.08);   // 单帧间位置变化阈值(米)
        nh_.param("yaw_movement_threshold", yaw_movement_threshold_, 0.1);   // 单帧间航向角变化阈值(弧度)
        nh_.param("first_to_last_distance_threshold", first_to_last_distance_threshold_, 0.1);   // 第一帧到最后一帧距离阈值(米)
        
        ROS_INFO("Vehicle Diagnostic Node Started");
    }

private:
    ros::NodeHandle nh_;
    
    // Subscribers
    ros::Subscriber position_sub_;
    ros::Subscriber stop_car_sub_;
    ros::Subscriber avoid_status_sub_;
    ros::Subscriber global_path_sub_;
    ros::Subscriber vehicle_cmd_sub_;
    
    // Publishers
    ros::Publisher diagnostic_pub_;
    
    // Timer
    ros::Timer diagnostic_timer_;
    
    // Status variables
    bool has_position_;
    bool stop_car_command_;
    uint8_t avoid_status_;
    bool has_global_path_;
    bool has_vehicle_cmd_;
    bool can_buffer_full_;
    
    float vehicle_speed_;
    ros::Time last_cmd_time_;
    ros::Time last_position_time_;
    ros::Time last_path_time_;
    ros::Time last_vehicle_cmd_time_;
    
    // Position tracking for stop detection - using sliding window approach
    struct PositionPoint {
        double x;
        double y;
        double yaw;  // 添加航向角
        ros::Time timestamp;
    };
    std::deque<PositionPoint> position_history_;
    int position_frames_for_stop_detection_;
    double position_movement_threshold_;
    double yaw_movement_threshold_;  // 航向角变化阈值
    double first_to_last_distance_threshold_; // 第一帧到最后一帧距离阈值
    
    // Parameters
    double timeout_position_;
    double timeout_path_;
    double timeout_vehicle_cmd_;
    double min_speed_threshold_;
    
    // Check if position data is valid
    bool isPositionDataValid(const util::Position::ConstPtr& msg)
    {
        if (!msg) {
            return false;
        }
        
        // Check if position data is reasonable (not NaN or infinite)
        if (!std::isfinite(msg->position_x) || !std::isfinite(msg->position_y) || 
            !std::isfinite(msg->position_z) || !std::isfinite(msg->yaw)) {
            return false;
        }
        
        // Check if position is within reasonable bounds (adjust as needed)
        const double MAX_POSITION = 10000.0;  // 10km
        const double MIN_POSITION = -10000.0; // -10km
        
        if (msg->position_x < MIN_POSITION || msg->position_x > MAX_POSITION ||
            msg->position_y < MIN_POSITION || msg->position_y > MAX_POSITION) {
            return false;
        }
        
        // Check position state if available
        if (msg->position_state == 0) {  // Assuming 0 means invalid state
            return false;
        }
        
        return true;
    }
    
    // Check if node is running
    bool isNodeRunning(const std::string& node_name)
    {
        try {
            // Use rosnode command to check if node is running
            std::string command = "rosnode list | grep -q " + node_name;
            int result = system(command.c_str());
            return (result == 0);  // If command returns 0, node found
        } catch (...) {
            ROS_WARN("Failed to check if node %s is running", node_name.c_str());
            return false;
        }
    }
    
    // 计算第一帧与最后一帧之间的直线距离
    double calculateFirstToLastDistance()
    {
        if (position_history_.size() < 2) {
            return 0.0;  // 需要至少2个点才能计算距离
        }
        
        double dx = position_history_.back().x - position_history_.front().x;
        double dy = position_history_.back().y - position_history_.front().y;
        return sqrt(dx * dx + dy * dy);
    }
    
    // 计算两个角度之间的最小差值（考虑2π周期性）
    double angleDistance(double angle1, double angle2) 
    {
        double diff = std::abs(angle1 - angle2);
        // 处理角度的周期性：-π到π或0到2π
        if (diff > M_PI) {
            diff = 2 * M_PI - diff;
        }
        return diff;
    }
    
    // 综合稳定性检测：结合逐帧比较、第一帧到最后一帧距离和航向角
    bool isVehicleStable(double pos_threshold, double yaw_threshold, double first_to_last_threshold, 
                        bool& frame_stable, bool& distance_stable, bool& yaw_stable)
    {
        frame_stable = false;
        distance_stable = false;
        yaw_stable = false;
        
        if (position_history_.size() < position_frames_for_stop_detection_) {
            return false; // 数据不足，无法判断稳定
        }

        // 方法1：逐帧比较检测（参考A*方法）
        frame_stable = true;
        for (size_t i = 1; i < position_history_.size(); ++i) {
            double dx = std::abs(position_history_[i].x - position_history_[i-1].x);
            double dy = std::abs(position_history_[i].y - position_history_[i-1].y);
            double dyaw = angleDistance(position_history_[i].yaw, position_history_[i-1].yaw);

            // 判断是否超过阈值
            if (dx > pos_threshold || dy > pos_threshold || dyaw > yaw_threshold) {
                frame_stable = false;
                break;
            }
        }

        // 方法2：第一帧到最后一帧的直线距离检测
        double first_to_last_distance = calculateFirstToLastDistance();
        distance_stable = (first_to_last_distance <= first_to_last_threshold);
        
        // 方法3：第一帧到最后一帧的航向角变化检测
        double first_to_last_yaw_change = angleDistance(position_history_.back().yaw, position_history_.front().yaw);
        yaw_stable = (first_to_last_yaw_change <= yaw_threshold * 2);  // 允许航向角有一定变化

        // 综合判断：所有方法都认为稳定才算稳定
        bool is_stable = frame_stable && distance_stable && yaw_stable;
        
        return is_stable;
    }
    
    // Check if vehicle position is stable (stopped) using frame-by-frame comparison
    bool isPositionStable(double threshold)
    {
        if (position_history_.size() < position_frames_for_stop_detection_) {
            return false; // 数据不足，无法判断稳定
        }

        // 检查每帧之间的变化是否都在阈值内
        for (size_t i = 1; i < position_history_.size(); ++i) {
            double dx = std::abs(position_history_[i].x - position_history_[i-1].x);
            double dy = std::abs(position_history_[i].y - position_history_[i-1].y);

            // 判断是否超过阈值
            if (dx > threshold || dy > threshold) {
                return false;
            }
        }
        return true;
    }
    
    // Calculate total movement distance over the position history window (keep for debugging)
    double calculateTotalMovement()
    {
        if (position_history_.size() < 2) {
            return 0.0;  // Need at least 2 points to calculate movement
        }
        
        double total_movement = 0.0;
        for (size_t i = 1; i < position_history_.size(); ++i) {
            double dx = position_history_[i].x - position_history_[i-1].x;
            double dy = position_history_[i].y - position_history_[i-1].y;
            double distance = sqrt(dx * dx + dy * dy);
            total_movement += distance;
        }
        
        return total_movement;
    }
    
    void positionCallback(const util::Position::ConstPtr& msg)
    {
        // Check if position data is valid
        if (!isPositionDataValid(msg)) {
            ROS_WARN("Received invalid position data - position_x: %.2f, position_y: %.2f, position_state: %u", 
                     msg->position_x, msg->position_y, msg->position_state);
            has_position_ = false;
            return;
        }
        
        has_position_ = true;
        last_position_time_ = ros::Time::now();
        
        // Add new position to history
        PositionPoint new_point;
        new_point.x = msg->position_x;
        new_point.y = msg->position_y;
        new_point.yaw = msg->yaw; // 添加航向角
        new_point.timestamp = ros::Time::now();
        position_history_.push_back(new_point);
        
        // Remove old positions from history if it exceeds the window size
        while (position_history_.size() > position_frames_for_stop_detection_) {
            position_history_.pop_front();
        }
    }
    
    void stopCarCallback(const std_msgs::Bool::ConstPtr& msg)
    {
        stop_car_command_ = msg->data;
    }
    
    void avoidStatusCallback(const std_msgs::UInt8::ConstPtr& msg)
    {
        avoid_status_ = msg->data;
    }
    
    void globalPathCallback(const util::LocalPath::ConstPtr& msg)
    {
        has_global_path_ = (msg->point_num > 1);
        last_path_time_ = ros::Time::now();
    }
    
    // void vehicleCmdCallback(const util::VehicleControl::ConstPtr& msg)
    // {
    //     // Only used for recording speed and last control time, not for judging control node status
    //     vehicle_speed_ = sqrt(msg->drive_value * msg->drive_value + msg->turn_value * msg->turn_value);
    //     last_vehicle_cmd_time_ = ros::Time::now();
    //     last_cmd_time_ = ros::Time::now();
    // }
    
    void diagnosticTimerCallback(const ros::TimerEvent& event)
    {
        ros::Time now = ros::Time::now();
        
        // Check data timeout - this is the main way to detect if position data is not being received
        bool position_timeout = (now - last_position_time_).toSec() > timeout_position_;
        bool path_timeout = (now - last_path_time_).toSec() > timeout_path_;
        
        if (position_timeout) {
            has_position_ = false;
            ROS_WARN("Position data timeout - no valid position data received for %.2f seconds", timeout_position_);
        }
        if (path_timeout) has_global_path_ = false;
        
        // Check if vehicle is stopped using comprehensive stability check
        bool vehicle_stopped = false;
        bool frame_stable, distance_stable, yaw_stable;
        
        if (!has_position_) {
            // 如果没有位置数据，认为车辆停止（因为无法确定是否在移动）
            vehicle_stopped = true;
            ROS_WARN("No position data available - assuming vehicle is stopped");
        } else {
            // 使用综合稳定性检测方法：结合逐帧比较、累积距离和航向角
            vehicle_stopped = isVehicleStable(position_movement_threshold_, 
                                            yaw_movement_threshold_, 
                                            first_to_last_distance_threshold_,
                                            frame_stable, distance_stable, yaw_stable);
            
            // 调试信息：显示详细的稳定性分析结果
            if (position_history_.size() >= position_frames_for_stop_detection_) {
                ROS_INFO("Stability check: frame_stable=%s, distance_stable=%s, yaw_stable=%s, result=%s", 
                         frame_stable ? "true" : "false",
                         distance_stable ? "true" : "false", 
                         yaw_stable ? "true" : "false",
                         vehicle_stopped ? "STOPPED" : "MOVING");
            }
        }
        
        util::DiagnosticResult diagnostic_msg;
        diagnostic_msg.header.stamp = now;
        diagnostic_msg.header.frame_id = "base_link";
        
        // 每次诊断时都检查节点状态，确保状态信息是最新的
        // has_vehicle_cmd_ = isNodeRunning("pid_controller");
        has_vehicle_cmd_ = isNodeRunning("pure_pursuit");
        
        // 实时检查路径状态，确保has_global_path_是最新的
        if ((now - last_path_time_).toSec() > timeout_path_) {
            has_global_path_ = false;
        }
        
        uint8_t fault_code = 0;  // Default to normal operation
        
        // 首先检查位置数据是否有效
        if (!has_position_) {
            fault_code = 1;  // Position data not available or invalid
        }
        // 有位置数据且车辆移动时，只报告正常状态，不检查其他故障
        else if (!vehicle_stopped) {
            fault_code = 0;  // Normal operation
            can_buffer_full_ = false;  // 重置CAN缓冲区状态
        } 
        // 有位置数据且车辆停止时，检查停止原因
        else {
            if (stop_car_command_) {
                fault_code = 2;
            } else if (avoid_status_ == 3) {
                fault_code = 3;
            } else if (!has_global_path_) {
                fault_code = 4;
            } else if (!has_vehicle_cmd_) {
                fault_code = 5;
            } else {
                // 所有条件都正常但车辆停止，怀疑CAN通信问题
                can_buffer_full_ = true;  // Set as CAN buffer problem
                fault_code = 6;
            }
        }
        
        // 车辆移动时重置CAN缓冲区状态，避免故障码一直显示为6
        if (!vehicle_stopped) {
            can_buffer_full_ = false;
        }
        
        // Set fault code
        diagnostic_msg.fault_code = fault_code;
        
        // Publish diagnostic result
        diagnostic_pub_.publish(diagnostic_msg);
        
        // Print diagnostic information
        if (vehicle_stopped) {
            ROS_WARN("Vehicle STOPPED - Fault code: %d", diagnostic_msg.fault_code);
        } else {
            ROS_INFO("Vehicle MOVING - Status: Normal (fault code: %d)", diagnostic_msg.fault_code);
        }
    }
};

int main(int argc, char** argv)
{
    ros::init(argc, argv, "vehicle_diagnostic_node");
    ros::NodeHandle nh;
    
    VehicleDiagnostic diagnostic(nh);
    
    ros::spin();
    
    return 0;
} 