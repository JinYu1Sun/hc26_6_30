#include <ros/ros.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <udp_com/UdpSocket.h>
#include <udp_com/UdpSend.h>
#include <udp_com/UdpPacket.h>
#include <mower_msgs/VehicleCmd.h>

class UdpVehicleSender {
private:
    ros::NodeHandle nh_;
    ros::NodeHandle private_nh_;
    
    // UDP相关
    ros::ServiceClient create_udp_client_;
    ros::ServiceClient send_udp_client_;
    ros::Subscriber udp_receive_sub_;
    
    // 订阅者
    ros::Subscriber vehicle_cmd_sub_;
    
    // 配置参数
    std::string local_address_;
    std::string remote_address_;
    uint16_t local_port_;
    uint16_t remote_port_;
    bool use_multicast_;
    
    // UDP端口（固定使用一个端口进行通信）
    const uint16_t UDP_PORT = 8888;
    
    // 私有成员回调函数
    void vehicleCmdCallback(const mower_msgs::VehicleCmd::ConstPtr &msg) {
        ROS_DEBUG("Received vehicle command");
        
        // 获取原始值并乘以100保持精度（0.01精度）
        const int SCALE = 100;  // 改为100以保持两位小数精度
        int32_t linear_vel = static_cast<int32_t>(msg->drive_value * SCALE);
        int32_t angular_vel = static_cast<int32_t>(msg->turn_value * SCALE);
        
        // 调试信息
        ROS_INFO("Original values: linear=%.2f, angular=%.2f", 
                 msg->drive_value, msg->turn_value);
        ROS_INFO("Scaled values: linear=%d, angular=%d", linear_vel, angular_vel);
        
        // 准备UDP数据包
        std::vector<uint8_t> udp_data(12);  // 4字节命令头 + 4字节线性速度 + 4字节角速度
        
        // 命令头：0x56454843 (ASCII "VEHC" - Vehicle Command)
        udp_data[0] = 0x56;  // 'V'
        udp_data[1] = 0x45;  // 'E'
        udp_data[2] = 0x48;  // 'H'
        udp_data[3] = 0x43;  // 'C'
        
        // 线性速度（4字节，大端序）
        udp_data[4] = (linear_vel >> 24) & 0xFF;
        udp_data[5] = (linear_vel >> 16) & 0xFF;
        udp_data[6] = (linear_vel >> 8) & 0xFF;
        udp_data[7] = linear_vel & 0xFF;
        
        // 角速度（4字节，大端序）
        udp_data[8] = (angular_vel >> 24) & 0xFF;
        udp_data[9] = (angular_vel >> 16) & 0xFF;
        udp_data[10] = (angular_vel >> 8) & 0xFF;
        udp_data[11] = angular_vel & 0xFF;
        
        // 通过UDP发送数据
        bool success = sendUdpData(udp_data);
        
        if (success) {
            ROS_INFO("Successfully sent vehicle command via UDP");
        } else {
            ROS_ERROR("Failed to send UDP data");
        }
    }
    
    // UDP接收回调（如果需要接收响应）
    void udpReceiveCallback(const udp_com::UdpPacket::ConstPtr &msg) {
        ROS_DEBUG("Received UDP packet from %s:%d, size: %lu", 
                 msg->address.c_str(), msg->srcPort, msg->data.size());
        
        // 解析接收到的数据
        if (msg->data.size() >= 4) {
            // 检查命令头
            if (msg->data[0] == 0x41 && msg->data[1] == 0x43 &&  // "ACK" 响应
                msg->data[2] == 0x4B && msg->data[3] == 0x00) {
                ROS_INFO("Received ACK from vehicle");
                
                // 可以在这里处理响应，比如确认命令执行状态
                if (msg->data.size() >= 5) {
                    uint8_t status = msg->data[4];
                    if (status == 0x00) {
                        ROS_INFO("Command executed successfully");
                    } else {
                        ROS_WARN("Command execution failed, status: 0x%02X", status);
                    }
                }
            }
        }
    }
    
public:
    UdpVehicleSender() : private_nh_("~") {
        // 读取配置参数
        private_nh_.param<std::string>("local_address", local_address_, "0.0.0.0");
        private_nh_.param<std::string>("remote_address", remote_address_, "192.168.1.100");
        private_nh_.param<uint16_t>("local_port", local_port_, UDP_PORT);
        private_nh_.param<uint16_t>("remote_port", remote_port_, UDP_PORT);
        private_nh_.param<bool>("use_multicast", use_multicast_, false);
        
        // 初始化UDP服务客户端
        create_udp_client_ = nh_.serviceClient<udp_com::UdpSocket>(
            "/udp_com_nodelet/create_socket");
        send_udp_client_ = nh_.serviceClient<udp_com::UdpSend>(
            "/udp_com_nodelet/send");
        
        // 等待UDP服务
        ROS_INFO("Waiting for UDP Com Nodelet services...");
        if (!create_udp_client_.waitForExistence(ros::Duration(5.0)) ||
            !send_udp_client_.waitForExistence(ros::Duration(5.0))) {
            ROS_ERROR("UDP Com Nodelet services not available");
            return;
        }
        
        // 创建UDP Socket
        if (!createUdpSocket()) {
            ROS_ERROR("Failed to create UDP socket");
            return;
        }
        
        // 订阅UDP接收话题
        std::string receive_topic = "udp_received_port_" + std::to_string(UDP_PORT);
        udp_receive_sub_ = nh_.subscribe<udp_com::UdpPacket>(
            receive_topic, 10, &UdpVehicleSender::udpReceiveCallback, this);
        
        // 使用lambda表达式订阅车辆控制话题
        vehicle_cmd_sub_ = nh_.subscribe<mower_msgs::VehicleCmd>(
            "/vehicle/cmd", 10, 
            [this](const mower_msgs::VehicleCmd::ConstPtr& msg) {
                this->vehicleCmdCallback(msg);
            });
        
        ROS_INFO("UDP Vehicle Sender initialized");
        ROS_INFO("Local: %s:%d", local_address_.c_str(), local_port_);
        ROS_INFO("Remote: %s:%d", remote_address_.c_str(), remote_port_);
        ROS_INFO("Multicast: %s", use_multicast_ ? "Yes" : "No");
        ROS_INFO("Subscribed to topic: /vehicle/cmd");
    }
    
