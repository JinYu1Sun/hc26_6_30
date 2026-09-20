Header header
Obstacle[] obstacles # 障碍物

# 可信感知区（costmap）四角点，map 坐标系，顺序：近左/近右/远右/近左（docs/costmap_region_plan.md）
# 区域 = [雷达前 costmap_near_x, roi_x_max] × 相机真实视野角围成的梯形，随车辆移动
geometry_msgs/Point[] costmap_corners
