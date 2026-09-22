#include "pub_position.h"
#include "util/GpsPosition.h"
#include <ros/ros.h>
#include <std_msgs/Empty.h>
#include <cstdlib>
#include <ctime>

ros::Time last_publish_time;

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

void PubPosition::processSerialData(const std::string &serial_data, ros::Publisher &pub, bool imugood_flag)
{
    util::GpsPosition gpsPosition;
    std::istringstream iss(serial_data);
    std::string token;
    int count = 0;
    gausskruger::WGS84Projection projection;
    double latitude, longitude, latitude1, longitude1, height, gauss_x, gauss_y, yaw, sig_yaw;
    int confidence = 0;
    std::cout << std::fixed << std::setprecision(10);

    while (getline(iss, token, ','))
    {
        count++;

        if (token.empty())
        {
            continue;
        }
        else
        {
            switch (count)
            {
            case 19:
                latitude = std::stod(token);
                break;
            case 20:
                longitude = std::stod(token);
                break;
            case 21:
                height = std::stod(token);
                break;
            case 32:
                yaw = std::stod(token);
                break;
            case 35:
                sig_yaw = std::stod(token);
                break;
            case 11:
                confidence = (token.find("NARROW_INT") != std::string::npos) ? 4 : 0;
                break;
            default:
                break;
            }
        }
    }
    // 在这里进行进一步处理ROS 消息中
    latitude1 = compute_gps(latitude);
    longitude1 = compute_gps(longitude);
    projection.geodeticToGrid(latitude, longitude, gauss_x, gauss_y);

    old_x = gauss_x;
    old_y = gauss_y;

    // 航向角转换
    yaw = 90 - yaw;
    if (yaw < 0)
    {
        yaw += 360;
    }
    else if (yaw > 360)
    {
        yaw -= 360;
    }

    if (confidence == 4 && imugood_flag)
    {
        gpsPosition.header.frame_id = "/gps";
        gpsPosition.header.stamp = ros::Time::now();

        gpsPosition.gps_flag = confidence;

        gpsPosition.longitude = longitude;
        gpsPosition.latitude = latitude;
        gpsPosition.height = height;

        gpsPosition.gaussX = gauss_x * 100;
        gpsPosition.gaussY = gauss_y * 100;

        gpsPosition.azimuth = yaw * 100;

        gpsPosition.gps_confidence = confidence;
    }
    pub.publish(gpsPosition);
    last_publish_time = ros::Time::now();
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "serial_reader_node");
    ros::NodeHandle nh;
    PubPosition pubPosition;
    bool imugood_flag = false;
    int gga_gps_flag = 0;

    // 设置串口参数
    serial::Serial serial("/dev/ttyTHS1", 115200, serial::Timeout::simpleTimeout(1000));
    ros::Publisher pub = nh.advertise<util::GpsPosition>("/nanobot/gpsposition", 1);

    last_publish_time = ros::Time::now();

    while (ros::ok())
    {
        if ((ros::Time::now() - last_publish_time).toSec() > 5.0)
        {
            ROS_WARN("No gpsPosition published in the last 5 seconds. Restarting node...");
            // 重启节点
            int ret = std::system("bash -c 'source ~/libo/1ant/devel/setup.bash && rosnode kill /serial_reader_node && sleep 2 && rosrun serial_reader read_serial >> /dev/null 2>&1 &'");

            if (ret != 0)
            {
                ROS_ERROR("Failed to execute restart node: %s", "rosnode kill /serial_reader_node && sleep 2 && rosrun serial_reader read_serial");
            }
        }

        // 读取串口数据
        if (serial.available()) // if (serial.waitReadable())
        {
            std::string serial_data = serial.readline();
            // 判断数据是否以'#'符号为开始、是否为DRPVA或IMUATTA消息，否则跳过本次循环
            if (serial_data.empty() || (serial_data[0] != '#' && serial_data[0] != '$') || ((serial_data.find("DRPVA") == std::string::npos) && (serial_data.find("IMUATTA") == std::string::npos) && (serial_data.find("GNGGA") == std::string::npos)))
            {
                continue;
            }
            if (serial_data.find("GNGGA") != std::string::npos)
            {
                std::istringstream gga(serial_data);
                std::string token;
                int count = 0;
                while (getline(gga, token, ','))
                {
                    count++;
                    if (token.empty())
                    {
                        continue;
                    }
                    else
                    {
                        if (count == 7)
                        {
                            gga_gps_flag = std::stoi(token);
                        }
                    }
                }
                continue;
            }

            // 判断IMUATTA消息中INS解状态是否为GOOD状态
            if (serial_data.find("IMUATTA") != std::string::npos)
            {
                if (serial_data.find("INS_SOLUTION_GOOD") != std::string::npos)
                {
                    imugood_flag = true;
                }
                else
                {
                    imugood_flag = false;
                }
                continue;
            }
            // 若INS解为GOOD状态，则处理串口数据并发布ROS消息
            if (serial_data.find("DRPVA") != std::string::npos && gga_gps_flag == 4 && imugood_flag)
            {
                pubPosition.processSerialData(serial_data, pub, imugood_flag);
            }
        }
    }
    ros::spin();
    return 0;
}