#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
cd "$SCRIPT_DIR"

if [ ! -d "src" ]; then
  echo "错误：当前目录不是 catkin 工作区根目录(需要包含 src 目录)" >&2
  exit 1
fi

# ROS_SETUP="/opt/ros/noetic/setup.bash"
# if [ ! -f "$ROS_SETUP" ]; then
#   echo "错误：未找到 ROS 环境文件 $ROS_SETUP" >&2
#   exit 1
# fi

# source "$ROS_SETUP"

echo "开始按指定顺序编译："

build_steps=(
  "util"
  "polygon_coverage_benchmark"
  "polygon_coverage_solvers"
  "polygon_coverage_geometry"
  "polygon_coverage_msgs"
  "polygon_coverage_planners"
  "polygon_coverage_ros"
  "rviz_polygon_tool"
  "location_map"
  "obstacle_avoidance"
  "global_planning"
  "read_waypoints"
)

for pkg in "${build_steps[@]}"; do
  echo "\n===== 编译 $pkg ====="
  catkin_make -DCATKIN_WHITELIST_PACKAGES="$pkg"
done

echo "全部编译完成!"
