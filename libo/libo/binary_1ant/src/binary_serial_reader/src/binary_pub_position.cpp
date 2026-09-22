#include <ros/ros.h>
#include <serial/serial.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <cstdint>
#include "pub_position.h"
#include "util/GpsPosition.h"
#include <std_msgs/Empty.h>
#include <cstdlib>
#include <ctime>

bool imugood_flag = false;
bool rtk_flag = false;
std::string command = "reset"; // 替换为实际指令
double publish_window_start = 0;
int publish_count = 0;
int publish_error_cnt_ = 0;
bool reboot = false;

// 工具函数：从 buffer 中小端读取类型 T
template<typename T>
T read_le(const uint8_t* data) {
    T value;
    std::memcpy(&value, data, sizeof(T));
    return value;
}

constexpr size_t IMU_HEADER_LEN = 0x1C;
constexpr size_t RTK_HEADER_LEN = 0x18;

enum IMUStatus : uint32_t {
    INS_INACTIVE = 0,               // 无效
    INS_ALIGNING = 1,               // 初始对准
    INS_HIGH_VARIANCE = 2,          // 航向角误差超限
    INS_SOLUTION_GOOD = 3,          // 正常
    INS_SOLUTION_FREE = 6,          // 无GNSS解参与组合解算
    INS_ALIGNMENT_COMPLETE = 7      // 完成初始对准，但无足够的动态使其满足精度指标要求
};

enum INSPOSType : uint32_t {
    NONE__ = 0,             // 无解
    INS__ = 1,              // 纯惯导定位解
    INS_PSRSP__ = 2,        // 惯导与单点定位组合解
    INS_PSRDIFF__ = 3,      // 惯导与伪距差分定位组合解
    INS_RTKFLOAT__ = 6,     // 惯导与载波相位差分浮点解组合解
    INS_RTKFIXED__ = 7      // 惯导与载波相位差分固定解组合解
};

enum POSVELType : uint32_t {
    NONE = 0,             // 无解
    FIXEDPOS = 1,         // 位置由 FIX POSITION 命令指定
    FIXEDHEIGHT = 2,      // 暂不支持
    DOPPLER_VELOCITY = 8, // 速度由即时多普勒信息导出
    SINGLE = 16,          // 单点定位
    PSRDIFF = 17,         // 伪距差分解
    SBAS = 18,            // SBAS定位
    L1_FLOAT = 32,        // L1 浮点解
    IONOFREE_FLOAT = 33,  // 消电离层浮点解
    NARROW_FLOAT = 34,    // 窄巷浮点解
    L1_INT = 48,          // L1 固定解
    WIDE_INT = 49,        // 宽巷固定解
    NARROW_INT = 50,      // 窄巷固定解
    INS = 52,             // 纯惯导定位解
    INS_PSRSP = 53,       // 惯导与单点定位组合解
    INS_PSRDIFF = 54,     // 惯导与伪距差分定位组合解
    INS_RTKFLOAT = 55,    // 惯导与载波相位差分浮点解组合解
    INS_RTKFIXED = 56,    // 惯导与载波相位差分固定解组合解
    PPP_CONVERGING = 68,  // PPP 状态收敛中
    PPP  = 69             // PPP定位
};

