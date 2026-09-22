;; Auto-generated. Do not edit!


(when (boundp 'util::Position)
  (if (not (find-package "UTIL"))
    (make-package "UTIL"))
  (shadow 'Position (find-package "UTIL")))
(unless (find-package "UTIL::POSITION")
  (make-package "UTIL::POSITION"))

(in-package "ROS")
;;//! \htmlinclude Position.msg.html
(if (not (find-package "STD_MSGS"))
  (ros::roseus-add-msgs "std_msgs"))


(defclass util::Position
  :super ros::object
  :slots (_header _position_x _position_y _position_z _roll _pitch _yaw _position_state ))

(defmethod util::Position
  (:init
   (&key
    ((:header __header) (instance std_msgs::Header :init))
    ((:position_x __position_x) 0.0)
    ((:position_y __position_y) 0.0)
    ((:position_z __position_z) 0.0)
    ((:roll __roll) 0.0)
    ((:pitch __pitch) 0.0)
    ((:yaw __yaw) 0.0)
    ((:position_state __position_state) 0)
    )
   (send-super :init)
   (setq _header __header)
   (setq _position_x (float __position_x))
   (setq _position_y (float __position_y))
   (setq _position_z (float __position_z))
   (setq _roll (float __roll))
   (setq _pitch (float __pitch))
   (setq _yaw (float __yaw))
   (setq _position_state (round __position_state))
   self)
  (:header
   (&rest __header)
   (if (keywordp (car __header))
       (send* _header __header)
     (progn
       (if __header (setq _header (car __header)))
       _header)))
  (:position_x
   (&optional __position_x)
   (if __position_x (setq _position_x __position_x)) _position_x)
  (:position_y
   (&optional __position_y)
   (if __position_y (setq _position_y __position_y)) _position_y)
  (:position_z
   (&optional __position_z)
   (if __position_z (setq _position_z __position_z)) _position_z)
  (:roll
   (&optional __roll)
   (if __roll (setq _roll __roll)) _roll)
  (:pitch
   (&optional __pitch)
   (if __pitch (setq _pitch __pitch)) _pitch)
  (:yaw
   (&optional __yaw)
   (if __yaw (setq _yaw __yaw)) _yaw)
  (:position_state
   (&optional __position_state)
   (if __position_state (setq _position_state __position_state)) _position_state)
  (:serialization-length
   ()
   (+
    ;; std_msgs/Header _header
    (send _header :serialization-length)
    ;; float64 _position_x
    8
    ;; float64 _position_y
    8
    ;; float64 _position_z
    8
    ;; float64 _roll
    8
    ;; float64 _pitch
    8
    ;; float64 _yaw
    8
    ;; uint32 _position_state
    4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; std_msgs/Header _header
       (send _header :serialize s)
     ;; float64 _position_x
       (sys::poke _position_x (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _position_y
       (sys::poke _position_y (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _position_z
       (sys::poke _position_z (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _roll
       (sys::poke _roll (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _pitch
       (sys::poke _pitch (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _yaw
       (sys::poke _yaw (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; uint32 _position_state
       (write-long _position_state s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; std_msgs/Header _header
     (send _header :deserialize buf ptr-) (incf ptr- (send _header :serialization-length))
   ;; float64 _position_x
     (setq _position_x (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _position_y
     (setq _position_y (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _position_z
     (setq _position_z (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _roll
     (setq _roll (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _pitch
     (setq _pitch (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _yaw
     (setq _yaw (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; uint32 _position_state
     (setq _position_state (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;;
   self)
  )

(setf (get util::Position :md5sum-) "b162b5fae3a9f45d1742394d3c0bb3a6")
(setf (get util::Position :datatype-) "util/Position")
(setf (get util::Position :definition-)
      "# 通用定位输出。修改注释不会改变ROS消息MD5；字段、类型和顺序不得随意修改，
# 否则所有util/Position生产者、消费者必须同步重新编译和部署。
Header header

# 位置单位：m；姿态单位：rad。
# 当前fusion_4dof在/Mower/position上发布的是“base_link在地图ENU中的位姿”：
# position_x=East，position_y=North，position_z=Up；roll/pitch/yaw遵循ROS
# base_link约定。为兼容既有下游，header.frame_id当前仍填\"base_link\"，因此不能
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

================================================================================
MSG: std_msgs/Header
# Standard metadata for higher-level stamped data types.
# This is generally used to communicate timestamped data 
# in a particular coordinate frame.
# 
# sequence ID: consecutively increasing ID 
uint32 seq
#Two-integer timestamp that is expressed as:
# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')
# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')
# time-handling sugar is provided by the client library
time stamp
#Frame this data is associated with
string frame_id

")



(provide :util/Position "b162b5fae3a9f45d1742394d3c0bb3a6")


