#include "pub_position.h"
#include <unordered_map>
#include "util/GpsPosition.h"

std::unordered_map<std::string, int> confidence_map = {
    {"NONE", 0},
    {"SINGLE", 1},
    {"PSRDIFF", 2},
    {"NARROW_FLOAT", 3},
    {"NARROW_INT", 4},
    {"INS", 5},
    {"INS_RTKFIXED", 6}};

PubPosition::PubPosition()
{
    // Constructor logic (if any)
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
    // 解析串口数据，提取经纬度信息
    //serial_reader::gpsinfo gpsinfo; // serial_reader::gpsinfo改为util·································
    //util::GpsPosition gpsPosition;
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
        // 在这里根据你的需求提取经纬度信息
        count++;

        if (token.empty())
        {
            continue; // 跳过空字段
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
                // if(token.find("NARROW_INT") != std::string::npos){
                //     confidence = 4;
                // }else {
                //     confidence = 0;
                // }

                // auto it = confidence_map.find(token);
                // if (it != confidence_map.end())
                // {
                //     confidence = it->second;
                // }
                // else
                // {
                //     confidence = -99; // 未知状态
                // }

                if (token.find("NONE") != std::string::npos) // TODO：// 简化
                {
                    confidence = 0;
                }
                else if (token.find("SINGLE") != std::string::npos)
                {
                    confidence = 1;
                }
                else if (token.find("PSRDIFF") != std::string::npos)
                {
                    confidence = 2;
                }
                else if (token.find("NARROW_FLOAT") != std::string::npos)
                {
                    confidence = 3;
                }
                else if (token.find("NARROW_INT") != std::string::npos)
                {
                    confidence = 4;
                }
                else if (token.find("INS") != std::string::npos)
                {
                    confidence = 5;
                }
                else if (token.find("INS_RTKFIXED") != std::string::npos)
                {
                    confidence = 6;
                }
                else
                {
                    confidence = -99;
                }
                break;
            }
        }
    }
    // 在这里进行进一步处理ROS 消息中
    latitude1 = compute_gps(latitude);
    longitude1 = compute_gps(longitude);
    projection.geodeticToGrid(latitude, longitude, gauss_x, gauss_y);
    // std::cout<<latitude  <<longitude  <<gauss_x  <<gauss_y  <<std::endl;
    old_x = gauss_x;
    old_y = gauss_y;
    std::cout << yaw << std::endl;

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

    // std::cout << yaw << std::endl;
    // std::cout << imugood_flag << std::endl;
    if (confidence == 4 && imugood_flag) // imugood_flag == 1 &&
    {
        gpsPosition.header.frame_id = "/gps";
        gpsPosition.header.stamp = ros::Time::now();

        gpsPosition.plat_id = 0;
        gpsPosition.error_code = 0;
        gpsPosition.gps_flag = confidence; //>>>
        gpsPosition.positionStatus = 0;

        gpsPosition.gps_week = 0;
        gpsPosition.gps_millisecond = 0;

        gpsPosition.longitude = longitude;
        gpsPosition.latitude = latitude;
        gpsPosition.height = height;

        gpsPosition.gaussX = gauss_x * 100;
        gpsPosition.gaussY = gauss_y * 100;

        gpsPosition.pitch = 0;
        gpsPosition.roll = 0;
        gpsPosition.azimuth = yaw * 100;

        gpsPosition.acc_x = 0;
        gpsPosition.acc_y = 0;
        gpsPosition.acc_z = 0;

        gpsPosition.rot_x = 0;
        gpsPosition.rot_y = 0;
        gpsPosition.rot_z = 0;

        gpsPosition.northVelocity = 0;
        gpsPosition.eastVelocity = 0;
        gpsPosition.upVelocity = 0;

        /*switch (pGnavp16.gps_status)
        {
        case 4:
            gpsPosition.gps_confidence = 10;
            break;
        case 5:
            gpsPosition.gps_confidence = 8;
            break;
        default:
            gpsPosition.gps_confidence = pGnavp16.gps_status;
        }*/
        gpsPosition.gps_confidence = confidence;

        gpsPosition.INS_GpsFlag_Pos = 0;
        gpsPosition.INS_NumSV = 0;
        gpsPosition.INS_GpsFlag_Heading = 0;
        gpsPosition.INS_Gps_Age = 0;
        gpsPosition.INS_Car_Status = 0;
        gpsPosition.INS_Status = 0;
        gpsPosition.INS_VehicleAlign = 0;
    }
    pub.publish(gpsPosition);
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "serial_reader_node");
    ros::NodeHandle nh;
    PubPosition pubPosition;
    bool imugood_flag = false;
    int gga_gps_flag = 0;

    // 设置串口参数
    serial::Serial serial("/dev/ttyUSB0", 115200, serial::Timeout::simpleTimeout(1000));
    ros::Publisher pub = nh.advertise<util::GpsPosition>("/nanobot/gpsposition", 1);

    while (ros::ok())
    {
        // 读取串口数据
        if (serial.available()) // if (serial.waitReadable())
        {
            std::string serial_data = serial.readline();

            printf("stamp :  %14.4f  ---  %s\n", ros::Time::now().toSec() ,serial_data.c_str());
            // 判断数据是否以'#'符号为开始、是否为DRPVA或IMUATTA消息，否则跳过本次循环
            if (serial_data.empty() || (serial_data[0] != '#' && serial_data[0] != '$') || ((serial_data.find("DRPVA") == std::string::npos) && (serial_data.find("IMUATTA") == std::string::npos) && (serial_data.find("GNGGA") == std::string::npos)))
            {
                // ROS_WARN("--------: %s", serial_data.c_str());//ROS_WARN("Invalid data received: %s", serial_data.c_str());
                continue;
            }
            //printf("%s\n", serial_data.c_str());
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
                        continue; // 跳过空字段
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
            if (serial_data.find("DRPVA") != std::string::npos && gga_gps_flag == 4 && imugood_flag) // TODO：&& imugood_flag == 1;???根据imu未收敛时的定位好坏来判断
            {
                pubPosition.processSerialData(serial_data, pub, imugood_flag);
            }
        }
    }
    ros::spin();
    return 0;
}