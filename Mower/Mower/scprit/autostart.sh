#!/bin/bash

# ros_bridge �ŵ���ǰ������location-map�������Ż�pmodʱ��׿�ղ���map name�������ݵ�����
source /home/nvidia/crawler_control/devel/setup.bash && roslaunch rosbridge_server rosbridge_websocket.launch > /dev/null 2>&1 &
sleep 3

# RTK
source /home/nvidia/libo/serial_ws/devel/setup.bash && source /home/nvidia/libo/binary_1ant/devel/setup.bash && rosrun binary_serial_reader read_serial > ~/RTK.log 2>&1 &
#source /home/nvidia/libo/serial_ws/devel/setup.bash && source /home/nvidia/libo/1ant/devel/setup.bash && rosrun serial_reader read_serial > ~/RTK.log 2>&1 &
sleep 3
#source /home/nvidia/libo/binary_1ant/devel/setup.bash && rosrun binary_serial_reader read_serial > /dev/null 2>&1 &
#sleep 2

# Lidar
#source /home/nvidia/mid360/ws_livox/devel/setup.bash && roslaunch livox_ros_driver2 msg_MID360.launch > /dev/null 2>&1 &
#sleep 3

# Pointcloud type transformer
source /home/nvidia/livox2pointcloud2/ws_livox/devel/setup.bash && rosrun livox_to_pointcloud2 livox_to_pointcloud2_node > /dev/null 2>&1 &
sleep 3

# ros_bridge
#source /home/nvidia/crawler_control/devel/setup.bash && roslaunch rosbridge_server rosbridge_websocket.launch > /dev/null 2>&1 &
#sleep 3

# android
source /home/nvidia/libo/lidar-slam-v2/devel/setup.bash && roslaunch android_manager android.launch > /dev/null 2>&1 & 
sleep 2
source /home/nvidia/libo/lidar-slam-v2/devel/setup.bash && roslaunch fast_lio create_mode.launch > /dev/null 2>&1 &
sleep 3

# yolo warning
source /home/nvidia/yys/catkin_yolo/devel/setup.bash && roslaunch yolov8 yolo.launch  > /dev/null 2>&1 &
sleep 3
# yolo warning
source /home/nvidia/yys/catkin_campose/devel/setup.bash && python /home/nvidia/yys/catkin_campose/src/seg_poly.py  > /dev/null 2>&1 &
sleep 3

# Lidar obstacle
source /home/nvidia/zyx/perception_gcj_1010/devel/setup.bash && roslaunch object_perception start_perception.launch > /dev/null 2>&1 &
#source /home/nvidia/zyx/HL_PERCEPTION_FUSION_v1.0/devel/setup.bash && roslaunch object_perception start_perception.launch > /dev/null 2>&1 &
sleep 3

# polygen
source /home/nvidia/Mower/devel/setup.bash && roslaunch polygon_coverage_ros coverage_planner_hl.launch > /home/nvidia/Mower/Log/coverage_planner_hl.log 2>&1 &
sleep 3

# vehicle_diagnostic
source /home/nvidia/Mower/devel/setup.bash && rosrun util vehicle_diagnostic_node  > /dev/null 2>&1 &
sleep 3

# location map
source /home/nvidia/Mower/devel/setup.bash && roslaunch location_map location_map.launch > /home/nvidia/Mower/Log/location_map.log 2>&1 &
sleep 3

# Obstacle.
source /home/nvidia/Mower/devel/setup.bash && roslaunch obstacle_avoidance obstacle_avoidance.launch > /home/nvidia/Mower/Log/obstacle_avoidance.log 2>&1 &
sleep 3

# global planing
source /home/nvidia/Mower/devel/setup.bash && rosrun global_planning global_planning > /home/nvidia/Mower/Log/global_planning.log 2>&1 &
sleep 3

# readway
source /home/nvidia/Mower/devel/setup.bash && rosrun read_waypoints read_waypoints > /home/nvidia/Mower/Log/read_waypoints.log 2>&1 &
sleep 3

# pure pursuit
#source /home/nvidia/crawler_control/devel/setup.bash && rosrun pure_pursuit pure_pursuit > /dev/null 2>&1 &
#sleep 3

# task
source /home/nvidia/crawler_control/devel/setup.bash && rosrun task task > /dev/null 2>&1 &

# ros_bridge �ŵ�������Ż�pmodʱ��׿�ղ���map name�������ݵ�����
#source /home/nvidia/crawler_control/devel/setup.bash && roslaunch rosbridge_server rosbridge_websocket.launch > /dev/null 2>&1 &
#sleep 3


