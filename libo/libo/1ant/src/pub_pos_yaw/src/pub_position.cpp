#include "pub_position.h"
#include "util/GpsPosition.h"
#include <ros/ros.h>
#include <std_msgs/Empty.h>
#include <std_msgs/Bool.h>
#include <std_msgs/String.h>
#include <serial/serial.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <stdexcept>
#include <sstream>
#include <cmath>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <atomic>
#include <limits>

// ----------------- 原有全局/变量 -----------------
ros::Time last_publish_time; // 记录最后一次发布消息的时间

unsigned long last_week = 0;
double last_seconds = 0.0;
double last_gauss_x = 0.0, last_gauss_y = 0.0;
int heading_match_count = 0;            // 连续匹配计数器
const int heading_match_threshold = 20; // 连续匹配次数阈值
bool heading_flag = false;
int frame_counter = 0;
bool restart_imu = false;
double first_height = 0.0;

// DRPVA and IMUATTA are independent receiver logs.  Cache only the newest
// verified IMU/GGA samples and require them to be fresh when a DRPVA frame is
// published, so a stale "INS_SOLUTION_GOOD" cannot survive a data dropout.
ins_protocol::ImuAttaData g_latest_imu;
ins_protocol::GgaData g_latest_gga;
// Receipt/freshness decisions use a monotonic clock. ROS wall time can jump
// when NTP/GNSS/manual clock correction occurs and must not drive watchdogs or
// cached-frame ages.
double g_latest_imu_rx_steady_sec = 0.0;
double g_latest_gga_rx_steady_sec = 0.0;
// IMUATTA/GNGGA are configured at 1 Hz, while DRPVAA is 20 Hz and reuses the
// latest verified status. IMUATTA gets 2.5 periods to tolerate one missed log;
// GNGGA (the RTK-fixed gate) expires after one period plus scheduling margin.
constexpr double kMaxImuAgeSec = 2.50;
constexpr double kMaxGgaAgeSec = 1.50;

// ----------------- 健壮性/看门狗相关全局 -----------------
std::atomic<double> g_last_rx_time{0.0};  // 最近一次收到串口数据的单调时钟(秒)
std::atomic<double> g_last_pub_time{0.0}; // 最近一次成功发布的单调时钟(秒)
std::atomic<bool> g_need_reopen{false};   // 看门狗/重置请求重新打开串口

// 复位所有解析过程状态(供 init_location / INS 复位时调用, 避免依赖进程重启)
void resetParserState()
{
    heading_flag = false;
    first_height = 0.0;
    heading_match_count = 0;
    frame_counter = 0;
    last_week = 0;
    last_seconds = 0.0;
    last_gauss_x = 0.0;
    last_gauss_y = 0.0;
    g_latest_imu = ins_protocol::ImuAttaData();
    g_latest_gga = ins_protocol::GgaData();
    g_latest_imu_rx_steady_sec = 0.0;
    g_latest_gga_rx_steady_sec = 0.0;
}

class GNSS_CRC32
{
private:
    static const uint32_t CRC_TABLE[256];

public:
    // 计算字符串的CRC32
    static uint32_t calculate(const std::string &data)
    {
        return calculate(reinterpret_cast<const uint8_t *>(data.data()), data.length());
    }

    // 计算字节数组的CRC32（UM98系列说明书上CRC校验代码的直接翻译）
    static uint32_t calculate(const uint8_t *buffer, int size)
    {
        uint32_t crc = 0;
        for (int i = 0; i < size; i++)
        {
            crc = CRC_TABLE[(crc ^ buffer[i]) & 0xff] ^ (crc >> 8);
        }
        return crc;
    }

    // 验证数据的CRC32（用于你的GNSS数据）
    static bool verify(const std::string &data_with_crc)
    {
        // 查找CRC分隔符 '*'
        size_t crc_pos = data_with_crc.find_last_of('*');
        if (crc_pos == std::string::npos)
        {
            return false;
        }

        // 提取数据和CRC值
        std::string data = data_with_crc.substr(1, crc_pos - 1);
        std::string crc_hex = data_with_crc.substr(crc_pos + 1);

        // 计算数据的CRC
        uint32_t calculated_crc = calculate(data);

        // 将十六进制字符串转换为整数(防止行噪声/截断帧导致 std::stoul 抛出未捕获异常)
        uint32_t expected_crc = 0;
        try
        {
            expected_crc = std::stoul(crc_hex, nullptr, 16);
        }
        catch (const std::exception &)
        {
            return false;
        }

        if (calculated_crc == expected_crc)
        {
            return true;
        }
        else
        {
            std::cout << "Calculated CRC: 0x" << std::hex << std::uppercase << calculated_crc << "\n";
            std::cout << "Expected CRC:   0x" << std::hex << std::uppercase << expected_crc << "\n";
            return false;
        }
    }

