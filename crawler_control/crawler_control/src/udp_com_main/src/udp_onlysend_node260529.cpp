/**
 * @file udp_onlysend_node.cpp
 * @brief UDP车辆控制命令发送器（只发送，不接收）
 * 
 * 修改说明：
 * - 将本地源端口默认值改为 8888（原为 0，表示系统分配）
 * - 仍保留通过私有参数 local_port 覆盖默认值的功能
 */

#include <ros/ros.h>
#include <mower_msgs/VehicleCmd.h>  // 使用mower_msgs消息
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <vector>
#include <cmath>

// 定义常量宏
#define DEFAULT_PORT 8888
#define DATA_SIZE 14

class UdpVehicleSender {
private:
    ros::NodeHandle nh_;
    ros::NodeHandle private_nh_;
    ros::Subscriber vehicle_cmd_sub_;  // 修改订阅者名称
    
    int udp_socket_;
    struct sockaddr_in remote_addr_;
    
    // 配置参数
    std::string remote_ip_;
    int remote_port_;
    bool send_test_frame_;
    int local_port_;            // 本地源端口（0表示系统分配，默认8888）
    
public:
    UdpVehicleSender() : 
        udp_socket_(-1),
        private_nh_("~") {
        
        // 1. 读取配置参数
        loadParameters();
        
        // 2. 初始化UDP socket
        if (!initializeUdpSocket()) {
            ROS_ERROR("Failed to initialize UDP socket. Shutting down...");
            ros::shutdown();
            return;
        }
        
        // 3. 订阅车辆控制话题 - 使用mower_msgs::VehicleCmd
        vehicle_cmd_sub_ = nh_.subscribe<mower_msgs::VehicleCmd>(
            "/vehicle/cmd", 10, &UdpVehicleSender::vehicleCmdCallback, this);
        
        // 4. 可选：发送测试帧
        if (send_test_frame_) {
            sendTestFrame();
        }
    }
    
    ~UdpVehicleSender() {
        // 清理资源
        if (udp_socket_ >= 0) {
            close(udp_socket_);
            ROS_INFO("UDP socket closed");
        }
        
        // 发送停止命令
        sendStopCommand();
        ROS_INFO("UDP Vehicle Sender shut down");
    }
    
    /**
     * @brief 检查socket是否有效
     * @return socket是否有效
     */
    bool isInitialized() const {
        return udp_socket_ >= 0;
    }
    
private:
    /**
     * @brief 加载配置参数
     */
    void loadParameters() {
        // 目标IP地址
        private_nh_.param<std::string>("remote_ip", remote_ip_, "192.168.1.30");
        
        // 目标端口
        private_nh_.param<int>("remote_port", remote_port_, DEFAULT_PORT);
        
        // 是否发送测试帧
        private_nh_.param<bool>("send_test_frame", send_test_frame_, false);
        
        // 本地源端口：默认 8888（可通过 _local_port:=其他值 覆盖）
        private_nh_.param<int>("local_port", local_port_, 8888);
        
        ROS_INFO("Configuration loaded:");
        ROS_INFO("  Remote IP: %s", remote_ip_.c_str());
        ROS_INFO("  Remote Port: %d", remote_port_);
        ROS_INFO("  Send Test Frame: %s", send_test_frame_ ? "Yes" : "No");
        ROS_INFO("  Local Port: %d (0=auto assign)", local_port_);
    }
    