void parseIMUATTA(const std::vector<uint8_t>& buffer) {
    if (buffer.size() < RTK_HEADER_LEN + 36) return;

    uint32_t imu_status_raw = read_le<uint32_t>(&buffer[RTK_HEADER_LEN + 0]);
    IMUStatus imu_status = static_cast<IMUStatus>(imu_status_raw);
    uint32_t pos_status_raw = read_le<uint32_t>(&buffer[RTK_HEADER_LEN + 4]);
    INSPOSType imu_pos_status = static_cast<INSPOSType>(pos_status_raw);

    int16_t roll = read_le<int16_t>(&buffer[RTK_HEADER_LEN + 14]);
    int16_t pitch = read_le<int16_t>(&buffer[RTK_HEADER_LEN + 16]);
    int16_t azimuth = read_le<int16_t>(&buffer[RTK_HEADER_LEN + 18]);

    int16_t acc_x = read_le<int16_t>(&buffer[RTK_HEADER_LEN + 20]);
    int16_t acc_y = read_le<int16_t>(&buffer[RTK_HEADER_LEN + 22]);
    int16_t acc_z = read_le<int16_t>(&buffer[RTK_HEADER_LEN + 24]);

    int16_t gyro_x = read_le<int16_t>(&buffer[RTK_HEADER_LEN + 26]);
    int16_t gyro_y = read_le<int16_t>(&buffer[RTK_HEADER_LEN + 28]);
    int16_t gyro_z = read_le<int16_t>(&buffer[RTK_HEADER_LEN + 30]);

    double roll_deg = roll * 360.0 / 32767;
    double pitch_deg = pitch * 360.0 / 32767;
    double yaw_deg = azimuth * 360.0 / 32767;

    double ax = acc_x * 80.0 / 32767;
    double ay = acc_y * 80.0 / 32767;
    double az = acc_z * 80.0 / 32767;

    double gx = gyro_x * 500.0 / 32767;
    double gy = gyro_y * 500.0 / 32767;
    double gz = gyro_z * 500.0 / 32767;

    if(imu_status == INS_SOLUTION_GOOD){
        imugood_flag = true;
    } else {
        imugood_flag = false;
    }

    // ROS_INFO("[IMUATTA] Roll: %.2f, Pitch: %.2f, Yaw: %.2f", roll_deg, pitch_deg, yaw_deg);
    // ROS_INFO("[IMUATTA] Acc: [%.2f, %.2f, %.2f] m/s^2", ax, ay, az);
    // ROS_INFO("[IMUATTA] Gyro: [%.2f, %.2f, %.2f] deg/s", gx, gy, gz);

    // ROS_INFO("[IMUATTA] IMU Status: %u, Position Status: %u", imu_status, imu_pos_status);
    // ROS_INFO("imugood_flag = %d", imugood_flag);
}

void parseDRPVA(const std::vector<uint8_t>& buffer, ros::Publisher &pub) {
    if (buffer.size() < IMU_HEADER_LEN + 200) return;

    gausskruger::WGS84Projection projection;

    double lat = read_le<double>(&buffer[IMU_HEADER_LEN + 24]);
    double lon = read_le<double>(&buffer[IMU_HEADER_LEN + 32]);
    double hgt = read_le<double>(&buffer[IMU_HEADER_LEN + 40]);

    // 判断经纬度和高度是否有效
    if (lat < -90.0 || lat > 90.0) {
        ROS_ERROR("[DRPVA] Invalid latitude: %.8f", lat);
        return;
    }
    if (lon < -180.0 || lon > 180.0) {
        ROS_ERROR("[DRPVA] Invalid longitude: %.8f", lon);
        return;
    }
    if (hgt < -500.0 || hgt > 10000.0) {
        ROS_ERROR("[DRPVA] Invalid height: %.3f", hgt);
        return;
    }

    double ve = read_le<double>(&buffer[IMU_HEADER_LEN + 64]);
    double vn = read_le<double>(&buffer[IMU_HEADER_LEN + 72]);
    double vu = read_le<double>(&buffer[IMU_HEADER_LEN + 80]);

    double heading = read_le<double>(&buffer[IMU_HEADER_LEN + 100]);
    double pitch = read_le<double>(&buffer[IMU_HEADER_LEN + 108]);
    double roll = read_le<double>(&buffer[IMU_HEADER_LEN + 116]);

    double gauss_x,gauss_y;
    // 将经纬度转换为高斯平面坐标
    projection.geodeticToGrid(lat, lon, gauss_x, gauss_y);

    // 计算水平速度
    double speed = sqrt(ve * ve + vn * vn);

    // 航向角转换
    heading = 90 - heading;
    if (heading < 0)
    {
        heading += 360;
    }
    else if (heading > 360)
    {
        heading -= 360;
    }

    //ROS_INFO("[DRPVA] Lat: %.8f, Lon: %.8f, Hgt: %.3f", lat, lon, hgt);
    //ROS_INFO("[DRPVA] Vn: %.2f, Ve: %.2f, Vu: %.2f", vn, ve, vu);
    //ROS_INFO("[DRPVA] Heading: %.2f, Pitch: %.2f, Roll: %.2f", heading, pitch, roll);

    util::GpsPosition gpsPosition;
    if (rtk_flag && imugood_flag)
    {
        gpsPosition.header.frame_id = "/gps";
        gpsPosition.header.stamp = ros::Time::now();

        gpsPosition.gps_flag = 4;

        gpsPosition.longitude = lon;
        gpsPosition.latitude = lat;
        gpsPosition.height = hgt;

        gpsPosition.gaussX = gauss_x * 100;
        gpsPosition.gaussY = gauss_y * 100;

        gpsPosition.azimuth = heading * 100;

        gpsPosition.gps_confidence = 4;

        pub.publish(gpsPosition);

        publish_count++;
    }
}