    // 获取CRC的十六进制字符串
    static std::string to_hex_string(uint32_t crc)
    {
        char buffer[9];
        snprintf(buffer, sizeof(buffer), "%08X", crc);
        return std::string(buffer);
    }
};

// 必须在类外定义静态成员
const uint32_t GNSS_CRC32::CRC_TABLE[256] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
    0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
    0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d};

void test_gnss_crc_verification()
{
    std::cout << "Testing GNSS CRC32 verification...\n\n";

    // 你的测试数据
    std::string test_data_with_crc = "#INSPVAXA,COM1,0,73.5,FINESTEERING,1695,309428.000,00000040,4e77,43562;INS_SOLUTION_GOOD,INS_PSRSP,51.11637873403,-114.03825114994,1063.6093,-16.9000,-0.0845,-0.0464,-0.0127,0.138023492,0.069459386,90.000923268,0.9428,0.6688,1.4746,0.0430,0.0518,0.0521,0.944295466,0.944567084,1.000131845,3,0*e877c178";

    // 验证CRC
    bool is_valid = GNSS_CRC32::verify(test_data_with_crc);

    std::cout << "Data with CRC: " << test_data_with_crc << "\n\n";
    std::cout << "CRC Verification: " << (is_valid ? "PASS" : "FAIL") << "\n";

    if (!is_valid)
    {
        // 如果不匹配，显示计算出的CRC值
        size_t crc_pos = test_data_with_crc.find_last_of('*');
        std::string data = test_data_with_crc.substr(0, crc_pos);
        uint32_t calculated_crc = GNSS_CRC32::calculate(data);

        std::cout << "Calculated CRC: 0x" << std::hex << std::uppercase << calculated_crc << "\n";
        std::cout << "Expected CRC:   0x9AA8B508\n";
    }
}

// 用于处理实时数据流的函数
class GNSSDataParser
{
public:
    static bool parseAndVerify(const std::string &raw_data)
    {
        return GNSS_CRC32::verify(raw_data);
    }

    static std::string extractData(const std::string &raw_data)
    {
        size_t crc_pos = raw_data.find_last_of('*');
        if (crc_pos != std::string::npos)
        {
            return raw_data.substr(0, crc_pos);
        }
        return raw_data;
    }

    static uint32_t calculateCRC(const std::string &data)
    {
        return GNSS_CRC32::calculate(data);
    }
};

// 打开(或重新打开)串口; 失败返回 false 由调用方择机重试, 不再因构造抛异常而崩溃
bool openSerial(serial::Serial &sp, const std::string &port, uint32_t baud)
{
    try
    {
        if (sp.isOpen())
        {
            sp.close();
        }
        sp.setPort(port);
        sp.setBaudrate(baud);
        serial::Timeout to = serial::Timeout::simpleTimeout(1000); // setTimeout 取非常量引用, 需 lvalue
        sp.setTimeout(to);
        sp.open();
        if (sp.isOpen())
        {
            sp.flushInput();
            ROS_INFO("Serial port %s opened (baud %u).", port.c_str(), baud);
            return true;
        }
        return false;
    }
    catch (const std::exception &e)
    {
        ROS_ERROR("Open serial %s failed: %s", port.c_str(), e.what());
        return false;
    }
}

