#include "location_map.h"

int main(int argc, char **argv)
{
    setlocale(LC_ALL,"");
    ros::init(argc, argv, "location_map_node");
    ROS_INFO("START LOAD VSLAM LOCATION");
    LocationMap location_map;

    ros::spin();
    return 0;
}