void parseRTKSTATUS(const std::vector<uint8_t>& buffer) {
    if (buffer.size() < RTK_HEADER_LEN + 56) return;

    uint32_t rtk_status_raw = read_le<uint32_t>(&buffer[RTK_HEADER_LEN + 44]);
    POSVELType rtk_status = static_cast<POSVELType>(rtk_status_raw);

    if(rtk_status == NARROW_INT){
        rtk_flag = true;
    } else {
        rtk_flag = false;
    }

    //ROS_INFO("[RTKSTATUS] RTK Status: %u", rtk_status);
    //ROS_INFO("rtk_flag = %d", rtk_flag);
}

int main(int argc, char** argv) {
    ros::init(argc, argv, "imuatta_drpva_decoder");
    ros::NodeHandle nh;

    ros::Publisher pub = nh.advertise<util::GpsPosition>("/nanobot/gpsposition", 1);

    serial::Serial ser;
    try {
        ser.setPort("/dev/ttyTHS1");
        ser.setBaudrate(115200);
        serial::Timeout to = serial::Timeout::simpleTimeout(1000);
        ser.setTimeout(to);
        ser.open();
    } catch (serial::IOException& e) {
        ROS_ERROR("Unable to open port");
        return -1;
    }

    ROS_INFO("Serial port opened successfully");

    std::vector<uint8_t> buffer;
    while (ros::ok()) {
        size_t n = ser.available();
        if (n > 0) {
            std::vector<uint8_t> data(n);
            size_t read_bytes = ser.read(data.data(), n);

            if (read_bytes > 0) {
                printf("time =  %14.4f  raw-data: ", ros::Time::now().toSec());
                for (size_t i = 0; i < read_bytes; i++) {
                    printf("%02X ", data[i]);
                }
                printf("\n");

                // 将读取的二进制数据追加到buffer中
                buffer.insert(buffer.end(), data.begin(), data.begin() + read_bytes);
            }
        }

        publish_window_start = (publish_window_start == 0) ? ros::Time::now().toSec() : publish_window_start;
        double current_time = ros::Time::now().toSec();
        double window_duration = current_time - publish_window_start;
        // printf("current_time = %14.4f\n", current_time);
        // printf("publish_window_start = %14.4f\n", publish_window_start);
        // std::cout << "Duration: " << window_duration << std::endl;
	//ROS_WARN("restarting the node!");  
               // ser.write(command);
	//	ser.write("\r\n");
        //        ROS_INFO("Command sent: %s", command.c_str());


        if (rtk_flag && imugood_flag && window_duration >= 1.0) // 检查 1 秒内的发布频率
        {
            double publish_frequency = publish_count / window_duration;
            std::cout << "Publish frequency: " << publish_frequency << " Hz" << std::endl;
            if (publish_frequency < 18)
            {
                publish_error_cnt_++;
                reboot = publish_error_cnt_ >= 5 ? true : reboot;
            }
            else
            {
                publish_error_cnt_ = 0;
            }
            publish_window_start = current_time;
            publish_count = 0; // 重置计数器
            if(reboot){
		ROS_WARN("Publish frequency is too low: %.2f Hz, restarting the node!", publish_frequency);
                ser.write(command);
                ROS_INFO("Command sent: %s", command.c_str());    
            }
        }

        // 处理buffer中的数据帧
        while (buffer.size() >= 60) { // 根据最小数据包长度判断
            if (buffer[0] == 0xAA && buffer[1] == 0x44 && buffer[2] == 0x12) { // IMU相关
                uint16_t msg_id = read_le<uint16_t>(&buffer[4]);
                uint16_t msg_len = read_le<uint16_t>(&buffer[8]);

                if (buffer.size() < IMU_HEADER_LEN + msg_len) break;

                if (msg_id == 0xDEC0) {
                    parseDRPVA(buffer, pub);
                }
                buffer.erase(buffer.begin(), buffer.begin() + IMU_HEADER_LEN + msg_len);

            } else if (buffer[0] == 0xAA && buffer[1] == 0x44 && buffer[2] == 0xB5) { // RTK相关
                uint16_t msg_id = read_le<uint16_t>(&buffer[4]);
                uint16_t msg_len = read_le<uint16_t>(&buffer[6]);

                if (buffer.size() < RTK_HEADER_LEN + msg_len) break;

                if (msg_id == 0x01FD) {
                    parseRTKSTATUS(buffer);
                }else if (msg_id == 0x05A2) {
                    parseIMUATTA(buffer);
                }    
                buffer.erase(buffer.begin(), buffer.begin() + RTK_HEADER_LEN + msg_len);

            } else {
                // 找不到包头，丢弃第一个字节
                buffer.erase(buffer.begin());
            }
        }
        ros::spinOnce();
    }
    return 0;
}