// 软件看门狗线程: 监控串口数据是否中断(节点存活但不再收发数据的根因)。
// 数据中断时只请求"重开串口", 由主线程执行, 避免跨线程操作串口及杀整节点。
void watchdogThread()
{
    const double RX_TIMEOUT = 3.0;  // 超过该秒数无串口数据 -> 重开串口
    const double PUB_TIMEOUT = 5.0; // 串口正常但长时间不发布 -> 仅告警(多为 INS/RTK 未就绪)
    ros::Rate rate(2);              // 2Hz 巡检
    while (ros::ok())
    {
        const double now = ros::SteadyTime::now().toSec();
        double rx_age = now - g_last_rx_time.load();
        double pub_age = now - g_last_pub_time.load();

        if (g_last_rx_time.load() > 0.0 && rx_age > RX_TIMEOUT)
        {
            if (!g_need_reopen.load())
            {
                ROS_WARN("Watchdog: no serial data for %.1fs -> request serial reopen", rx_age);
            }
            g_need_reopen.store(true);
        }
        else if (g_last_pub_time.load() > 0.0 && pub_age > PUB_TIMEOUT && rx_age < RX_TIMEOUT)
        {
            ROS_WARN_THROTTLE(5.0,
                              "Watchdog: serial alive but no publish for %.1fs (INS not GOOD / RTK not fixed?)",
                              pub_age);
        }
        rate.sleep();
    }
}

PubPosition::PubPosition()
{
    old_x = 1;
    old_y = 1;
}
PubPosition::~PubPosition()
{
    // Destructor logic (if any)
}

double PubPosition::compute_gps(double a)
{
    int integerPart = static_cast<int>(a / 100);
    // 计算剩余部分
    double remainingPart = fmod(a, 100);
    // 将剩余部分除以60得到结果
    return integerPart + (remainingPart / 60);
}

