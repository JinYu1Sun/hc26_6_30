#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
模拟割草机上位机数据，用于测试 cloud_bridge 到公网 MQTT broker 的通路。

发布:
  - /Mower/position      (mower_msgs/Position)
  - /vehicle/status      (mower_msgs/VehicleStatus)

用法:
  rosrun cloud_bridge mock_robot_data.py
  # 或
  python3 crawler_control/src/cloud_bridge/scripts/mock_robot_data.py
"""

import math
import random

import rospy
from mower_msgs.msg import Position, VehicleStatus
from std_msgs.msg import UInt16


def main():
    rospy.init_node("mock_robot_data")

    pub_position = rospy.Publisher("/Mower/position", Position, queue_size=1)
    pub_status = rospy.Publisher("/vehicle/status", VehicleStatus, queue_size=1)
    pub_height = rospy.Publisher("/vehicle/mower_height_to_app", UInt16, queue_size=1)

    rate = rospy.Rate(10)  # 10Hz，高于 cloud_bridge 的 location_hz/status_hz
    t = 0.0

    rospy.loginfo("mock_robot_data started, publishing /Mower/position & /vehicle/status")

    while not rospy.is_shutdown():
        now = rospy.Time.now()

        # 模拟一个绕圈的位置
        pos = Position()
        pos.header.stamp = now
        pos.header.frame_id = "map"
        pos.position_x = 10.0 * math.cos(t * 0.1)
        pos.position_y = 10.0 * math.sin(t * 0.1)
        pos.position_z = 0.0
        pos.roll = 0.0
        pos.pitch = 0.0
        pos.yaw = t * 0.1
        pos.position_state = 1  # 1=定位有效
        pub_position.publish(pos)

        # 模拟车辆状态
        status = VehicleStatus()
        status.header.stamp = now
        status.header.frame_id = ""
        status.battery_soc = 80
        status.warning_state_one = 0
        status.warning_state_two = 0
        pub_status.publish(status)

        # 模拟刀盘高度反馈
        height = UInt16()
        height.data = 6
        pub_height.publish(height)

        t += 0.1
        rate.sleep()


if __name__ == "__main__":
    main()
