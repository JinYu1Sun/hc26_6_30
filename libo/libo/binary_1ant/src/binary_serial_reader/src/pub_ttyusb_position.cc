#include "ros/ros.h"
#include <iostream>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include "std_msgs/String.h"
int openSerialPort(const char* serial_port){
    int serial_fd = open(serial_port, O_RDWR | O_NOCTTY);

    if (serial_fd == -1) {
        std::cerr << "Error opening serial port" << std::endl;
        return 1;
    }

    struct termios tty;
    tcgetattr(serial_fd, &tty);

    // 设置波特率
    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);

    // 设置控制模式
    tty.c_cflag &= ~PARENB;   // 无奇偶校验
    tty.c_cflag &= ~CSTOPB;   // 1位停止位
    tty.c_cflag &= ~CSIZE;    // 清除数据位设置
    tty.c_cflag |= CS8;       // 8位数据位
    tty.c_cflag |= CREAD;     // 启用接收
    tty.c_cflag |= CLOCAL;    // 忽略调制解调器状态线

    // 设置输入模式
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);  // 禁用软件流控制

    // 设置输出模式
    tty.c_oflag &= ~OPOST;  // 原始输出

    // 读取至少一个字符时返回
    tty.c_cc[VMIN] = 1;

    // 无限超时
    tty.c_cc[VTIME] = 0;

    // 应用设置
    tcsetattr(serial_fd, TCSANOW, &tty);

    return serial_fd;
}

void pub_serial_data(){

}
int main(int argc,char** argv) {
    const char* serial_port = "/dev/ttyUSB0";  // 替换为你的串口设备文件名
    int serial_fd = openSerialPort(serial_port);
    ros::init(argc,argv,"serial_reader");
    std_msgs::String msg;
    while (ros::ok()) {
        char buffer[256];
        ssize_t bytes_read = read(serial_fd, buffer, sizeof(buffer));

        if (bytes_read > 0) {
            // 处理读取到的数据
            for (int i = 0; i < bytes_read; ++i) {
                std::cout << buffer[i];
            }
        }
    }

    close(serial_fd);

    return 0;
}