void PubPosition::processSerialData(const std::string &serial_data,
                                    ros::Publisher &pub,
                                    const ins_protocol::ImuAttaData &imu,
                                    double imu_age_sec,
                                    const ins_protocol::GgaData &gga,
                                    double gga_age_sec,
                                    const ros::Time &frame_rx_ros_stamp,
                                    double frame_rx_steady_sec)
{
    // ---------- CRC32 校验（仅对 '#' 类帧） ----------
    // 约定：CRC 是在 '*' 之后以 ASCII hex (8 chars) 给出，校验计算范围为 '#' 之后到 '*' 之前的所有字节（不包括 '#' 和 '*'）。

    if (!GNSS_CRC32::verify(serial_data))
    {
        return;
    }

    ins_protocol::DrpvaData drpva;
    if (!ins_protocol::ParseDrpva(serial_data, &drpva))
    {
        ROS_WARN_THROTTLE(2.0, "[DRPVA] verified frame has an invalid field layout");
        return;
    }
    if (!drpva.solution_computed || !drpva.rtk_fixed)
    {
        return;
    }
    if (!imu.valid || !imu.solution_good || imu_age_sec < 0.0 ||
        imu_age_sec > kMaxImuAgeSec)
    {
        ROS_WARN_THROTTLE(1.0,
                          "[DRPVA] reject stale/invalid IMUATTA (valid=%d good=%d age=%.3fs)",
                          imu.valid ? 1 : 0, imu.solution_good ? 1 : 0,
                          imu_age_sec);
        return;
    }

    util::GpsPosition gpsPosition;
    gausskruger::WGS84Projection projection;
    const double latitude = drpva.latitude_deg;
    const double longitude = drpva.longitude_deg;
    const double height = drpva.height_m;
    double gauss_x = 0.0;
    double gauss_y = 0.0;
    double yaw = 0.0;
    const double seconds = drpva.gps_millisecond * 1e-3;
    const unsigned long week = drpva.gps_week;
    const int confidence = 4;

    // 判断经纬度和高度是否有效
    if (latitude < -90.0 || latitude > 90.0 || fabs(latitude) < 1e-1 || latitude == 0.0)
    {
        ROS_ERROR("[DRPVA] Invalid latitude: %.8f", latitude);
        return;
    }
    if (longitude < -180.0 || longitude > 180.0 || fabs(longitude) < 1e-1 || longitude == 0.0)
    {
        ROS_ERROR("[DRPVA] Invalid longitude: %.8f", longitude);
        return;
    }
    if (height < -500.0 || height > 10000.0 || fabs(height) < 1e-1 || height == 0.0)
    {
        ROS_ERROR("[DRPVA] Invalid height: %.3f", height);
        return;
    }
    // 高度相对参考值跳变过大: 复位航向匹配状态使其重新收敛, 而非永久拒绝本帧
    // (原实现会因一次高程跳变导致此后每帧 return -> 节点永久停发, 仅能靠重启恢复)
    if (heading_flag && fabs(height - first_height) > 50.0)
    {
        ROS_WARN("[DRPVA] height jump %.1fm (ref %.1f) -> reset heading state", height - first_height, first_height);
        heading_flag = false;
        first_height = 0.0;
        heading_match_count = 0;
    }

    // PDF defines heading in [0, 360]. In particular 0/90/360 are valid.
    if (!ins_protocol::HeadingNorthClockwiseToEastCounterClockwise(
            drpva.heading_deg, &yaw) ||
        !std::isfinite(drpva.pitch_deg) || !std::isfinite(drpva.roll_deg) ||
        fabs(drpva.pitch_deg) > 90.0 || fabs(drpva.roll_deg) > 180.0)
    {
        ROS_ERROR("[DRPVA] Invalid attitude: heading=%.3f pitch=%.3f roll=%.3f",
                  drpva.heading_deg, drpva.pitch_deg, drpva.roll_deg);
        return;
    }

    projection.geodeticToGrid(latitude, longitude, gauss_x, gauss_y);
    gauss_y += 500000.0; // 添加500公里假东距

    // 判断高斯坐标是否有效(3度带范围)
    if (gauss_x == 0 || fabs(gauss_x) > 10000000 || fabs(gauss_x) < 5e-1 || gauss_y == 0 || gauss_y < 166000 || gauss_y > 833000 || fabs(gauss_y) < 5e-1)
    {
        ROS_ERROR("[DRPVA] Invalid Gauss coordinates: X: %.3f, Y: %.3f", gauss_x, gauss_y);
        return;
    }

    double speed = 0.0, calculated_heading = 0.0;
    if (confidence == 4)
    {
        frame_counter++;
        if (frame_counter % 2 == 0)
        {
            // 计算基于高斯坐标的车速和航向
            if (last_seconds && last_week)
            {
                double delta_time = (week == last_week) ? (seconds - last_seconds) : (604800 + seconds - last_seconds); // 处理跨周的情况
                double delta_x = gauss_x - last_gauss_x;
                double delta_y = gauss_y - last_gauss_y;

                if (delta_time > 1e-3 && delta_time < 1.0)
                {
                    // 计算车速
                    double distance = sqrt(delta_x * delta_x + delta_y * delta_y); // 单位：米
                    speed = distance / delta_time;

                    // gauss_x=北向, gauss_y=东向，因此该角同样是东=0、逆时针为正。
                    calculated_heading = atan2(delta_x, delta_y) * 180 / M_PI;
                    if (calculated_heading < 0)
                    {
                        calculated_heading += 360;
                    }
                }
            }

            // 更新上一帧的高斯坐标和时间
            last_gauss_x = gauss_x;
            last_gauss_y = gauss_y;
            last_week = week;
            last_seconds = seconds;
        }

        // 判断航向差异
        if (!heading_flag && speed > 0.15)
        {
            double heading_diff = fabs(calculated_heading - yaw);
            std::cout << "Calculated heading: " << calculated_heading << " °, Yaw: " << yaw << " °" << std::endl;
            std::cout << "Heading difference: " << heading_diff << " ° " << std::endl;
            if (heading_diff > 180)
            {
                heading_diff = 360 - heading_diff;
            }

            if (heading_diff < 20) // 差异小于20度
            {
                heading_match_count++;
            }
            else
            {
                heading_match_count = 0; // 重置计数器
            }

            if (heading_match_count >= heading_match_threshold)
            {
                heading_flag = true;
                heading_match_count = heading_match_threshold; // 防止溢出
                first_height = height;
            }
            else
            {
                heading_flag = false;
                first_height = 0.0;
            }
            std::cout << "Heading match count: " << heading_match_count << std::endl;
            std::cout << "Heading flag: " << (heading_flag ? "true" : "false") << std::endl;
        }

        gpsPosition.header.frame_id = "/gps";
        // Timestamp at complete serial-frame receipt, before CRC parsing,
        // projection, quality gates and ROS publication add variable latency.
        // This remains a host-receipt estimate, not the DRPVA measurement epoch.
        gpsPosition.header.stamp = frame_rx_ros_stamp;

        gpsPosition.plat_id = 0;
        gpsPosition.error_code = 0;
        gpsPosition.gps_flag = gga.quality;
        gpsPosition.positionStatus = heading_flag ? 1 : 0; // 位置+航向已连续验证
        gpsPosition.gps_week = drpva.gps_week;
        gpsPosition.gps_millisecond = drpva.gps_millisecond;

        gpsPosition.longitude = longitude;
        gpsPosition.latitude = latitude;
        gpsPosition.height = height;

        gpsPosition.gaussX = gauss_x * 100;
        gpsPosition.gaussY = gauss_y * 100;

        // GpsPosition.msg要求姿态单位为0.01度。DRPVA的pitch/roll是度，
        // 保留接收机车体轴定义；右轴pitch到ROS左轴pitch的符号转换
        // 由fusion_4dof的ins_pitch_scale_rad唯一完成。
        gpsPosition.pitch = drpva.pitch_deg * 100.0;
        gpsPosition.roll = drpva.roll_deg * 100.0;
        gpsPosition.azimuth = yaw * 100;

        // IMUATTA原始惯性量已按协议量程转成消息声明的SI/度每秒单位。
        gpsPosition.acc_x = imu.acc_x_mps2;
        gpsPosition.acc_y = imu.acc_y_mps2;
        gpsPosition.acc_z = imu.acc_z_mps2;
        gpsPosition.rot_x = imu.gyro_x_dps;
        gpsPosition.rot_y = imu.gyro_y_dps;
        gpsPosition.rot_z = imu.gyro_z_dps;

        gpsPosition.northVelocity = drpva.north_velocity_mps * 100.0;
        gpsPosition.eastVelocity = drpva.east_velocity_mps * 100.0;
        gpsPosition.upVelocity = drpva.up_velocity_mps * 100.0;

        // GpsPosition.msg中8--10表示良好；当前只在RTK fixed + INS good时发布。
        gpsPosition.gps_confidence = 10;
        gpsPosition.INS_GpsFlag_Pos = imu.position_type; // UM98x INS position type raw code
        gpsPosition.INS_GpsFlag_Heading = imu.solution_good ? 1 : 0;
        gpsPosition.INS_Status = imu.ins_status;          // 3 == INS_SOLUTION_GOOD
        if (gga.valid && gga_age_sec >= 0.0 && gga_age_sec <= kMaxGgaAgeSec)
        {
            gpsPosition.INS_NumSV = gga.satellites;
            const double age = std::max(0.0, std::min(255.0, gga.differential_age_sec));
            gpsPosition.INS_Gps_Age = static_cast<uint8_t>(std::lround(age));
        }
        // INS_Car_Status/INS_VehicleAlign在UM98x当前输出中无同义字段，
        // 保持0，避免伪造有效状态。
        gpsPosition.INS_Car_Status = 0;
        gpsPosition.INS_VehicleAlign = 0;
        gpsPosition.INS_Std_Lat = drpva.latitude_std_m;
        gpsPosition.INS_Std_Lon = drpva.longitude_std_m;
        gpsPosition.INS_Std_LocatHeight = drpva.height_std_m;
        gpsPosition.INS_Std_Heading = drpva.heading_std_deg;

        ROS_INFO_THROTTLE(1.0,
                          "[GPS-INS] r/p/y_ros=(%.3f,%.3f,%.3f)deg std_h=%.3fdeg "
                          "vel_neu=(%.3f,%.3f,%.3f)m/s imu_age=%.3fs sv=%u",
                          drpva.roll_deg, -drpva.pitch_deg, yaw,
                          drpva.heading_std_deg, drpva.north_velocity_mps,
                          drpva.east_velocity_mps, drpva.up_velocity_mps,
                          imu_age_sec, static_cast<unsigned int>(gpsPosition.INS_NumSV));
    }
    pub.publish(gpsPosition);
    last_publish_time = ros::Time::now();
    g_last_pub_time.store(ros::SteadyTime::now().toSec());
    const double processing_latency_ms =
        (ros::SteadyTime::now().toSec() - frame_rx_steady_sec) * 1000.0;
    ROS_DEBUG_THROTTLE(1.0,
                       "[DRPVA-TIME] complete-frame receipt to publish %.3f ms",
                       processing_latency_ms);
}