    /**
     * @brief 初始化UDP Socket
     * @return 初始化是否成功
     */
    bool initializeUdpSocket() {
        // 创建UDP socket
        udp_socket_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (udp_socket_ < 0) {
            ROS_ERROR("Failed to create UDP socket: %s", strerror(errno));
            return false;
        }
        ROS_INFO("UDP socket created successfully (fd: %d)", udp_socket_);
        
        // --- 绑定本地端口（如果指定了 local_port）---
        if (local_port_ > 0) {
            struct sockaddr_in local_addr;
            memset(&local_addr, 0, sizeof(local_addr));
            local_addr.sin_family = AF_INET;
            local_addr.sin_addr.s_addr = INADDR_ANY;   // 绑定到所有可用接口
            local_addr.sin_port = htons(local_port_);

            if (bind(udp_socket_, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
                ROS_ERROR("Failed to bind local port %d: %s", local_port_, strerror(errno));
                close(udp_socket_);
                udp_socket_ = -1;
                return false;
            }
            ROS_INFO("Bound to local port: %d", local_port_);
        } else {
            ROS_INFO("Local port will be assigned by OS (dynamic)");
        }
        // -------------------------------------------------
        
        // 设置socket选项（允许广播）
        int broadcast = 1;
        if (setsockopt(udp_socket_, SOL_SOCKET, SO_BROADCAST, 
                      &broadcast, sizeof(broadcast)) < 0) {
            ROS_WARN("Failed to set broadcast option: %s", strerror(errno));
        }
        
        // 设置发送超时
        struct timeval tv;
        tv.tv_sec = 1;   // 1秒超时
        tv.tv_usec = 0;
        if (setsockopt(udp_socket_, SOL_SOCKET, SO_SNDTIMEO, 
                      &tv, sizeof(tv)) < 0) {
            ROS_WARN("Failed to set send timeout: %s", strerror(errno));
        }
        
        // 配置远程地址
        memset(&remote_addr_, 0, sizeof(remote_addr_));
        remote_addr_.sin_family = AF_INET;
        remote_addr_.sin_port = htons(remote_port_);
        
        // 转换IP地址
        if (inet_pton(AF_INET, remote_ip_.c_str(), &remote_addr_.sin_addr) <= 0) {
            ROS_ERROR("Invalid IP address: %s", remote_ip_.c_str());
            close(udp_socket_);
            udp_socket_ = -1;
            return false;
        }
        
        ROS_INFO("Socket configured for %s:%d", remote_ip_.c_str(), remote_port_);
        return true;
    }
    
    /**
     * @brief 车辆控制命令回调函数
     * @param msg 车辆控制消息
     */
    void vehicleCmdCallback(const mower_msgs::VehicleCmd::ConstPtr &msg) {
        // 数据验证
        if (!std::isfinite(msg->drive_value) || !std::isfinite(msg->turn_value)) {
            ROS_WARN("Received NaN or INF values, skipping");
            return;
        }
        
        // 生成UDP数据包
        std::vector<uint8_t> udp_data = createUdpPacket(msg->mower_height,msg->drive_value, msg->turn_value);
        
        // 发送数据
        sendUdpData(udp_data);
        
        // 调试信息（每秒显示一次，避免日志过多）
        static ros::Time last_log_time = ros::Time::now();
        ros::Time now = ros::Time::now();
        if ((now - last_log_time).toSec() > 1.0) {
            ROS_INFO("Sending command: drive=%d., turn=%d", 
                    msg->drive_value, msg->turn_value);
            last_log_time = now;
        }
    }
    
    /**
     * @brief 创建UDP数据包
     * @param drive_value 驱动值
     * @param turn_value 转向值
     * @return UDP数据包
     */
    std::vector<uint8_t> createUdpPacket(int mover_bool, float drive_value, float turn_value) {
        std::vector<uint8_t> data(DATA_SIZE);
        
        // 1. 命令头："VEHC" (Vehicle Command)
        data[0] = 'V';  // 0x56
        data[1] = 'E';  // 0x45
        data[2] = 'H';  // 0x48
        data[3] = 'C';  // 0x43
        
        // 2. 驱动值（32位有符号整数，大端序，单位：0.01）
        
        
        const int SCALE = 1;  // 保持两位小数精度
        int32_t drive_scaled = static_cast<int32_t>(drive_value * SCALE);
        
        
        data[4] = static_cast<uint8_t>((drive_scaled >> 24) & 0xFF);
        data[5] = static_cast<uint8_t>((drive_scaled >> 16) & 0xFF);
        data[6] = static_cast<uint8_t>((drive_scaled >> 8) & 0xFF);
        data[7] = static_cast<uint8_t>(drive_scaled & 0xFF);
        
        // 3. 转向值（32位有符号整数，大端序，单位：0.01）
        int32_t turn_scaled = static_cast<int32_t>(turn_value * SCALE);
        
        data[8] = static_cast<uint8_t>((turn_scaled >> 24) & 0xFF);
        data[9] = static_cast<uint8_t>((turn_scaled >> 16) & 0xFF);
        data[10] = static_cast<uint8_t>((turn_scaled >> 8) & 0xFF);
        data[11] = static_cast<uint8_t>(turn_scaled & 0xFF);
        
        
        int32_t mover_bool1 = static_cast<int32_t>(mover_bool);
        // data[12] = static_cast<uint8_t>(mover_bool1>>8 & 0xFF);
        // data[13] = static_cast<uint8_t>(mover_bool1 & 0xFF);
        data[12]=static_cast<uint8_t>(0x00);
        data[13]=static_cast<uint8_t>(0x00);
        return data;
    }
    
    /**
     * @brief 发送UDP数据
     * @param data 要发送的数据
     * @return 发送是否成功
     */
    bool sendUdpData(const std::vector<uint8_t>& data) {
        if (udp_socket_ < 0) {
            ROS_ERROR("UDP socket not initialized");
            return false;
        }
        
        ssize_t bytes_sent = sendto(udp_socket_, 
                                   data.data(), 
                                   data.size(),
                                   0,  // flags
                                   (struct sockaddr*)&remote_addr_,
                                   sizeof(remote_addr_));
        
        if (bytes_sent < 0) {
            // 错误处理
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                ROS_DEBUG("Send buffer full, packet dropped");
            } else if (errno == ECONNREFUSED) {
                ROS_WARN("Connection refused by remote host");
            } else {
                ROS_WARN("Failed to send UDP data: %s", strerror(errno));
            }
            return false;
        } else if (bytes_sent != static_cast<ssize_t>(data.size())) {
            ROS_WARN("Partial send: %zd/%zu bytes", bytes_sent, data.size());
            return false;
        }
        
        return true;
    }
    
