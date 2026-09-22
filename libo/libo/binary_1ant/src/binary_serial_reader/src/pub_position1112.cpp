
#include "pub_position.h"

using namespace std;
#define math_pi 3.14159265358979323846264338327950288419716939937510

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
    serial_reader::gpsPosition gpsPosition;
    std::istringstream iss(serial_data);
    std::string token;
    int count = 0;
    gausskruger::WGS84Projection projection;
    double latitude, longitude, height, gauss_x, gauss_y, yaw, sig_yaw;
    int confidence = 0;
    std::cout<<std::fixed<<std::setprecision(10);

    while (getline(iss, token, ',')) {
        // 在这里根据你的需求提取经纬度信息
        // 假设纬度信息在第3个字段，经度信息在第5个字段
        count++;

        if (count == 19 && !token.empty()) {
            // 处理纬度信息
            latitude = std::stod(token);
            std::cout << latitude << std::endl;

        }
        else if (count == 20 && !token.empty()) {
            // 处理经度信息
            longitude = std::stod(token);
            std::cout << longitude << std::endl;

        }
        else if (count == 21 && !token.empty()) {
            // 处理海拔信息
            height = std::stod(token);
            std::cout << height << std::endl;

        }
        else if (count == 32 && !token.empty())
        {
            yaw = std::stod(token);
            std::cout << yaw << std::endl;

        }
        else if (count == 35 && !token.empty())
        {
            sig_yaw = std::stod(token);
            std::cout << sig_yaw << std::endl;

        }
        else if (count == 11 && !token.empty())
        {
            printf("pos type : %s\n", token.c_str());
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
    //latitude = compute_gps(latitude);
    //longitude = compute_gps(longitude);
    projection.geodeticToGrid(latitude, longitude, gauss_x, gauss_y);
    //std::cout<<latitude<<longitude<<gauss_x<<gauss_y<<std::endl;
    old_x = gauss_x;
    old_y = gauss_y;

    // 航向角转换   匹配gpsposition
    yaw = yaw - 90;
    if (yaw < 0) {
        yaw += 360;//gpsposition中航向角以正东为0度
    }

    std::cout << sig_yaw << std::endl;
    std::cout << imugood_flag << std::endl;
    if (imugood_flag == 1  && confidence == 4) { //&& sig_yaw <= 1
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
        gpsPosition.gaussY = gauss_x * 100;

        gpsPosition.pitch = 0;
        gpsPosition.roll = 0;
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
    }
    
    std::cout << gpsPosition << std::endl;
    // 发布ROS消息
    pub.publish(gpsPosition);
    
}

int main(int argc, char **argv) {
    ros::init(argc, argv, "serial_reader_node");
    ros::NodeHandle nh;
    PubPosition pubPosition;

    ros::Publisher navsat_fix_pub_;
    ros::Publisher Publisher;
    //ros::Publisher util_gpsposition_publisher;
    ros::Publisher util_localpose_publisher;
    ros::Subscriber vehicle_report_subscriber;
    Eigen::Vector3f acc_ = Eigen::Vector3f::Zero(3);

    int imugood_flag = 0;

    // 设置串口参数
    serial::Serial serial("/dev/ttyTHS1", 115200, serial::Timeout::simpleTimeout(1000));

    // 创建ROS Publisher
    ros::Publisher pub = nh.advertise<serial_reader::gpsPosition>("gps_info", 10);

    ros::Rate loop_rate(10);
    


    try {
        Publisher = nh.advertise<sensor_msgs::Imu>("/imu", 1000);
        navsat_fix_pub_ = nh.advertise<sensor_msgs::NavSatFix>("/gnss", 1000);
        util_gpsposition_publisher = nh.advertise<util::GpsPosition>("/nanobot/gpsposition", 1000);
        util_localpose_publisher = nh.advertise<util::LocalPose>("/nanobot/localpose", 1000);
        vehicle_report_subscriber = nh.subscribe<util::VehicleReport>("/nanobot/vehicle_report", 100, vehicleReportCallback);
        int nCnt = 0;

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
                    ROS_WARN("--------: %s", serial_data.c_str());//ROS_WARN("Invalid data received: %s", serial_data.c_str());
                    continue;
                }

                printf("%s\n", serial_data.c_str());

                // 判断IMUATTA消息中INS解状态是否为GOOD状态
                if (found2 != std::string::npos) {
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
            //        loop_rate.sleep();

            float roll, pitch, yaw;

            // roll  = pGnavp16.pitch;
            // pitch = pGnavp16.roll;
            // yaw  = -pGnavp16.yaw;
            roll = 0;// (pGnavp16.pitch / 180.0)* math_pi;
            pitch = 0;//(pGnavp16.roll / 180.0) * math_pi;
            yaw = 0;//-(pGnavp16.yaw / 180.0) * math_pi;
            geometry_msgs::Quaternion quat = tf::createQuaternionMsgFromRollPitchYaw(roll, pitch, yaw);

            geometry_msgs::Vector3 angular_velocity;
            angular_velocity.x = 0;//(pGnavp16.gyrox) * M_PI / 180;
            angular_velocity.y = 0;//(pGnavp16.gyroy) * M_PI / 180;
            angular_velocity.z = 0;//(pGnavp16.gyroz) * M_PI / 180;

            geometry_msgs::Vector3 linear_acceleration;
            linear_acceleration.x = 0;//pGnavp16.accx;
            linear_acceleration.y = 0;//pGnavp16.accy;
            linear_acceleration.z = 0;//pGnavp16.accz;

            sensor_msgs::Imu ImuData;
            ImuData.header.stamp = ros::Time::now();

            ImuData.header.frame_id = "UM981_IMU";
            ImuData.orientation = quat;
            ImuData.angular_velocity = angular_velocity;
            ImuData.linear_acceleration = linear_acceleration;

            ImuData.orientation_covariance[0] = 0;//pGnavp16.roll;
            ImuData.orientation_covariance[1] = 0;//pGnavp16.pitch;
            ImuData.orientation_covariance[2] = 0;//pGnavp16.yaw;

            ImuData.orientation_covariance[3] = 0;//pGnavp16.Ve;
            ImuData.orientation_covariance[4] = 0;//pGnavp16.Vn;
            ImuData.orientation_covariance[5] = 0;//pGnavp16.Vu;

            // ImuData.orientation_covariance[6] = pGnavp16.vehicle_speed;

            ImuData.header.seq = nCnt;

            Publisher.publish(ImuData);

            sensor_msgs::NavSatStatus Nstatus; // 差分信号信息

            if (pGnavp16.status == 0)///
            {
                Nstatus.status = -1;
            }
            else///
            {
                Nstatus.status = pGnavp16.status - 1; // 0.单点定位 1.RTD 3.RTK固定解 4.RTK浮点解
            }
            Nstatus.service = 4;
            sensor_msgs::NavSatFix satdata;
            satdata.header.stamp = ImuData.header.stamp;
            satdata.header.frame_id = "UM981_IMU";
            satdata.header.seq = nCnt;
            satdata.status = Nstatus;
            satdata.latitude = 0;// pGnavp16.latitude;
            satdata.longitude = 0;// pGnavp16.longitude;
            satdata.altitude = 0;// pGnavp16.altitude;
            // satdata.position_covariance[0] = (float)pGnavp16.hour;
            // satdata.position_covariance[1] = (float)pGnavp16.min;
            // satdata.position_covariance[2] = (float)pGnavp16.sec;
            // satdata.position_covariance[3] = (float)pGnavp16.msec;

            satdata.position_covariance[4] = 0;//(float)pGnavp16.vehicle_speed;
            satdata.position_covariance[5] = 0;//(float)pGnavp16.baseline;
            satdata.position_covariance[6] = 0;//(float)pGnavp16.speed_status; // 非零表示有车速信息
            satdata.position_covariance[7] = 0;//(float)pGnavp16.Nsv1;
            satdata.position_covariance[8] = 0;//(float)pGnavp16.Nsv2;
            navsat_fix_pub_.publish(satdata);
            ++nCnt;

            //gpsPosition
            //.....
            //gpsPosition

            localpose_msg.header.frame_id = "/local_pose";
            localpose_msg.header.stamp = ros::Time::now();
            t_new = ros::Time::now();

            double dt = t_new.toSec() - t_old.toSec();
            t_old = t_new;
            double ds = 0;// vehicle_report.vehicle_speed / 3.6 * dt;
            const double deg2rad = M_PI / 180.0;
            double dtheta = 0;// pGnavp16.gyroz* deg2rad* dt; // using gyro, rad
            if (ds == 0.0f)
                dtheta = 0.0f;

            dr_heading += dtheta;
            localpose_msg.dr_x += ds * cos(dr_heading);
            localpose_msg.dr_y += ds * sin(dr_heading);

            if (dr_heading > 2 * M_PI)
            {
                dr_heading -= (2 * M_PI);
            }
            if (dr_heading < 0)
            {
                dr_heading += (2 * M_PI);
            }
            localpose_msg.dr_heading = dr_heading / M_PI * 180.0f;                     // deg
            localpose_msg.vehicle_speed = 0;// vehicle_report.vehicle_speed / 3.6 * 100.0f; // cm/s

            util_localpose_publisher.publish(localpose_msg);
        }
    }
    catch (std::exception& e)
    {
        cout << "Run pub_position Error: " << e.what() << endl;
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