// 解析单行串口数据并按消息类型分发(用 return 代替 continue, 保证主循环每轮都能 spinOnce)
void handleSerialLine(const std::string &serial_data, PubPosition &pubPosition,
                      ros::Publisher &pub, bool &imugood_flag, int &gga_gps_flag,
                      const ros::Time &frame_rx_ros_stamp,
                      double frame_rx_steady_sec)
{
    // 仅处理以 '#'/'$' 开头且含 '*' 的完整帧
    if (serial_data.empty() || (serial_data[0] != '#' && serial_data[0] != '$') ||
        (serial_data.find("*") == std::string::npos))
    {
        return;
    }

    // GNGGA: 取定位质量、卫星数和差分龄期。
    if (serial_data.find("$GNGGA") != std::string::npos)
    {
        ins_protocol::GgaData parsed;
        if (ins_protocol::ParseGga(serial_data, &parsed))
        {
            g_latest_gga = parsed;
            g_latest_gga_rx_steady_sec = frame_rx_steady_sec;
            gga_gps_flag = parsed.quality;
        }
        else
        {
            ROS_WARN_THROTTLE(2.0, "Malformed GNGGA frame");
        }
        return;
    }

    // IMUATTA: 判断 INS 解状态
    if (serial_data.find("#IMUATTA") != std::string::npos)
    {
        if (!GNSS_CRC32::verify(serial_data))
        {
            return;
        }
        ins_protocol::ImuAttaData parsed;
        if (!ins_protocol::ParseImuAtta(serial_data, &parsed))
        {
            imugood_flag = false;
            g_latest_imu = ins_protocol::ImuAttaData();
            ROS_WARN_THROTTLE(2.0, "Verified IMUATTA frame has an invalid field layout");
            return;
        }
        g_latest_imu = parsed;
        g_latest_imu_rx_steady_sec = frame_rx_steady_sec;
        imugood_flag = parsed.solution_good;
        return;
    }

    // DRPVA: 满足 RTK 固定 + INS GOOD 时解析并发布
    if (serial_data.find("#DRPVA") != std::string::npos)
    {
        const double imu_age_sec = g_latest_imu_rx_steady_sec <= 0.0
                                       ? std::numeric_limits<double>::infinity()
                                       : frame_rx_steady_sec - g_latest_imu_rx_steady_sec;
        const double gga_age_sec = g_latest_gga_rx_steady_sec <= 0.0
                                       ? std::numeric_limits<double>::infinity()
                                       : frame_rx_steady_sec - g_latest_gga_rx_steady_sec;
        if (gga_gps_flag == 4 && gga_age_sec >= 0.0 &&
            gga_age_sec <= kMaxGgaAgeSec && imugood_flag &&
            imu_age_sec >= 0.0 && imu_age_sec <= kMaxImuAgeSec)
        {
            pubPosition.processSerialData(serial_data, pub, g_latest_imu,
                                          imu_age_sec, g_latest_gga,
                                          gga_age_sec, frame_rx_ros_stamp,
                                          frame_rx_steady_sec);
        }
        else
        {
            ROS_WARN_THROTTLE(1.0,
                              "DRPVA gated: gga=%d age=%.2fs imu_good=%d age=%.3fs",
                              gga_gps_flag, gga_age_sec,
                              imugood_flag ? 1 : 0, imu_age_sec);
        }
    }
}