    ~UdpVehicleSender() {
        ROS_INFO("UDP Vehicle Sender shutting down");
    }
    
    bool createUdpSocket() {
        udp_com::UdpSocket srv;
        srv.request.srcAddress = local_address_;
        srv.request.destAddress = remote_address_;
        srv.request.port = local_port_;
        srv.request.isMulticast = use_multicast_;
        
        ROS_INFO("Creating UDP socket...");
        
        if (create_udp_client_.call(srv)) {
            if (srv.response.success) {
                ROS_INFO("UDP socket created successfully");
                return true;
            } else {
                ROS_ERROR("Failed to create UDP socket (service returned false)");
                return false;
            }
        } else {
            ROS_ERROR("Failed to call create_socket service");
            return false;
        }
    }
    
    bool sendUdpData(const std::vector<uint8_t>& data) {
        udp_com::UdpSend srv;
        srv.request.srcPort = local_port_;
        srv.request.address = remote_address_;
        srv.request.dstPort = remote_port_;
        srv.request.data = data;
        
        // 打印发送的数据
        ROS_DEBUG("Sending UDP data (%lu bytes):", data.size());
        std::string hex_str;
        for (size_t i = 0; i < data.size(); ++i) {
            char buf[4];
            snprintf(buf, sizeof(buf), "%02X ", data[i]);
            hex_str += buf;
            if ((i + 1) % 16 == 0) hex_str += "\n";
        }
        ROS_DEBUG("%s", hex_str.c_str());
        
        if (send_udp_client_.call(srv)) {
            if (srv.response.success) {
                ROS_DEBUG("UDP data sent successfully");
                return true;
            } else {
                if (!srv.response.socketCreated) {
                    ROS_ERROR("UDP socket not created, attempting to recreate...");
                    // 尝试重新创建socket
                    if (createUdpSocket()) {
                        // 重新发送
                        return sendUdpData(data);
                    }
                }
                ROS_ERROR("Failed to send UDP data (service returned false)");
                return false;
            }
        } else {
            ROS_ERROR("Failed to call send service");
            return false;
        }
    }
    
    // 发送测试帧
    bool sendTestFrame() {
        ROS_INFO("Sending UDP test frame...");
        
        std::vector<uint8_t> test_data(16);
        
        // 测试数据头
        test_data[0] = 0x54;  // 'T'
        test_data[1] = 0x45;  // 'E'
        test_data[2] = 0x53;  // 'S'
        test_data[3] = 0x54;  // 'T'
        
        // 填充测试数据
        for (int i = 4; i < 16; i++) {
            test_data[i] = i;
        }
        
        return sendUdpData(test_data);
    }
    
    // 发送紧急停止命令
    bool sendEmergencyStop() {
        ROS_WARN("Sending emergency stop command via UDP");
        
        std::vector<uint8_t> emergency_data(8);
        
        // 紧急停止命令头
        emergency_data[0] = 0x45;  // 'E'
        emergency_data[1] = 0x4D;  // 'M'
        emergency_data[2] = 0x47;  // 'G'
        emergency_data[3] = 0x53;  // 'S' (EMGS - Emergency Stop)
        
        // 设置速度和转向为0
        for (int i = 4; i < 8; i++) {
            emergency_data[i] = 0;
        }
        
        return sendUdpData(emergency_data);
    }
    
    bool isInitialized() const {
        return create_udp_client_.exists() && send_udp_client_.exists();
    }
};

int main(int argc, char** argv) {
    ros::init(argc, argv, "udp_vehicle_sender");
    ros::NodeHandle nh;
    
    ROS_INFO("Starting UDP Vehicle Sender Node");
    
    // 创建UdpVehicleSender实例
    UdpVehicleSender sender;
    
    if (!sender.isInitialized()) {
        ROS_ERROR("Failed to initialize UDP Vehicle Sender. Exiting...");
        return 1;
    }
    
    // 可选：发送测试帧
    ros::NodeHandle private_nh("~");
    if (private_nh.param<bool>("send_test_frame", false)) {
        ROS_INFO("Sending test frame...");
        if (!sender.sendTestFrame()) {
            ROS_WARN("Test frame sending failed");
        }
    }
    
    // 设置ROS循环频率
    ros::Rate rate(10);  // 10Hz
    
    // 主循环
    while (ros::ok()) {
        ros::spinOnce();
        rate.sleep();
    }
    
    // 程序结束前发送停止命令
    ROS_INFO("Shutting down, sending stop command...");
    sender.sendEmergencyStop();
    
    return 0;
}