#!/usr/bin/env python3

import argparse
import time
import uuid

import rclpy
from angmen_ad_msgs.msg import CarEvent
from rclpy.node import Node


def parse_args():
    parser = argparse.ArgumentParser(description="向ROS话题发布车辆事件")
    parser.add_argument("--topic", default="/vehicle/car_event")
    parser.add_argument("--level", type=int, choices=range(5))
    parser.add_argument("--event-type", type=int, default=0)
    parser.add_argument("--description", default="车云联调测试事件")
    parser.add_argument("--longitude", type=float, default=112.1)
    parser.add_argument("--latitude", type=float, default=28.1)
    parser.add_argument("--mission-id", default="00000000-0000-0000-0000-000000000000")
    parser.add_argument("--count", type=int, default=2)
    parser.add_argument("--interval", type=float, default=0.5)
    return parser.parse_known_args()


class CarEventPublisher(Node):
    def __init__(self, topic):
        super().__init__("car_event_test_publisher")
        self.publisher = self.create_publisher(CarEvent, topic, 10)

    def publish_event(self, args, level, sequence):
        message = CarEvent()
        message.ver = 0
        message.event_level = level
        message.event_type = args.event_type
        message.event_description = f"{args.description} level={level} seq={sequence}"
        message.event_time = time.time()
        message.event_longitude = args.longitude
        message.event_latitude = args.latitude
        message.event_mission_id = list(uuid.UUID(args.mission_id).bytes)
        self.publisher.publish(message)
        self.get_logger().info(
            f"已发布事件: level={level}, type={args.event_type}, seq={sequence}"
        )


def main():
    args, ros_args = parse_args()
    if args.count < 1:
        raise SystemExit("--count 必须大于 0")

    rclpy.init(args=ros_args)
    node = CarEventPublisher(args.topic)
    levels = [args.level] if args.level is not None else list(range(5))

    try:
        time.sleep(1.0)
        for level in levels:
            for sequence in range(1, args.count + 1):
                node.publish_event(args, level, sequence)
                rclpy.spin_once(node, timeout_sec=0.05)
                time.sleep(args.interval)
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()
