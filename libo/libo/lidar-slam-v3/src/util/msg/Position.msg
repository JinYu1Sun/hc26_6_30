# 通用定位输出。修改注释不会改变ROS消息MD5；字段、类型和顺序不得随意修改，
# 否则所有util/Position生产者、消费者必须同步重新编译和部署。
Header header

# 位置单位：m；姿态单位：rad。
# 当前fusion_4dof在/Mower/position上发布的是“base_link在地图ENU中的位姿”：
# position_x=East，position_y=North，position_z=Up；roll/pitch/yaw遵循ROS
# base_link约定。为兼容既有下游，header.frame_id当前仍填"base_link"，因此不能
# 仅按frame_id把position_*误解为车体系坐标。
# 旧SE2生产者的地图坐标定义不同；两版并行对照时必须使用不同输出话题，且不能
# 不经坐标转换直接逐点相减。
float64 position_x
float64 position_y
float64 position_z
float64 roll
float64 pitch
float64 yaw

# /Mower/position的position_state取值（调试话题可能复用该字段表达自身布尔状态，
# 不得把调试话题的0/1直接套用以下生产定位语义）：
#   0 = 无有效生产定位/保留值；不得驱动车辆作业
#   1 = GPS/RTK正常：x/y/yaw以GPS/组合导航为主；同步RTK短暂缺帧的配置宽限内，
#       使用“最后可信RTK锚点+LIO相对增量”短时延拓；z使用LIO相对高程+稳健tz
#   2 = 4DoF LIO回退，水平yaw已锁定（经过二维展布充分的鲁棒Umeyama拟合）
#   3 = 纯GPS调试模式（pub_fusion==2），不得等同于正常自动融合状态1
#   4 = 纯LIO调试模式（pub_fusion==3）；只有完整4DoF有效时才具有全局ENU语义
#   5 = 4DoF LIO回退，水平yaw仅播种未锁定（沿迹基线或RTK航向种子）；短时
#       可用，但长直线长时间丢RTK时误差风险高于状态2
#   6 = MID360 IMU受限积分里程计；仅用于制动期位姿连续，
#       不得自动作业，超过时间/距离/数据新鲜度门限立即失效
#   9 = 航向尚未初始化（heading_flag_为假时覆盖输出状态）；不得驱动车辆作业
# 当前生产控制中只有1/2/5属于“有效可用位置”；所有按状态判断定位有效性的
# 消费者都必须显式包含5，并可对状态5采用更保守的速度/作业策略。
uint32 position_state
