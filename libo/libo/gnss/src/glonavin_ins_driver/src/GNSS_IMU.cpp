#include <ros/ros.h>
#include <unistd.h>
#include <sstream>
#include <stdio.h>
#include <math.h>
#include <ctime>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sys/time.h>
#include <unistd.h>
#include <thread>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/asio/deadline_timer.hpp>

#include <Eigen/Geometry>
#include <Eigen/Core>
#include <deque>
#include <sensor_msgs/Imu.h>
#include <tf/tf.h>
#include "AHRS.h"
#include <sensor_msgs/NavSatFix.h>
#include <sensor_msgs/NavSatStatus.h>
#include <util/GpsPosition.h>
#include <util/LocalPose.h>
#include <util/VehicleReport.h>
#include "gauss-kruger-cpp/gausskruger.h"

using namespace boost::lambda;
using namespace std;
#define math_pi 3.14159265358979323846264338327950288419716939937510

unsigned short check_sum16(unsigned char *ptr, unsigned short len)
{
  unsigned short i;
  unsigned short res_val = 0;
  if (!ptr)
    return 0;
  for (i = 0; i < len; i++)
  {
    res_val += ptr[i];
  }
  return res_val;
}

util::VehicleReport vehicle_report;

void vehicleReportCallback(const util::VehicleReport::ConstPtr &msg)
{
  vehicle_report = *msg;
}
int main(int argc, char **argv)
{
  using namespace boost::asio;
  string sFrameDefault = "GNW_IMU";
  string sTopicDefault = "/imu";
  string sPortDefault = "/dev/ttyTHS1";
  string gnTopicDefault = "/gnss";
  string gpsTopicDefault = "/nanobot/gpsposition";
  int nBaudRateDefault = 115200;

  int nBaudRate = 115200;
  std::string sPort = "/dev/ttyTHS1";
  std::string sTopic;
  std::string sFrame;
  std::string gnTopic;
  std::string gpsTopic;

  ros::init(argc, argv, "GNW_GPS_IMU");
  ros::NodeHandle n_private_("~");
  ros::NodeHandle nh;
  n_private_.param("frame", sFrame, sFrameDefault);
  n_private_.param("topic", sTopic, sTopicDefault);
  n_private_.param("port", sPort, sPortDefault);
  n_private_.param("baudrate", nBaudRate, nBaudRateDefault);
  n_private_.param("gnTopic", gnTopic, gnTopicDefault);
  n_private_.param("gpsTopic", gpsTopic, gpsTopicDefault);

  ros::Publisher navsat_fix_pub_;
  ros::Publisher Publisher;
  ros::Publisher util_gpsposition_publisher;
  ros::Publisher util_localpose_publisher;
  ros::Subscriber vehicle_report_subscriber;
  double back_lattitue = 0;
  double back_longitude = 0;
  Eigen::Vector3f acc_ = Eigen::Vector3f::Zero(3);

  //std::cout<<"11111111111111111"<<std::endl;
  try
  {
    //Publisher = nh.advertise<sensor_msgs::Imu>(sTopic, 1000);
    //navsat_fix_pub_ = nh.advertise<sensor_msgs::NavSatFix>(gnTopic, 1000);
    util_gpsposition_publisher = nh.advertise<util::GpsPosition>(gpsTopic, 1);
    util_localpose_publisher = nh.advertise<util::LocalPose>("/nanobot/localpose", 1000);
    //vehicle_report_subscriber = nh.subscribe<util::VehicleReport>("/nanobot/vehicle_report", 100, vehicleReportCallback);
    io_service io;
    serial_port port(io, sPort);
    port.set_option(serial_port_base::baud_rate(nBaudRate));
    port.set_option(serial_port_base::character_size(8));
    port.set_option(serial_port_base::flow_control(serial_port_base::flow_control::none));
    port.set_option(serial_port_base::parity(serial_port_base::parity::none));
    port.set_option(serial_port_base::stop_bits(serial_port_base::stop_bits::one));
    boost::array<unsigned char, 2000> DataBuf;
    //std::cout<<"2222222222"<<std::endl;
    int nCnt = 0;
    // static char chrBuffer[2000]={0};
    std::deque<char> chrBuffer;
    char chrTemp[DATA_LEN] = {0};
    bool align = false;
    GNAVP16_t pGnavp16;
    util::GpsPosition gps_msg;
    util::LocalPose localpose_msg;
    double dr_heading = 0.5 * M_PI;
    ros::Time t_old, t_new;
    t_old = ros::Time::now();
    ros::AsyncSpinner spinner(1);
    spinner.start();
    ros::Rate rate(102);
    
    int epoch = 0;
    float vehicle_speed_now = 0;
    float vspeed[5] = { 0 };
    float vspeed_average = 0;
    int overthd = 0;
    double acc = 0.0;
    //std::cout<<"33333333333"<<std::endl;
    while (ros::ok())
    {
      //std::cout<<"44444444"<<std::endl;
      try
      {
        // std::cout<<"55555"<<std::endl;
        size_t usLength = boost::asio::read(port, boost::asio::buffer(DataBuf),
                                            boost::asio::transfer_at_least(1));
        //std::cout<<"usLength "<<usLength<<std::endl;
	      //std::cout<<"6666666"<<std::endl;
        boost::array<unsigned char, 2000>::iterator itr = DataBuf.begin(); // 缓存起始
        for (size_t i = 0; i < usLength; ++i)
        {
          chrBuffer.push_back(*itr);
          itr++;
        }

        if (chrBuffer.size() < DATA_LEN)
          continue;

        while (!chrBuffer.empty()) // 数据位对齐
        {
          align = false;
          if (chrBuffer.size() < DATA_LEN)
            break;
          if ((chrBuffer.at(0) & 0xff) != HEAD_00)
          {
            chrBuffer.pop_front();
            continue;
          }
          if (chrBuffer.size() < DATA_LEN)
            break;
          if ((chrBuffer.at(1) & 0xff) != 0xCE)
          {
            chrBuffer.pop_front();
            chrBuffer.pop_front();
            continue;
          }
          if (chrBuffer.size() < DATA_LEN)
            break;
          if ((chrBuffer.at(2) & 0xff) != 0x3A)
          {
            chrBuffer.pop_front();
            chrBuffer.pop_front();
            chrBuffer.pop_front();
            continue;
          }
          if (chrBuffer.size() < DATA_LEN)
            break;
          if ((chrBuffer.at(3) & 0xff) != 0x55)
          {
            chrBuffer.pop_front();
            chrBuffer.pop_front();
            chrBuffer.pop_front();
            chrBuffer.pop_front();
            continue;
          }
          if (chrBuffer.size() < DATA_LEN)
            break;
          if (chrBuffer.size() >= DATA_LEN)
          {
            for (int i = 0; i < DATA_LEN; i++)
            {
              // chrTemp[i] = chrBuffer.at(i);
              chrTemp[i] = chrBuffer.front();
              chrBuffer.pop_front();
            }
            memcpy(&pGnavp16, &chrTemp[0], DATA_LEN);
            unsigned short recvCheckSum = pGnavp16.check_sum;
            unsigned short calcCheckSum = check_sum16((unsigned char *)chrTemp, DATA_LEN - 2);
            if (recvCheckSum != calcCheckSum)
            {
              align = false;
              ROS_WARN("checksum error:%d!=%d", recvCheckSum, calcCheckSum);
              break;
            }
            else
            {
              // chrBuffer.erase(chrBuffer.begin(),chrBuffer.begin()+93);
              align = true;
              break;
            }
          }
        }

        if (align)
        {
          float roll, pitch, yaw;

          // roll  = pGnavp16.pitch;
          // pitch = pGnavp16.roll;
          // yaw  = -pGnavp16.yaw;
          roll = (pGnavp16.pitch / 180.0) * math_pi;
          pitch = (pGnavp16.roll / 180.0) * math_pi;
          yaw = -(pGnavp16.yaw / 180.0) * math_pi;
          //geometry_msgs::Quaternion quat = tf::createQuaternionMsgFromRollPitchYaw(roll, pitch, yaw);

          // geometry_msgs::Vector3 angular_velocity;
          // angular_velocity.x = (pGnavp16.gyrox) * M_PI / 180;
          // angular_velocity.y = (pGnavp16.gyroy) * M_PI / 180;
          // angular_velocity.z = (pGnavp16.gyroz) * M_PI / 180;

          // geometry_msgs::Vector3 linear_acceleration;
          // linear_acceleration.x = pGnavp16.accx;
          // linear_acceleration.y = pGnavp16.accy;
          // linear_acceleration.z = pGnavp16.accz;

          // sensor_msgs::Imu ImuData;
          // ImuData.header.stamp = ros::Time::now();

          // ImuData.header.frame_id = sFrame;
          // ImuData.orientation = quat;
          // ImuData.angular_velocity = angular_velocity;
          // ImuData.linear_acceleration = linear_acceleration;

          // ImuData.orientation_covariance[0] = pGnavp16.roll;
          // ImuData.orientation_covariance[1] = pGnavp16.pitch;
          // ImuData.orientation_covariance[2] = pGnavp16.yaw;

          // ImuData.orientation_covariance[3] = pGnavp16.Ve;
          // ImuData.orientation_covariance[4] = pGnavp16.Vn;
          // ImuData.orientation_covariance[5] = pGnavp16.Vu;

          // // ImuData.orientation_covariance[6] = pGnavp16.vehicle_speed;

          // ImuData.header.seq = nCnt;

          // Publisher.publish(ImuData);

          // sensor_msgs::NavSatStatus Nstatus; // 差分信号信息

          // if (pGnavp16.status == 0)
          // {
          //   Nstatus.status = -1;
          // }
          // else
          // {
          //   Nstatus.status = pGnavp16.status - 1; // 0.单点定位 1.RTD 3.RTK固定解 4.RTK浮点解
          // }
          // Nstatus.service = 4;
          // sensor_msgs::NavSatFix satdata;
          // satdata.header.stamp = ImuData.header.stamp;
          // satdata.header.frame_id = sFrame;
          // satdata.header.seq = nCnt;
          // satdata.status = Nstatus;
          // satdata.latitude = pGnavp16.latitude;
          // satdata.longitude = pGnavp16.longitude;
          // satdata.altitude = pGnavp16.altitude;
          // // satdata.position_covariance[0] = (float)pGnavp16.hour;
          // // satdata.position_covariance[1] = (float)pGnavp16.min;
          // // satdata.position_covariance[2] = (float)pGnavp16.sec;
          // // satdata.position_covariance[3] = (float)pGnavp16.msec;

          // satdata.position_covariance[4] = (float)pGnavp16.vehicle_speed;
          // satdata.position_covariance[5] = (float)pGnavp16.baseline;
          // satdata.position_covariance[6] = (float)pGnavp16.speed_status; // 非零表示有车速信息
          // satdata.position_covariance[7] = (float)pGnavp16.Nsv1;
          // satdata.position_covariance[8] = (float)pGnavp16.Nsv2;
          // navsat_fix_pub_.publish(satdata);
          ++nCnt;

          gps_msg.header.frame_id = "/gps";
          //gps_msg.header.stamp = ros::Time::now();

          gps_msg.gps_flag = pGnavp16.gps_status;
          gps_msg.positionStatus = pGnavp16.status;
          gps_msg.gps_week = 0;
          gps_msg.gps_millisecond = 0;

          gps_msg.longitude = pGnavp16.longitude;
          gps_msg.latitude = pGnavp16.latitude;
          gps_msg.height = pGnavp16.altitude;

          gausskruger::WGS84Projection gauss_projection;
          gauss_projection.geodeticToGrid(pGnavp16.latitude, pGnavp16.longitude,
                                          gps_msg.gaussX, gps_msg.gaussY);
          gps_msg.gaussX *= 100;
          gps_msg.gaussY *= 100;

          gps_msg.pitch = pGnavp16.pitch * 100.0;
          gps_msg.roll = pGnavp16.roll * 100.0;
          double azimuth = ((-1) * pGnavp16.yaw + 90.0f);
          if (azimuth < 0)
            azimuth = azimuth + 360.0f;
          gps_msg.azimuth = azimuth * 100;

          gps_msg.acc_x = pGnavp16.accx;
          gps_msg.acc_y = pGnavp16.accy;
          gps_msg.acc_z = pGnavp16.accz;

          gps_msg.rot_x = pGnavp16.gyrox;
          gps_msg.rot_y = pGnavp16.gyroy;
          gps_msg.rot_z = pGnavp16.gyroz;
          std::cout<<"epoch = "<<epoch<<std::endl;
          std::cout<<"gps_msg.rot_z =  "<<gps_msg.rot_z<<std::endl;

          gps_msg.northVelocity = pGnavp16.Vn * 100.0;// cm/s
          gps_msg.eastVelocity = pGnavp16.Ve * 100.0;
          gps_msg.upVelocity = pGnavp16.Vu * 100.0;

          vehicle_speed_now = sqrt(pow(gps_msg.northVelocity, 2) + pow(gps_msg.eastVelocity, 2));//20241204
          std::cout << "vehicle_speed_now = " << vehicle_speed_now << std::endl;
          //gps_msg.speed_now = vehicle_speed_now;

          
          if (vehicle_speed_now > 2.5 && (vehicle_speed_now - vspeed_average < 20 || epoch == 0 )) {// 参与计算平均速度的条件
              epoch++;
              vspeed_average = 0;
              if (epoch <= 5) {
                  vspeed[epoch - 1] = vehicle_speed_now;//前3个历元
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
                  vspeed_average /= 5;//求3个历元的平均速度
              }
          }
          else {
              if (vehicle_speed_now > 2.5) {
                  overthd += 1;//不符合条件计数
              }
              // else{
              //     epoch = 0;
              //     overthd = 0;
              //     vspeed_average = 0;
              // }
          }
          std::cout << "overthd = " << overthd << std::endl;
          if (overthd > 5) {//计数超过2时，判断为平均速度失效，将当前历元速度赋值为平均速度重新做判断。
              vspeed_average = vehicle_speed_now;
              epoch = 0;
              overthd = 0;//20241206 该句需要加在控制器上
          }


          localpose_msg.vehicle_speed = vspeed_average;

	        //std::cout<<pGnavp16.Vn<<std::endl;//20241204
	        //std::cout<<pGnavp16.Ve<<std::endl;//20241204
	        std::cout<<"localpose_msg.vehicle_speed =  "<<localpose_msg.vehicle_speed<<std::endl<<std::endl;
      	  //std::cout<<"1"<<std::endl;
          switch (pGnavp16.gps_status)
          {
          case 4:
            gps_msg.gps_confidence = 10;
            break;
          case 5:
            gps_msg.gps_confidence = 8;
            break;
          default:
            gps_msg.gps_confidence = pGnavp16.gps_status;
          }

          gps_msg.INS_GpsFlag_Pos = pGnavp16.gps_status;
          gps_msg.INS_NumSV = pGnavp16.Nsv1;
          gps_msg.INS_GpsFlag_Heading = pGnavp16.heading_status;
          gps_msg.INS_Gps_Age = pGnavp16.rtcm_status;
          gps_msg.INS_Car_Status = pGnavp16.vehicle_align;
          gps_msg.INS_Status = pGnavp16.status;
          gps_msg.INS_VehicleAlign = pGnavp16.vehicle_align;
            
          gps_msg.header.stamp = ros::Time::now();
          util_gpsposition_publisher.publish(gps_msg);

          localpose_msg.header.frame_id = "/local_pose";
          localpose_msg.header.stamp = ros::Time::now();
          t_new = ros::Time::now();

          double dt = t_new.toSec() - t_old.toSec();
          t_old = t_new;
          double ds = vehicle_report.vehicle_speed / 3.6 * dt;
          const double deg2rad = M_PI / 180.0;
          double dtheta = pGnavp16.gyroz * deg2rad * dt; // using gyro, rad
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
          //localpose_msg.vehicle_speed = vehicle_report.vehicle_speed / 3.6 * 100.0f; // cm/s 20241206
	        //std::cout<<localpose_msg.vehicle_speed<<std::endl;
          util_localpose_publisher.publish(localpose_msg);
        }
      }
      catch (boost::system::system_error &e)
      {
        if (e.code().message() == "Interrupted system call")
        {
          ros::shutdown();
        }
        else
        {
          cout << "ReadData Error:" << e.what() << endl;
        }
      }
      rate.sleep();
    }
    ros::spinOnce();
  }
  catch (std::exception &e)
  {
    cout << "Run_GPS_IMU Error: " << e.what() << endl;
  }

  // Run_GPS_IMU(sPort, nBaudRate, sTopic, sFrame,gnTopic);

  ros::spin();
  return 0;
}

