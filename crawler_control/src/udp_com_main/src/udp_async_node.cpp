/**
 * @file udp_async_node.cpp
 * @brief UDP 异步收发节点（带接收缓冲区）
 * 
 * 功能：
 * - 订阅 /vehicle/cmd，通过 UDP 发送车辆控制命令（与 udp_onlysend_node.cpp 协议一致）
 * - 独立接收线程异步接收 UDP 数据，存入环形缓冲区
 * - 将接收到的原始数据发布到 /udp/received 话题（std_msgs/UInt8MultiArray）
 */

#include <ros/ros.h>
#include <mower_msgs/VehicleCmd.h>
#include <mower_msgs/VehicleStatus.h>
#include <std_msgs/UInt8MultiArray.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include <string.h>
#include <errno.h>
#include <vector>
#include <deque>
#include <mutex>
#include <thread>
#include <atomic>

#define DATA_SIZE 15

struct UdpPacket {
    std::vector<uint8_t> data;
    std::string src_ip;
    int src_port;
    ros::Time stamp;
};

class UdpAsyncNode {
private:
    ros::NodeHandle nh_;
    ros::NodeHandle private_nh_;
    ros::Subscriber vehicle_cmd_sub_;
    ros::Publisher recv_pub_;
    
    int udp_socket_;
    struct sockaddr_in remote_addr_;
    
    // 参数
    std::string remote_ip_;
    int remote_port_;
    int local_port_;
    int max_queue_size_;
    
    // 接收线程与缓冲区
    std::thread recv_thread_;
    std::atomic<bool> running_;
    std::deque<UdpPacket> rx_queue_;
    std::mutex rx_mutex_;
    
public:
    UdpAsyncNode() : udp_socket_(-1), private_nh_("~"), running_(false), max_queue_size_(100) {
        loadParameters();
        
        if (!initializeUdpSocket()) {
            ROS_ERROR("Failed to initialize UDP socket");
            ros::shutdown();
            return;
        }
        
        // 订阅车辆控制话题
        vehicle_cmd_sub_ = nh_.subscribe<mower_msgs::VehicleCmd>(
            "/vehicle/cmd", 10, &UdpAsyncNode::vehicleCmdCallback, this);
        
        // 创建接收数据发布者
        recv_pub_ = nh_.advertise<mower_msgs::VehicleStatus>("/vehicle/status", 100);
        
        // 启动接收线程
        running_ = true;
        recv_thread_ = std::thread(&UdpAsyncNode::receiveLoop, this);
        
        ROS_INFO("UDP Async Node started");
        ROS_INFO("  Local port: %d", local_port_);
        ROS_INFO("  Remote: %s:%d", remote_ip_.c_str(), remote_port_);
        ROS_INFO("  Max RX queue: %d", max_queue_size_);
    }
    
    ~UdpAsyncNode() {
        shutdown();
    }
    
    bool isInitialized() const {
        return udp_socket_ >= 0;
    }
    
private:
    void shutdown() {
        running_ = false;
        
        if (recv_thread_.joinable()) {
            recv_thread_.join();
        }
        
        if (udp_socket_ >= 0) {
            sendStopCommand();
            close(udp_socket_);
            udp_socket_ = -1;
            ROS_INFO("UDP socket closed");
        }
        
        ROS_INFO("UDP Async Node shut down");
    }
    
    void loadParameters() {
        private_nh_.param<std::string>("remote_ip", remote_ip_, "192.168.1.30");
        private_nh_.param<int>("remote_port", remote_port_, 8888);
        private_nh_.param<int>("local_port", local_port_, 8888);
        private_nh_.param<int>("max_queue_size", max_queue_size_, 100);
        
        ROS_INFO("Parameters loaded:");
        ROS_INFO("  Remote IP: %s", remote_ip_.c_str());
        ROS_INFO("  Remote Port: %d", remote_port_);
        ROS_INFO("  Local Port: %d", local_port_);
    }
    
