#include <ros/ros.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <boost/asio.hpp>
#include <mower_msgs/VehicleCmd.h>

using boost::asio::ip::udp;

class UdpVehicleSender {
private:
    ros::NodeHandle nh_;
    ros::NodeHandle private_nh_;
    
    // UDP相关
    boost::asio::io_service io_service_;
    udp::socket udp_socket_;
    udp::endpoint remote_endpoint_;
    boost::thread io_thread_;
    
    // 订阅者
    ros::Subscriber vehicle_cmd_sub_;
    
    // 配置参数
    std::string local_address_;
    std::string remote_address_;
    uint16_t local_port_;
    uint16_t remote_port_;
    bool use_multicast_;
    bool is_initialized_;
    
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
    
public:
    UdpVehicleSender() : 
        private_nh_("~"),
        udp_socket_(io_service_),
        is_initialized_(false) {
        
        // 读取配置参数
        private_nh_.param<std::string>("local_address", local_address_, "0.0.0.0");
        private_nh_.param<std::string>("remote_address", remote_address_, "192.168.1.100");
        private_nh_.param<uint16_t>("local_port", local_port_, 8888);
        private_nh_.param<uint16_t>("remote_port", remote_port_, 8888);
        private_nh_.param<bool>("use_multicast", use_multicast_, false);
        
        // 初始化UDP Socket
        if (!initializeUdpSocket()) {
            ROS_ERROR("Failed to initialize UDP socket");
            return;
        }
        
        // 使用lambda表达式订阅车辆控制话题
        vehicle_cmd_sub_ = nh_.subscribe<mower_msgs::VehicleCmd>(
            "/vehicle/cmd", 10, 
            [this](const mower_msgs::VehicleCmd::ConstPtr& msg) {
                this->vehicleCmdCallback(msg);
            });
        
        // 启动IO服务线程
        io_thread_ = boost::thread(boost::bind(&boost::asio::io_service::run, &io_service_));
        
        is_initialized_ = true;
        
        ROS_INFO("UDP Vehicle Sender initialized");
        ROS_INFO("Local: %s:%d", local_address_.c_str(), local_port_);
        ROS_INFO("Remote: %s:%d", remote_address_.c_str(), remote_port_);
        ROS_INFO("Multicast: %s", use_multicast_ ? "Yes" : "No");
        ROS_INFO("Subscribed to topic: /vehicle/cmd");
    }
    
    ~UdpVehicleSender() {
        // 停止IO服务
        io_service_.stop();
        
        // 关闭socket
        if (udp_socket_.is_open()) {
            udp_socket_.close();
        }
        
        // 等待线程结束
        if (io_thread_.joinable()) {
            io_thread_.join();
        }
        
        ROS_INFO("UDP Vehicle Sender shutting down");
    }
    
    bool initializeUdpSocket() {
        try {
            // 解析地址
            boost::asio::ip::address local_addr = 
                boost::asio::ip::address::from_string(local_address_);
            boost::asio::ip::address remote_addr = 
                boost::asio::ip::address::from_string(remote_address_);
            
            // 创建端点
            udp::endpoint local_endpoint(local_addr, local_port_);
            remote_endpoint_ = udp::endpoint(remote_addr, remote_port_);
            
            // 打开socket
            udp_socket_.open(udp::v4());
            
            // 设置socket选项
            udp_socket_.set_option(udp::socket::reuse_address(true));
            
            // 设置缓冲区大小
            udp_socket_.set_option(boost::asio::socket_base::send_buffer_size(65536));
            udp_socket_.set_option(boost::asio::socket_base::receive_buffer_size(65536));
            
            // 绑定到本地地址
            udp_socket_.bind(local_endpoint);
            ROS_INFO("Socket bound to %s:%d", local_address_.c_str(), local_port_);
            
            // 如果是多播，设置多播选项
            if (use_multicast_ && remote_addr.is_multicast()) {
                udp_socket_.set_option(boost::asio::ip::multicast::enable_loopback(true));
                udp_socket_.set_option(
                    boost::asio::ip::multicast::join_group(remote_addr));
                ROS_INFO("Joined multicast group: %s", remote_address_.c_str());
            }
            
            // 允许广播（可选）
            udp_socket_.set_option(boost::asio::socket_base::broadcast(true));
            
            return true;
            
        } catch (const std::exception& e) {
            ROS_ERROR("Failed to initialize UDP socket: %s", e.what());
            return false;
        }
    }
    
    bool sendUdpData(const std::vector<uint8_t>& data) {
        try {
            // 发送数据
            size_t bytes_sent = udp_socket_.send_to(
                boost::asio::buffer(data), remote_endpoint_);
            
            // 打印发送的数据
            ROS_DEBUG("Sent %lu bytes to %s:%d", 
                     bytes_sent, remote_address_.c_str(), remote_port_);
            
            if (bytes_sent == data.size()) {
                ROS_DEBUG("UDP data sent successfully");
                return true;
            } else {
                ROS_WARN("Partial send: %lu of %lu bytes", bytes_sent, data.size());
                return false;
            }
            
        } catch (const std::exception& e) {
            ROS_ERROR("Failed to send UDP data: %s", e.what());
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
    
    // 添加接收功能（可选）
    void startReceiving() {
        // 异步接收数据
        std::vector<uint8_t> recv_buffer(65507);  // UDP最大包大小
        
        udp_socket_.async_receive_from(
            boost::asio::buffer(recv_buffer), remote_endpoint_,
            [this, recv_buffer](boost::system::error_code ec, std::size_t bytes_recvd) {
                if (!ec && bytes_recvd > 0) {
                    ROS_DEBUG("Received %lu bytes from %s:%d", 
                             bytes_recvd, 
                             remote_endpoint_.address().to_string().c_str(),
                             remote_endpoint_.port());
                    
                    // 处理接收到的数据
                    processReceivedData(recv_buffer, bytes_recvd);
                }
                
                // 继续接收
                if (udp_socket_.is_open()) {
                    startReceiving();
                }
            }
        );
    }
    
    void processReceivedData(const std::vector<uint8_t>& data, size_t size) {
        // 处理接收到的数据
        if (size >= 4) {
            // 检查命令头
            if (data[0] == 0x41 && data[1] == 0x43 &&  // "ACK" 响应
                data[2] == 0x4B && data[3] == 0x00) {
                ROS_INFO("Received ACK from vehicle");
                
                // 可以在这里处理响应，比如确认命令执行状态
                if (size >= 5) {
                    uint8_t status = data[4];
                    if (status == 0x00) {
                        ROS_INFO("Command executed successfully");
                    } else {
                        ROS_WARN("Command execution failed, status: 0x%02X", status);
                    }
                }
            }
        }
    }
    
    bool isInitialized() const {
        return is_initialized_;
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
    
    // 可选：启动接收功能
    sender.startReceiving();
    
    // 可选：发送测试帧
    ros::NodeHandle private_nh("~");
    if (private_nh.param<bool>("send_test_frame", false)) {
        ROS_INFO("Sending test frame...");
        if (!sender.sendTestFrame()) {
            ROS_WARN("Test frame sending failed");
        }
    }
    
    // 主循环
    ros::spin();
    
    // 程序结束前发送停止命令
    ROS_INFO("Shutting down, sending stop command...");
    sender.sendEmergencyStop();
    
    return 0;
}