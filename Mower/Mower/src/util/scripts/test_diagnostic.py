#!/usr/bin/env python
# -*- coding: utf-8 -*-

import rospy
from util.msg import DiagnosticResult

def diagnostic_callback(msg):
    """处理诊断结果的回调函数"""
    print("=" * 50)
    print("车辆诊断结果:")
    print("时间戳: {}".format(msg.header.stamp))
    print("故障码: {}".format(msg.fault_code))
    print("-" * 30)
    print("详细状态:")
    print("有定位数据: {}".format(msg.has_position))
    print("停车指令: {}".format(msg.stop_car_command))
    print("避障状态: {}".format(msg.avoid_status))
    print("有全局路径: {}".format(msg.has_global_path))
    print("有控制指令: {}".format(msg.has_vehicle_cmd))
    print("CAN缓存满: {}".format(msg.can_buffer_full))
    print("=" * 50)

def main():
    """主函数"""
    rospy.init_node('diagnostic_test_node', anonymous=True)
    
    # 订阅诊断结果话题
    rospy.Subscriber('/vehicle/diagnostic', DiagnosticResult, diagnostic_callback)
    
    print("开始监听车辆诊断结果...")
    print("故障码说明:")
    print("0: 正常运行")
    print("1: 无定位数据")
    print("2: 收到停车指令")
    print("3: 避障紧急停车") 
    print("4: 无全局路径")
    print("5: PID控制节点未运行")
    print("6: CAN通讯缓存已满(兜底检查)")
    print("7: 多种原因混合")
    print("-" * 50)
    
    rospy.spin()

if __name__ == '__main__':
    try:
        main()
    except rospy.ROSInterruptException:
        pass 