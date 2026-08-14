/**
 * @file udp_async_node.cpp
 * @brief UDP 异步收发节点（带接收缓冲区）
 * 
 * 功能：
 * - 订阅 /vehicle/cmd，通过 UDP 发送车辆控制命令（与 udp_onlysend_node.cpp 协议一致）
 * - 独立接收线程异步接收 UDP 数据，存入环形缓冲区
 * - 按帧头 0xAA 0x55 + 消息ID 分发解析接收帧（每帧固定5字节，数据大端）：
 *   - ID 0x01：电量，发布到 /vehicle/status（mower_msgs/VehicleStatus）
 *   - ID 0x02：右轮转速（int16），发布到 /vehicle/right_wheel_speed（std_msgs/Int16）
 *   - ID 0x03：左轮转速（int16），发布到 /vehicle/left_wheel_speed（std_msgs/Int16）
 *   - ID 0x04：割盘高度（uint16），发布到 /vehicle/mower_height（std_msgs/UInt16），写入 height_msg.txt（只保留最新值），并从文件读回后按同格式回发给下位机
 */

#include <ros/ros.h>
#include <fstream>    // 核心：文件流
#include <iostream>   // 可选：用于 std::cout 打印调试
#include <mower_msgs/VehicleCmd.h>
#include <mower_msgs/VehicleStatus.h>
#include <std_msgs/UInt8MultiArray.h>
#include <std_msgs/Int16.h>
#include <std_msgs/UInt16.h>
#include <std_msgs/String.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include <cmath>
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
    ros::Publisher left_speed_pub_;
    ros::Publisher right_speed_pub_;
    ros::Publisher height_pub_;
    ros::Timer height_echo_timer_;

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
        

        // 创建左右轮转速发布者（std_msgs/Int16，单位由发送端约定）
        left_speed_pub_ = nh_.advertise<std_msgs::Int16>("/vehicle/left_wheel_speed", 10);
        right_speed_pub_ = nh_.advertise<std_msgs::Int16>("/vehicle/right_wheel_speed", 10);
        
        // 创建割盘高度发布者（std_msgs/UInt16）
        height_pub_ = nh_.advertise<std_msgs::UInt16>("/vehicle/mower_height_to_app", 10);
        
        // 启动接收线程
        running_ = true;
        recv_thread_ = std::thread(&UdpAsyncNode::receiveLoop, this);
        
        height_echo_timer_ = nh_.createTimer(ros::Duration(0.5), &UdpAsyncNode::heightEchoCallback, this);
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
            
            // ===== 帧头校验：AA 55 + 消息ID + 2字节数据，每帧固定5字节 =====
            if (pkt.data.size() < 5 || pkt.data[0] != 0xAA || pkt.data[1] != 0x55) {
                ROS_WARN("bad frame header, drop [%zu bytes] from %s:%d",
                         pkt.data.size(), pkt.src_ip.c_str(), pkt.src_port);
                continue;   // 不是我们的帧或长度不够，直接丢弃
            }
            
            uint8_t msg_id = pkt.data[2];
            // 有效数据为第4、5字节，大端拼装（带符号 int16）
            
            switch (msg_id) {
            case 0x01: {   // ===== 电量帧 =====
                mower_msgs::VehicleStatus status_msg;
                status_msg.header.stamp = pkt.stamp;
                const uint16_t MAX_SOC_RAW = 0xF9F0;
                uint8_t warning_state_one = 0;
                uint8_t warning_state_two = 0;
                uint8_t battery_soc = 0;

                // 电量按无符号解析，计算百分比，只保留整数部分
                uint16_t raw = static_cast<uint16_t>(pkt.data[3] << 8 | pkt.data[4]);
                if (raw > MAX_SOC_RAW) {
                    battery_soc = 100;
                    // 数据大于 F9F0：警报2的 bit0 置1
                    warning_state_two |= 0x01;
                } else {
                    battery_soc = static_cast<uint8_t>((raw * 100U) / MAX_SOC_RAW);
                }
                status_msg.battery_soc = battery_soc;
                status_msg.warning_state_one = warning_state_one;
                status_msg.warning_state_two = warning_state_two;

                recv_pub_.publish(status_msg);
                break;
            }
            case 0x02: {   // ===== 右轮转速帧 =====
                std_msgs::Int16 speed_msg;
                speed_msg.data = static_cast<int16_t>(pkt.data[6] << 8 | pkt.data[5]);
                ROS_INFO("rightspeed=%d,data3=%x,data4=%x",speed_msg.data,pkt.data[5],pkt.data[6]);
                right_speed_pub_.publish(speed_msg);
                break;
            }
            case 0x03: {   // ===== 左轮转速帧 =====
                std_msgs::Int16 speed_msg;
                speed_msg.data = static_cast<int16_t>(pkt.data[6] << 8 | pkt.data[5]);
                ROS_INFO("leftspeed=%d,data3=%x,data4=%x",speed_msg.data,pkt.data[5],pkt.data[6]);
                left_speed_pub_.publish(speed_msg);
                break;
            }
            case 0x04: {   // ===== 割盘高度帧 =====
                std_msgs::UInt16 height_msg;
                height_msg.data = std::min<int>(906, static_cast<uint16_t>(pkt.data[3] << 8 | pkt.data[4]));
        
                ROS_INFO("pulsenum=%d",height_msg.data);
                // 每收到一帧就把最新值追加到文件，只保留最近 kMaxHeightLines 条
                const std::string height_path = "/home/nvidia/crawler_control/src/udp_com_main/height_msg.txt";
                const size_t kMaxHeightLines = 100;   // 想保存多少条就改这里

                std::deque<std::string> lines;
                std::ifstream fin(height_path);
                std::string line;
                while (std::getline(fin, line)) {
                    lines.push_back(line);
                }
                fin.close();

                while (lines.size() >= kMaxHeightLines) {
                    lines.pop_front();
                }
                lines.push_back(std::to_string(height_msg.data));

                std::ofstream fout(height_path, std::ios::trunc);
                if (!fout) {
                    ROS_ERROR("open height_msg.txt for write failed");
                    break;
                }
                for (const std::string &l : lines) {
                    fout << l << '\n';
                }
                fout.close();
                break;
            }
            default:
                ROS_WARN("unknown msg id: 0x%02X", msg_id);
                break;
            }
            
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

    void heightEchoCallback(const ros::TimerEvent&) {
        const std::string height_path = "/home/nvidia/crawler_control/src/udp_com_main/height_msg.txt";

        // 以二进制+末尾定位方式打开，先判断最后一个字节是不是 '\n'
        std::ifstream fin(height_path, std::ios::binary | std::ios::ate);
        if (!fin) {
            return;   // 文件还不存在
        }
        if (fin.tellg() == 0) {
            return;   // 空文件
        }

        fin.seekg(-1, std::ios::end);
        char last_char = 0;
        fin.get(last_char);
        const bool last_line_complete = (last_char == '\n');

        // 回到开头，逐行读，同时记住最后一行和倒数第二行
        fin.seekg(0);
        std::string line, last, prev;
        while (std::getline(fin, line)) {
            if (line.empty()) continue;   // 跳过空行
            prev = last;
            last = line;
        }
        fin.close();

        // 最后一行完整就用它；不完整（写了一半断电）说明损坏，退回上一条
        std::string chosen;
        if (last_line_complete) {
            chosen = last;
        } else {
            chosen = prev;
            ROS_WARN("height_msg.txt last line incomplete, use previous value");
        }
        if (chosen.empty()) {
            return;   // 没有可用的完整数据（比如文件里只有一条残行）
        }

        uint16_t height = static_cast<uint16_t>(std::atoi(chosen.c_str()));
        std_msgs::UInt16 height_to_app_;
        height_to_app_.data = round(11 - 9*height/906);
        ROS_INFO("height=%d,height_to_app=%d", height, height_to_app_.data);
        height_pub_.publish(height_to_app_);
        // 组帧回发：AA 55 + 0x04 + 2字节数据（大端）
        uint8_t tx[5] = {0xAA, 0x55, 0x06,
                        static_cast<uint8_t>(height >> 8),
                        static_cast<uint8_t>(height & 0xFF)};
        ssize_t sent = sendto(udp_socket_, tx, sizeof(tx), 0,
                            (struct sockaddr*)&remote_addr_, sizeof(remote_addr_));
        if (sent < 0) {
            ROS_WARN("height echo send failed: %s", strerror(errno));
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
