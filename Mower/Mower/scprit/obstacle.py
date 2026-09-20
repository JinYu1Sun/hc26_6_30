#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
ROS障碍物发布器 - 发布 util::ObstacleList 消息类型
用于测试和调试避障功能
"""

import rospy
from std_msgs.msg import Header
from geometry_msgs.msg import Point
from util.msg import Obstacle, ObstacleList
import math
from datetime import datetime


def create_costmap_corners():
    """
    视野区域（costmap）四角点，顺序：近左、近右、远右、远左。
    这里我们扩展成一个更大的前视梯形，表示新增的视野区域。

    注意：当前 ObstacleList.msg 只有一个 costmap_corners 数组，
    所以它只能描述一个区域；如果要支持两个独立视野区域，
    必须在消息中增加第二个数组，例如 costmap_corners_2。
    """
    return [
        Point(x=0.0, y=2.5, z=0.0),
        Point(x=0.0, y=-2.5, z=0.0),
        Point(x=12.0, y=-4.5, z=0.0),
        Point(x=12.0, y=6, z=0.0),
    ]


def create_obstacle(local_x, local_y, global_x, global_y, 
                    x1, y1, x2, y2, x3, y3, x4, y4, is_dynamic=False, class_name = 'unknown'):
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
    obstacle.class_name = class_name
    
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

            obstacle_list.costmap_corners.extend(create_costmap_corners())
            
            # 示例 1: 静态障碍物（固定位置）
            obs1 = create_obstacle(
                local_x=10, local_y=10,
                global_x=0.0738, global_y=-7.11,
                x1=-0.373, y1=-6.57,
                x2=0.882, y2=-6.62,
                x3=0.716, y3=-7.64,
                x4=-0.535, y4=-7.6,
                is_dynamic=False,
                class_name = 'rock'
            )
            obstacle_list.obstacles.append(obs1)
            
            # 示例 2: 静态障碍物（另一位置）
            obs2 = create_obstacle(
                local_x=10, local_y=10,
                global_x=-2.046692371368408, global_y=9.550752639770508,
                x1=-2.0484609603881836, y1=9.480050086975098,
                x2=-1.9759907722473145, y2=9.548956871032715,
                x3=-2.044923782348633, y3=9.621455192565918,
                x4=-2.117393970489502, y4=9.5525484085083,
                is_dynamic=False,
                class_name = 'pole'
            )
            # obstacle_list.obstacles.append(obs2)

            obs3 = create_obstacle(
                local_x=10, local_y=10,
                global_x=-2.1948938369750977, global_y=9.624255180358887,
                x1=-2.248833656311035, y1=9.578532218933105,
                x2=-2.149171829223633, y2=9.57031536102295,
                x3=-2.14095401763916, y3=9.669978141784668,
                x4=-2.2406158447265625, y4=9.678194999694824,
                is_dynamic=False,
                class_name = 'pole'
            )
            # obstacle_list.obstacles.append(obs3)

            obs4 = create_obstacle(
                local_x=10, local_y=10,
                global_x=5.398457050323486, global_y=11.245819091796875,
                x1=5.309669017791748, y1=11.239631652832031,
                x2=5.371463298797607, y2=11.161008834838867,
                x3=5.487245082855225, y3=11.252006530761719,
                x4=5.425450801849365, y4=11.330629348754883,
                is_dynamic=False,
                class_name = 'pole'
            )
            # obstacle_list.obstacles.append(obs4)
            
            # 示例 3: 动态障碍物（随时间移动）
            """ time_offset = math.sin(rospy.get_time() / 5.0) * 2.0  # 2米范围内振荡
            obs3 = create_obstacle(
                local_x=3.0 + time_offset, local_y=8.0,
                global_x=8.0 + time_offset, global_y=12.0,
                x1=7.5 + time_offset, y1=12.5,
                x2=8.5 + time_offset, y2=12.5,
                x3=8.5 + time_offset, y3=11.5,
                x4=7.5 + time_offset, y4=11.5,
                is_dynamic=True
            )
            obstacle_list.obstacles.append(obs3) """
            
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