    /**
     * @brief 发送测试帧
     */
    void sendTestFrame() {
        ROS_INFO("Sending test frame...");
        
        // 测试数据：drive=0.5, turn=0.2
        std::vector<uint8_t> test_data = createUdpPacket(0,0.5f, 0.2f);
        
        // 修改头为测试标识"TEST"
        test_data[0] = '1';
        test_data[1] = '2';
        test_data[2] = '3';
        test_data[3] = '4';
        
        if (sendUdpData(test_data)) {
            ROS_INFO("Test frame sent successfully");
            
            // 打印测试数据
            std::string hex_str;
            for (size_t i = 0; i < test_data.size(); ++i) {
                char buf[8];
                snprintf(buf, sizeof(buf), "%02X ", test_data[i]);
                hex_str += buf;
            }
            ROS_INFO("Test data: %s", hex_str.c_str());
        } else {
            ROS_WARN("Failed to send test frame");
        }
    }
    
    /**
     * @brief 发送停止命令
     */
    void sendStopCommand() {
        ROS_INFO("Sending stop command...");
        
        // 停止命令：drive=0.0, turn=0.0
        std::vector<uint8_t> stop_data = createUdpPacket(0,0.0f, 0.0f);
        
        // 修改头为停止标识"STOP"
        stop_data[0] = '1';
        stop_data[1] = '2';
        stop_data[2] = '3';
        stop_data[3] = '4';
        
        // 发送3次确保收到
        for (int i = 0; i < 3; ++i) {
            sendUdpData(stop_data);
            usleep(10000);  // 10ms间隔
        }
        
        ROS_INFO("Stop command sent");
    }
};

/**
 * @brief 主函数
 */
int main(int argc, char** argv) {
    // 初始化ROS节点
    ros::init(argc, argv, "udp_vehicle_sender");
    ROS_INFO("Starting UDP Vehicle Sender Node...");
    
    // 创建发送器实例
    UdpVehicleSender sender;
    
    // 检查初始化状态
    if (!sender.isInitialized()) {
        ROS_ERROR("Failed to initialize UDP Vehicle Sender");
        return 1;
    }
    
    // 主循环
    ROS_INFO("Entering main loop (press Ctrl+C to exit)");
    ros::spin();
    
    ROS_INFO("UDP Vehicle Sender terminated");
    return 0;
}