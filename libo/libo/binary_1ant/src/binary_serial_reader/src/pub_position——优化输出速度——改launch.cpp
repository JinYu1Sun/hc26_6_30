
#include "pub_position.h"


PubPosition::PubPosition() {
    // Constructor logic (if any)
    old_x = 1;
    old_y = 1;
}

PubPosition::~PubPosition() {
    // Destructor logic (if any)
}

double PubPosition::compute_gps(double a){
    int integerPart = static_cast<int>(a / 100);
    // 计算剩余部分
    double remainingPart = fmod(a, 100);
    // 将剩余部分除以60得到结果
    double result = integerPart + (remainingPart / 60);
    return result;
}
void PubPosition::processSerialData(const std::string &serial_data, ros::Publisher &pub,int imugood_flag) {
    // 解析串口数据，提取经纬度信息
    //serial_reader::gpsPosition gpsPosition;
    util::GpsPosition gpsPosition;
    std::istringstream iss(serial_data);
    std::string token;
    int count = 0;
    gausskruger::WGS84Projection projection;
    double latitude, longitude, latitude1, longitude1, height, gauss_x, gauss_y, yaw, sig_yaw, Ve, Vn;
    int confidence = 0;
    std::cout<<std::fixed<<std::setprecision(10);
    Ve = Vn = 0;
    int epoch = 0;
    float vehicle_speed_now = 0;
    float vspeed[5] = { 0 };
    float vspeed_average = 0;
    int overthd = 0;

    while (getline(iss, token, ',')) {
        // 在这里根据你的需求提取经纬度信息
        // 假设纬度信息在第3个字段，经度信息在第5个字段
        count++;

        if (count == 19 && !token.empty()) {
            // 处理纬度信息
            latitude = std::stod(token);
            //std::cout << latitude << std::endl;

        }
        else if (count == 20 && !token.empty()) {
            // 处理经度信息
            longitude = std::stod(token);
            //std::cout << longitude << std::endl;

        }
        else if (count == 21 && !token.empty()) {
            // 处理海拔信息
            height = std::stod(token);
           // std::cout << height << std::endl;

        }
        else if (count == 32 && !token.empty())
        {
            yaw = std::stod(token);
            //std::cout << yaw << std::endl;

        }
        else if (count == 35 && !token.empty())
        {
            sig_yaw = std::stod(token);
            std::cout << sig_yaw << std::endl;

        }
        else if (count == 26 && !token.empty())
        {
            Ve = std::stod(token);

        }
        else if (count == 27 && !token.empty())
        {
            Vn = std::stod(token);

        }
        else if (count == 11 && !token.empty())
        {
           // printf("pos type : %s\n", token.c_str());
            //表 0-3 位置或速度类型
            size_t found0 = token.find("NONE");
            size_t found1 = token.find("SINGLE");
            size_t found2 = token.find("PSRDIFF");
            size_t found3 = token.find("NARROW_FLOAT");
            size_t found4 = token.find("NARROW_INT");
            size_t found5 = token.find("INS");
            size_t found6 = token.find("INS_RTKFIXED");

            if (found0 != std::string::npos) {
                confidence = 0;
            }
            else if (found1 != std::string::npos) {
                confidence = 1;
            }
            else if (found2 != std::string::npos) {
                confidence = 2;
            }
            else if (found3 != std::string::npos) {
                confidence = 3;
            }
            else if (found4 != std::string::npos) {
                confidence = 4;
            }
            else if (found5 != std::string::npos) {
                confidence = 5;
            }
            else if (found6 != std::string::npos) {
                confidence = 6;
            }
            else {
                confidence = 99;
            }
        }
        if (token.empty()) {
            continue;  // 跳过空字段
        }
    }
    // 在这里进行进一步处理ROS 消息中
    latitude1 = compute_gps(latitude);
    longitude1 = compute_gps(longitude);
    projection.geodeticToGrid(latitude, longitude, gauss_x, gauss_y);
    //std::cout<<latitude  <<longitude  <<gauss_x  <<gauss_y  <<std::endl;
    old_x = gauss_x;
    old_y = gauss_y;
    std::cout << yaw <<std::endl;
    // 航向角转换   匹配gpsposition
    //yaw = yaw - 90;
    //if (yaw < 0) {
    //    yaw += 360;//gpsposition中航向角以正东为0度
    //}
    //yaw = 360 - yaw;

    yaw = 90 - yaw;
    if (yaw < 0) {
        yaw += 360;
    }
    else if (yaw > 360) {
        yaw -= 360;
    }

    //std::cout << yaw << std::endl;
    //std::cout << imugood_flag << std::endl;
    if (imugood_flag == 1  && confidence == 4) {
        printf("1ant : ok---\n");
   	    gpsPosition.header.frame_id = "/gps";
        gpsPosition.header.stamp = ros::Time::now();

        gpsPosition.plat_id = 0;
        gpsPosition.error_code = 0;
        gpsPosition.gps_flag = confidence;//>>>
        gpsPosition.positionStatus = 0;

        gpsPosition.gps_week = 0;
        gpsPosition.gps_millisecond = 0;

        gpsPosition.longitude = longitude;
        gpsPosition.latitude = latitude;
        gpsPosition.height = height;

        gpsPosition.gaussX = gauss_x * 100;
        gpsPosition.gaussY = gauss_y * 100;

        vehicle_speed_now = sqrt(pow(Ve, 2) + pow(Vn, 2)) * 36;///0;
        gpsPosition.pitch = vehicle_speed_now;//////xuyaozhushi

        std::cout << "epoch = " << epoch << std::endl;
        std::cout << "vehicle_speed_now = " << vehicle_speed_now << std::endl;

        if (vehicle_speed_now > 2.5 && (vehicle_speed_now - vspeed_average < 20 || epoch == 0)) {// 参与计算平均速度的条件
            epoch++;
            vspeed_average = 0;
            if (epoch <= 5) {
                vspeed[epoch - 1] = vehicle_speed_now;//前5个历元
                for (int j = 0; j < epoch; j++) {
                    vspeed_average += vspeed[j];
                }
                vspeed_average /= epoch;
            }
            else {
                for (int i = 0; i < 4; i++) {
                    vspeed[i] = vspeed[i + 1];//递推赋值
                }
                vspeed[4] = vehicle_speed_now;
                for (int j = 0; j < 5; j++) {
                    vspeed_average += vspeed[j];
                }
                vspeed_average /= 5;//求5个历元的平均速度
            }
        }
        else {
            if (vehicle_speed_now > 2.5) {
                overthd += 1;//不符合条件计数
            }
        }
        std::cout << "overthd = " << overthd << std::endl;
        if (overthd > 2) {//计数超过2时，判断为平均速度失效，将当前历元速度赋值为平均速度重新做判断。
            vspeed_average = 0;
            epoch = 0;
            overthd = 0;
        }

        gpsPosition.roll = vspeed_average;
        std::cout << "vspeed_average = " << vspeed_average << std::endl;

        //gpsPosition.roll = 0;//xuyaoquxiaozhushi
        /*double azimuth = ((-1) * pGnavp16.yaw + 90.0f);
        if (azimuth < 0)
            azimuth = azimuth + 360.0f;*/
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
        
        gpsPosition.INS_Std_Heading=sig_yaw;
    }
    
    //std::cout << gpsPosition << std::endl;
    // 发布ROS消息
    pub.publish(gpsPosition);
    
}

