#!/usr/bin/env python3

import argparse
import time

import rclpy
from angmen_ad_msgs.msg import TakeoverRequest
from rclpy.node import Node


def parse_args():
    parser = argparse.ArgumentParser(description="向ROS话题发布车辆主动接管请求")
    parser.add_argument("--topic", default="/vehicle/takeover_request")
    parser.add_argument("--request", type=int, choices=(0, 1, 2), default=0)
    parser.add_argument("--reason", type=int, choices=range(7), default=1)
    parser.add_argument("--reason-description", default="车云联调主动接管测试")
    return parser.parse_known_args()


def main():
    args, ros_args = parse_args()
    rclpy.init(args=ros_args)
    node = Node("takeover_request_test_publisher")
    publisher = node.create_publisher(TakeoverRequest, args.topic, 10)

    try:
        time.sleep(1.0)
        message = TakeoverRequest()
        message.timestamp = time.time()
        message.request = args.request
        message.reason = args.reason
        message.reason_description = args.reason_description
        publisher.publish(message)
        node.get_logger().info(
            f"主动接管请求已发布到ROS: request={args.request}, reason={args.reason}"
        )
        rclpy.spin_once(node, timeout_sec=0.5)
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()
