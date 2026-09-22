cd /home/nvidia/libo/gnss
source /devel/setup.bash
roslaunch gps_imu gnss_imu.launch > /dev/null 2>&1 &
