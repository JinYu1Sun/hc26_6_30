#include <ros/ros.h>
#include <pure_pursuit/pure_pursuit.h>

int main(int argc, char** argv)
{
    setlocale(LC_ALL,"");
    ros::init(argc, argv, "pure_pursuit");
    ros::NodeHandle nh;
    PurePursuit pp(nh);
    return 0;
}
