#coding:utf-8
#!/usr/bin/env python

import rospy
import matplotlib.pyplot as plt
from util.msg import GpsPosition  # 注意这里的包名和消息名要与你的实际情况相匹配

# 初始化ROS节点
rospy.init_node('gps_position_plotter', anonymous=True)

# 订阅/nanobot/gpsposition话题
def callback(data):
    # 将数据添加到列表中
    global gaussX_list, gaussY_list
    gaussX_list.append(data.gaussX)
    gaussY_list.append(data.gaussY)

    # 可选：实时绘制图形（注意，这可能会导致性能问题）
    # plt.clf()  # 清除当前图形
    # plt.plot(gaussX_list, gaussY_list, marker='o')
    # plt.draw()
    # plt.pause(0.001)  # 暂停以更新图形

# 初始化存储数据的列表
gaussX_list = []
gaussY_list = []

# 设置订阅者
rospy.Subscriber('/nanobot/gpsposition', GpsPosition, callback)

# 保持节点运行，直到用户中断（Ctrl+C）
rospy.spin()

# 绘制最终图形
plt.plot(gaussX_list, gaussY_list, marker='o')
plt.xlabel('Gauss X')
plt.ylabel('Gauss Y')
plt.title('GPS Position Trajectory')
plt.grid(True)
plt.show()
