#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
ROS障碍物发布器 - 发布 util::ObstacleList 消息类型
用于测试和调试避障功能
"""

import rospy
from std_msgs.msg import Header
from util.msg import Obstacle, ObstacleList
import math
from datetime import datetime


def create_obstacle(local_x, local_y, global_x, global_y, 
                    x1, y1, x2, y2, x3, y3, x4, y4, is_dynamic=False):
    """
    创建一个障碍物对象
    
    参数:
        local_x, local_y: 局部坐标 (地图坐标系)
        global_x, global_y: 全局坐标 (全局坐标系)
        x1-y4: 矩形四个顶点坐标 (按顺序: min_x/max_y, max_x/max_y, max_x/min_y, min_x/min_y)
        is_dynamic: 是否为动态障碍物
    
    返回:
        Obstacle 对象
    """
    obstacle = Obstacle()
    obstacle.header = Header()
    obstacle.header.stamp = rospy.Time.now()
    obstacle.header.frame_id = "map"
    
    obstacle.local_x = local_x
    obstacle.local_y = local_y
    obstacle.global_x = global_x
    obstacle.global_y = global_y
    
    # 矩形四个顶点
    obstacle.x1 = x1
    obstacle.y1 = y1
    obstacle.x2 = x2
    obstacle.y2 = y2
    obstacle.x3 = x3
    obstacle.y3 = y3
    obstacle.x4 = x4
    obstacle.y4 = y4
    
    obstacle.is_dynamic = is_dynamic
    
    return obstacle


def main():
    # 初始化ROS节点
    rospy.init_node('obstacle_publisher', anonymous=True)
    
    # 获取发布频率参数
    publish_rate = rospy.get_param('~publish_rate', 10)  # 默认10Hz
    
    # 创建发布器
    pub = rospy.Publisher('/mower_perception/obstacles', ObstacleList, queue_size=10, latch=True)
    
    # 创建发布速率控制器
    rate = rospy.Rate(publish_rate)
    
    rospy.loginfo("Obstacle Publisher started, publishing at %.1f Hz", publish_rate)
    rospy.loginfo("Publishing on topic: /mower_perception/obstacles")
    
    # 发布循环
    counter = 1
    count = 50
    while not rospy.is_shutdown() and count > 0:
        count += 1
        try:
            # 创建 ObstacleList 消息
            obstacle_list = ObstacleList()
            obstacle_list.header = Header()
            obstacle_list.header.stamp = rospy.Time.now()
            obstacle_list.header.frame_id = "map"
            obstacle_list.header.seq = counter
            
            # 示例 1: 静态障碍物（固定位置）
            """ obs1 = create_obstacle(
                local_x=0, local_y=0,
                global_x=4, global_y=8,
                x1=3.9, y1=7.7,
                x2=4.1, y2=7.7,
                x3=4.1, y3=8.3,
                x4=3.9, y4=8.3,
                is_dynamic=False
            )
            obstacle_list.obstacles.append(obs1) """
            
            # 示例 2: 静态障碍物（另一位置）
            """ obs2 = create_obstacle(
                local_x=8.0, local_y=-3.0,
                global_x=9.0, global_y=3.0,
                x1=8.9, y1=2.9,
                x2=9.1, y2=2.9,
                x3=9.1, y3=3.1,
                x4=8.9, y4=3.1,
                is_dynamic=False
            )
            obstacle_list.obstacles.append(obs2) """
            
            # 示例 3: 动态障碍物（随时间移动）
            time_offset = math.sin(rospy.get_time() / 5.0) * 2.0  # 2米范围内振荡
            obs3 = create_obstacle(
                local_x=3.0 + time_offset, local_y=8.0,
                global_x=8.0 + time_offset, global_y=12.0,
                x1=7.5 + time_offset, y1=12.5,
                x2=8.5 + time_offset, y2=12.5,
                x3=8.5 + time_offset, y3=11.5,
                x4=7.5 + time_offset, y4=11.5,
                is_dynamic=True
            )
            obstacle_list.obstacles.append(obs3)
            
            # 发布消息
            pub.publish(obstacle_list)
            
            if counter % 50 == 0:  # 每50帧打印一次（50/10Hz = 5秒）
                rospy.loginfo("Published ObstacleList with %d obstacles (seq=%d)", 
                             len(obstacle_list.obstacles), counter)
            
            counter += 1
            rate.sleep()
            
        except Exception as e:
            rospy.logerr("Error in obstacle publisher: %s", str(e))
            rate.sleep()


if __name__ == '__main__':
    try:
        main()
    except rospy.ROSInterruptException:
        rospy.loginfo("Obstacle Publisher shutting down")