void android_callback(const std_msgs::String::ConstPtr &msg)
{
    std::string data = msg->data;
    std::cout << "Received data: " << data << std::endl;

    if (data == "init_location")
    {
        restart_imu = true;
    }
}

void reboot_callback(const std_msgs::Bool::ConstPtr &msg)
{
    if (msg->data)
    {
        std::cout << "Reboot signal received, restarting nodes..." << std::endl;
        // 必须后台非阻塞执行: 原命令缺少结尾 '&', 会在 ROS 回调中前台阻塞等待
        // rosrun 永不返回 -> 卡死 spinOnce, 整个节点停摆。
        if (std::system("nohup restart_ros_nodes.sh /serial_reader_node -- "
                        "bash -c 'source /home/nvidia/libo/1ant/devel/setup.bash && "
                        "rosrun serial_reader read_serial' > /dev/null 2>&1 &") != 0)
        {
            ROS_ERROR("Failed to launch the node: %s", "/serial_reader_node");
        }
    }
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "serial_reader_node");
    ros::NodeHandle nh;
    PubPosition pubPosition;
    bool imugood_flag = false;
    int gga_gps_flag = 0;
    const std::string android_topic = "/signal";
    const std::string reboot_topic = "/nanobot/reboot";
    const std::string serial_port = "/dev/ttyTHS1";
    const uint32_t baud = 115200;

    // 串口采用默认构造 + 循环内打开: 设备未就绪时不会因构造抛异常而启动即崩溃
    serial::Serial serial;
    ros::Publisher pub = nh.advertise<util::GpsPosition>("/nanobot/gpsposition", 1);
    ros::Subscriber sub_android = nh.subscribe<std_msgs::String>(android_topic, 1, android_callback);
    ros::Subscriber sub_reboot = nh.subscribe<std_msgs::Bool>(reboot_topic, 1, reboot_callback);

    const double t0 = ros::SteadyTime::now().toSec();
    last_publish_time = ros::Time::now();
    g_last_rx_time.store(t0);
    g_last_pub_time.store(t0);

    // 启动软件看门狗: 串口数据中断时请求重开串口(根因自愈, 无需人工重启进程)
    std::thread wd(watchdogThread);

    while (ros::ok())
    {
        // (1) init_location: 复位 INS 与解析状态。放在串口门控之外, 保证串口卡死时
        //     该自救路径仍可执行(原实现关在 if(serial.available()) 内, 串口一旦卡死即失效)。
        if (restart_imu)
        {
            restart_imu = false;
            ROS_WARN("init_location received -> reset INS & parser state");
            try
            {
                if (serial.isOpen())
                {
                    serial.write(std::string("reset\r\n"));
                    serial.flushInput();
                }
            }
            catch (const std::exception &e)
            {
                ROS_ERROR("write reset to serial failed: %s", e.what());
            }
            resetParserState();
            imugood_flag = false;
            gga_gps_flag = 0;
            g_need_reopen.store(true); // INS 复位后串口可能短暂中断, 主动重连一次
        }

        // (2) 看门狗请求或串口未打开 -> (重新)打开串口
        if (g_need_reopen.load() || !serial.isOpen())
        {
            g_need_reopen.store(false);
            if (!openSerial(serial, serial_port, baud))
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                ros::spinOnce();
                continue;
            }
            // 重连成功后刷新计时, 避免看门狗立即再次触发
            g_last_rx_time.store(ros::SteadyTime::now().toSec());
        }

        // (3) 读取并处理串口数据(整体 try/catch, 异常时关闭串口由 (2) 重连)
        try
        {
            if (serial.waitReadable()) // 阻塞至有数据或超时, 取代忙等空转(降CPU、减UART overrun)
            {
                std::string serial_data = serial.readline();
                // readline() returns after the delimiter or timeout. Capture
                // both clocks immediately, before logging/parsing/publishing.
                // A valid CRC/field layout below proves this was a complete
                // frame; partial timeout returns are rejected by the parser.
                const ros::SteadyTime frame_rx_steady = ros::SteadyTime::now();
                const ros::Time frame_rx_ros = ros::Time::now();
                if (!serial_data.empty())
                {
                    g_last_rx_time.store(frame_rx_steady.toSec());
                }

                printf("time : %14.4f : %s\n", ros::Time::now().toSec(), serial_data.c_str());

                handleSerialLine(serial_data, pubPosition, pub, imugood_flag,
                                 gga_gps_flag, frame_rx_ros,
                                 frame_rx_steady.toSec());
            }
        }
        catch (const serial::IOException &e)
        {
            ROS_ERROR("Serial IO error: %s -> reopen", e.what());
            try { serial.close(); } catch (...) {}
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
        catch (const std::exception &e)
        {
            // 其余多为串口层异常(SerialException/PortNotOpened 等), 关闭以触发 (2) 重连
            ROS_ERROR("Loop exception: %s -> reopen", e.what());
            try { serial.close(); } catch (...) {}
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }

        ros::spinOnce();
    }

    if (wd.joinable())
    {
        wd.join();
    }
    return 0;
}