int main(int argc, char **argv) {
    ros::init(argc, argv, "serial_reader_node");
    ros::NodeHandle nh;
    PubPosition pubPosition;
    int imugood_flag = 0;

    std::string node_name = ros::this_node::getName();
    std::string sport;
    int baud;
    std::string gps_topic;
    double yawsig_threshold;

    nh.param<std::string>(node_name + "sport", sport, "/dev/ttyUSB0");
    nh.param<int>(node_name + "baud", baud, 115200);
    nh.param<std::string>(node_name + "gps_topic", gps_topic, "/nanobot/gpsposition");
    nh.param<double>(node_name + "yawsig_threshold", yawsig_threshold, -1);


    // 设置串口参数
    serial::Serial serial(sport, baud, serial::Timeout::simpleTimeout(1000));

    // 创建ROS Publisher
    ros::Publisher pub = nh.advertise<util::GpsPosition>(gps_topic, 10);

    ros::Rate loop_rate(10);
    while (ros::ok()) {
        // 读取串口数据
        if (serial.available()) {
            std::string serial_data = serial.readline();
            size_t found1 = serial_data.find("DRPVA");
            size_t found2 = serial_data.find("IMUATTA");
            size_t found3 = serial_data.find("INS_SOLUTION_GOOD");

            // 判断数据是否以'#'符号为开始
            if (serial_data.empty() || serial_data[0] != '#' || ((found1 == std::string::npos) && (found2 == std::string::npos))) {

                // 如果不是以'#'符号为开始的数据、不是DRPVA或IMUATTA消息，跳过本次循环
                //ROS_WARN("--------: %s", serial_data.c_str());//ROS_WARN("Invalid data received: %s", serial_data.c_str());
                continue;
            }

            //printf("%s\n", serial_data.c_str());

            // 判断IMUATTA消息中INS解状态是否为GOOD状态
            if (found2 != std::string::npos ) {
                if (found3 != std::string::npos) {
                    imugood_flag = 1;
                }
                else {
                    imugood_flag = 0;
                }
                continue;
            }
//            std::cout << serial_data << std::endl;
            // 若INS解为GOOD状态，则处理串口数据并发布ROS消息
            pubPosition.processSerialData(serial_data, pub, imugood_flag);
        }
        loop_rate.sleep();
    }
    ros::spin();

    return 0;
}

//size_t found0 = token.find("NONE");
//size_t found1 = token.find("Reserved");
//size_t found2 = token.find("INS");
//size_t found3 = token.find("INS_PSRSP");
//size_t found4 = token.find("INS_PSRDIFF");
//size_t found5 = token.find("INS_RTKFLOAT");
//size_t found6 = token.find("INS_RTKFIXED");

