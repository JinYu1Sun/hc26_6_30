# polygon_coverage_geometry
CGAL algorithms to perform geometric operations on general polygon with holes.
### 编译依赖
sudo apt install mono-complete
sudo apt install libcgal-dev
### 编译顺序：
catkin_make -DCATKIN_WHITELIST_PACKAGES="util"
catkin_make -DCATKIN_WHITELIST_PACKAGES="polygon_coverage_benchmark"
catkin_make -DCATKIN_WHITELIST_PACKAGES="polygon_coverage_solvers"
catkin_make -DCATKIN_WHITELIST_PACKAGES="polygon_coverage_geometry"
catkin_make -DCATKIN_WHITELIST_PACKAGES="polygon_coverage_msgs"
catkin_make -DCATKIN_WHITELIST_PACKAGES="polygon_coverage_planners"
catkin_make -DCATKIN_WHITELIST_PACKAGES="polygon_coverage_ros"
catkin_make -DCATKIN_WHITELIST_PACKAGES="rviz_polygon_tool"