    bool initializeUdpSocket() {
        udp_socket_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (udp_socket_ < 0) {
            ROS_ERROR("Failed to create socket: %s", strerror(errno));
            return false;
        }
        
        // 允许地址重用
        int reuse = 1;
        if (setsockopt(udp_socket_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
            ROS_WARN("Failed to set SO_REUSEADDR: %s", strerror(errno));
        }
        
        // 绑定本地端口（收发节点必须绑定，否则无法稳定接收）
        struct sockaddr_in local_addr;
        memset(&local_addr, 0, sizeof(local_addr));
        local_addr.sin_family = AF_INET;
        local_addr.sin_addr.s_addr = INADDR_ANY;
        local_addr.sin_port = htons(local_port_);
        
        if (bind(udp_socket_, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
            ROS_ERROR("Failed to bind port %d: %s", local_port_, strerror(errno));
            close(udp_socket_);
            udp_socket_ = -1;
            return false;
        }
        ROS_INFO("Bound to local port: %d", local_port_);
        
        // 配置远程地址
        memset(&remote_addr_, 0, sizeof(remote_addr_));
        remote_addr_.sin_family = AF_INET;
        remote_addr_.sin_port = htons(remote_port_);
        if (inet_pton(AF_INET, remote_ip_.c_str(), &remote_addr_.sin_addr) <= 0) {
            ROS_ERROR("Invalid remote IP: %s", remote_ip_.c_str());
            close(udp_socket_);
            udp_socket_ = -1;
            return false;
        }
        
        // 设置发送超时
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        if (setsockopt(udp_socket_, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
            ROS_WARN("Failed to set send timeout: %s", strerror(errno));
        }
        
        return true;
    }
    
    /**
     * @brief 接收线程主循环（使用 poll 实现可中断的等待）
     */
    void receiveLoop() {
        std::vector<uint8_t> buf(1500);
        struct sockaddr_in sender_addr;
        socklen_t addr_len = sizeof(sender_addr);
        
        while (running_) {
            struct pollfd pfd;
            pfd.fd = udp_socket_;
            pfd.events = POLLIN;
            
            // 100ms 超时，便于及时检查 running_ 标志退出线程
            int ret = poll(&pfd, 1, 100);
            if (ret < 0) {
                if (errno == EINTR) continue;
                ROS_WARN("poll error: %s", strerror(errno));
                usleep(10000);
                continue;
            }
            if (ret == 0) continue; // 超时，继续循环
            
            ssize_t n = recvfrom(udp_socket_, buf.data(), buf.size(), 0,
                                 (struct sockaddr*)&sender_addr, &addr_len);
            if (n < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
                ROS_WARN("recvfrom error: %s", strerror(errno));
                continue;
            }
            
            UdpPacket pkt;
            pkt.data.assign(buf.begin(), buf.begin() + n);
            pkt.src_ip = inet_ntoa(sender_addr.sin_addr);
            pkt.src_port = ntohs(sender_addr.sin_port);
            pkt.stamp = ros::Time::now();
            
            {
                std::lock_guard<std::mutex> lock(rx_mutex_);
                rx_queue_.push_back(pkt);
                if (rx_queue_.size() > max_queue_size_) {
                    rx_queue_.pop_front();
                }
            }
            
            // 发布到 ROS 话题，方便其他节点处理
            mower_msgs::VehicleStatus status_msg;
            status_msg.header.stamp = ros::Time::now();
            const uint16_t MAX_SOC_RAW = 0xF9F0;
            uint8_t warning_state_one = 0;
            uint8_t warning_state_two = 0;
            uint8_t battery_soc = 0;

            if (pkt.data.size() < 2) {
    // 没有接收到足够数据：警报1的 bit0 置1
                warning_state_one |= 0x01;
            } else {
                // 把两字节拼成 16 位无符号数（大端：F9 40 → 0xF940）
                uint16_t raw = (static_cast<uint16_t>(pkt.data[0]) << 8) | pkt.data[1];
                
                // 计算百分比，只保留整数部分
                if (raw > MAX_SOC_RAW) {
                    battery_soc = 100;
                    // 数据大于 F9F0：警报2的 bit0 置1
                    warning_state_two |= 0x01;
                } else {
                    battery_soc = static_cast<uint8_t>((raw * 100U) / MAX_SOC_RAW);
                }
            }
            status_msg.battery_soc = battery_soc;
            status_msg.warning_state_one = warning_state_one;
            status_msg.warning_state_two = warning_state_two;

            recv_pub_.publish(status_msg);
            
            ROS_INFO("RX [%zd bytes] from %s:%d | queue=%zu/%d",
                     n, pkt.src_ip.c_str(), pkt.src_port,
                     rx_queue_.size(), max_queue_size_);
        }
    }
    
    void vehicleCmdCallback(const mower_msgs::VehicleCmd::ConstPtr &msg) {
        std::vector<uint8_t> data = createUdpPacket(
            msg->mover_bool, msg->drive_value, msg->turn_value, msg->mower_height,msg->ad_control_enable);
        
        ssize_t sent = sendto(udp_socket_, data.data(), data.size(), 0,
                              (struct sockaddr*)&remote_addr_, sizeof(remote_addr_));
        if (sent < 0) {
            ROS_WARN("Send failed: %s", strerror(errno));
        } else if (sent != DATA_SIZE) {
            ROS_WARN("Partial send: %zd/%d", sent, DATA_SIZE);
        } else {
            ROS_INFO("TX: drive=%d, turn=%d,ad_control=%d", msg->drive_value, msg->turn_value,msg->ad_control_enable);
        }
    }
    
    std::vector<uint8_t> createUdpPacket(int mover_bool, int drive_value, int turn_value, int mower_height,int ad_control_enable) {
        std::vector<uint8_t> data(DATA_SIZE);
        data[0] = 'V'; data[1] = 'E'; data[2] = 'H'; data[3] = 'C';
        
        int32_t drive_scaled = static_cast<int32_t>(drive_value);
        data[4] = (drive_scaled >> 24) & 0xFF;
        data[5] = (drive_scaled >> 16) & 0xFF;
        data[6] = (drive_scaled >> 8) & 0xFF;
        data[7] = drive_scaled & 0xFF;
        
        int32_t turn_scaled = static_cast<int32_t>(turn_value);
        data[8] = (turn_scaled >> 24) & 0xFF;
        data[9] = (turn_scaled >> 16) & 0xFF;
        data[10] = (turn_scaled >> 8) & 0xFF;
        data[11] = turn_scaled & 0xFF;
        
        data[12] = static_cast<uint8_t>(mover_bool);
        data[13] = static_cast<uint8_t>(mower_height);
        data[14] = static_cast<uint8_t>(ad_control_enable);
        return data;
    }
 
    void sendStopCommand() {
        ROS_INFO("Sending stop command...");
        std::vector<uint8_t> stop_data = createUdpPacket(0, 0, 0, 0,0);
        stop_data[0] = '1'; stop_data[1] = '2';
        stop_data[2] = '3'; stop_data[3] = '4';
        for (int i = 0; i < 3; ++i) {
            sendto(udp_socket_, stop_data.data(), stop_data.size(), 0,
                   (struct sockaddr*)&remote_addr_, sizeof(remote_addr_));
            usleep(10000);
        }
    }
    
public:
    /**
     * @brief 获取当前接收缓冲区副本（线程安全）
     */
    std::deque<UdpPacket> getRxQueue() {
        std::lock_guard<std::mutex> lock(rx_mutex_);
        return rx_queue_;
    }
    
    /**
     * @brief 获取接收缓冲区当前大小
     */
    size_t getRxQueueSize() {
        std::lock_guard<std::mutex> lock(rx_mutex_);
        return rx_queue_.size();
    }
};

int main(int argc, char** argv) {
    ros::init(argc, argv, "udp_async_node");
    ROS_INFO("Starting UDP Async Node...");
    
    UdpAsyncNode node;
    if (!node.isInitialized()) {
        ROS_ERROR("Initialization failed");
        return 1;
    }
    
    ros::spin();
    ROS_INFO("UDP Async Node terminated");
    return 0;
}
