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
catkin_make -DCATKIN_WHITELIST_PACKAGES="location_map"
catkin_make -DCATKIN_WHITELIST_PACKAGES="obstacle_avoidance"
catkin_make -DCATKIN_WHITELIST_PACKAGES="global_planning"
catkin_make -DCATKIN_WHITELIST_PACKAGES="read_waypoints"

### 割草启动流程

1. Android通过"/signal"信号发布"reset"重置割草机状态
2. Android通过"/signal"信号发布"p_mode"获取所有地图名
3. Android通过"/signal"信号发布“use_map/地图名”激活global_planning、read_waypoints节点
4. Android通过"/signal"信号发布"single_map"或"multi_map"，进入单或多地图模式
5. Android通过"/signal"信号发布"start_work"，订阅到位置数据后开始工作
6. Android开启自动导航模式

### 地图录制流程

1. Android通过"/signal"信号发布"reset"重置割草机状态
2. Android通过"/signal"信号发布"start_brd"开始录制割草区域
3. 控制小车绕割草区域一圈
4. Android通过"/signal"信号发布"cease_brd"停止录制割草区域
5. Android通过"/signal"信号发布"start_obs"开始录制障碍物区域
6. 控制小车绕障碍物一圈
7. Android通过"/signal"信号发布"cease_obs"停止录制障碍物区域
8. Android通过"/signal"信号发布"save_map/地图名"保存录制的地图到yaml文件中