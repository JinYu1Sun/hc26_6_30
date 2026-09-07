#!/usr/bin/env python3

import math

import rclpy
from angmen_ad_msgs.srv import PathPlanningRequest
from rclpy.node import Node


class BridgeServiceMocks(Node):
    def __init__(self):
        super().__init__("mqtt_bridge_service_mocks")
        self.create_service(
            PathPlanningRequest,
            "/mqtt_bridge/path_planning_service",
            self.on_path,
        )
        self.get_logger().info("路径规划测试服务已启动")

    @staticmethod
    def generate_road_points(longitude, latitude):
        route_longitude = []
        route_latitude = []

        for segment in range(len(longitude) - 1):
            start_lon = longitude[segment]
            start_lat = latitude[segment]
            end_lon = longitude[segment + 1]
            end_lat = latitude[segment + 1]
            mean_latitude = math.radians((start_lat + end_lat) * 0.5)
            meters_per_latitude = 111320.0
            meters_per_longitude = max(1.0, 111320.0 * math.cos(mean_latitude))
            delta_x = (end_lon - start_lon) * meters_per_longitude
            delta_y = (end_lat - start_lat) * meters_per_latitude
            distance = math.hypot(delta_x, delta_y)
            if distance < 0.1:
                continue

            samples = max(12, min(80, math.ceil(distance / 5.0)))
            curve_amplitude = min(12.0, max(2.0, distance * 0.08))
            curve_direction = 1.0 if segment % 2 == 0 else -1.0
            perpendicular_x = -delta_y / distance
            perpendicular_y = delta_x / distance

            for step in range(samples):
                ratio = step / samples
                smooth_ratio = ratio * ratio * (3.0 - 2.0 * ratio)
                lateral_offset = (
                    curve_direction
                    * curve_amplitude
                    * math.sin(math.pi * ratio)
                )
                base_lon = start_lon + (end_lon - start_lon) * smooth_ratio
                base_lat = start_lat + (end_lat - start_lat) * smooth_ratio
                route_longitude.append(
                    base_lon + perpendicular_x * lateral_offset / meters_per_longitude
                )
                route_latitude.append(
                    base_lat + perpendicular_y * lateral_offset / meters_per_latitude
                )

        if longitude:
            route_longitude.append(longitude[-1])
            route_latitude.append(latitude[-1])
        return route_longitude, route_latitude

    def on_path(self, request, response):
        longitude = list(request.longitude)
        latitude = list(request.latitude)
        self.get_logger().info(
            "收到路径规划请求: "
            f"longitude={longitude}, latitude={latitude}"
        )
        valid = len(longitude) >= 2 and len(longitude) == len(latitude)
        response.timestamp = request.timestamp
        if valid:
            route_longitude, route_latitude = self.generate_road_points(
                longitude, latitude
            )
            valid = len(route_longitude) >= 2
        else:
            route_longitude, route_latitude = [], []

        response.response = 1 if valid else 0
        response.route_longitude = route_longitude if valid else []
        response.route_latitude = route_latitude if valid else []
        self.get_logger().info(
            f"路径规划模拟完成: 输入点={len(longitude)}, 输出道路点={len(route_longitude)}"
        )
        return response


def main(args=None):
    rclpy.init(args=args)
    node = BridgeServiceMocks()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == "__main__":
    main()
