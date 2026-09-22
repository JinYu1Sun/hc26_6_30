;; Auto-generated. Do not edit!


(when (boundp 'util::LIOPose)
  (if (not (find-package "UTIL"))
    (make-package "UTIL"))
  (shadow 'LIOPose (find-package "UTIL")))
(unless (find-package "UTIL::LIOPOSE")
  (make-package "UTIL::LIOPOSE"))

(in-package "ROS")
;;//! \htmlinclude LIOPose.msg.html
(if (not (find-package "STD_MSGS"))
  (ros::roseus-add-msgs "std_msgs"))


(defclass util::LIOPose
  :super ros::object
  :slots (_header _position_x _position_y _position_z _q_w _q_x _q_y _q_z _lio_state _imu_speed _imt_static ))

(defmethod util::LIOPose
  (:init
   (&key
    ((:header __header) (instance std_msgs::Header :init))
    ((:position_x __position_x) 0.0)
    ((:position_y __position_y) 0.0)
    ((:position_z __position_z) 0.0)
    ((:q_w __q_w) 0.0)
    ((:q_x __q_x) 0.0)
    ((:q_y __q_y) 0.0)
    ((:q_z __q_z) 0.0)
    ((:lio_state __lio_state) nil)
    ((:imu_speed __imu_speed) 0.0)
    ((:imt_static __imt_static) nil)
    )
   (send-super :init)
   (setq _header __header)
   (setq _position_x (float __position_x))
   (setq _position_y (float __position_y))
   (setq _position_z (float __position_z))
   (setq _q_w (float __q_w))
   (setq _q_x (float __q_x))
   (setq _q_y (float __q_y))
   (setq _q_z (float __q_z))
   (setq _lio_state __lio_state)
   (setq _imu_speed (float __imu_speed))
   (setq _imt_static __imt_static)
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
  (:q_w
   (&optional __q_w)
   (if __q_w (setq _q_w __q_w)) _q_w)
  (:q_x
   (&optional __q_x)
   (if __q_x (setq _q_x __q_x)) _q_x)
  (:q_y
   (&optional __q_y)
   (if __q_y (setq _q_y __q_y)) _q_y)
  (:q_z
   (&optional __q_z)
   (if __q_z (setq _q_z __q_z)) _q_z)
  (:lio_state
   (&optional (__lio_state :null))
   (if (not (eq __lio_state :null)) (setq _lio_state __lio_state)) _lio_state)
  (:imu_speed
   (&optional __imu_speed)
   (if __imu_speed (setq _imu_speed __imu_speed)) _imu_speed)
  (:imt_static
   (&optional (__imt_static :null))
   (if (not (eq __imt_static :null)) (setq _imt_static __imt_static)) _imt_static)
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
    ;; float64 _q_w
    8
    ;; float64 _q_x
    8
    ;; float64 _q_y
    8
    ;; float64 _q_z
    8
    ;; bool _lio_state
    1
    ;; float64 _imu_speed
    8
    ;; bool _imt_static
    1
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
     ;; float64 _q_w
       (sys::poke _q_w (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _q_x
       (sys::poke _q_x (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _q_y
       (sys::poke _q_y (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _q_z
       (sys::poke _q_z (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; bool _lio_state
       (if _lio_state (write-byte -1 s) (write-byte 0 s))
     ;; float64 _imu_speed
       (sys::poke _imu_speed (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; bool _imt_static
       (if _imt_static (write-byte -1 s) (write-byte 0 s))
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
   ;; float64 _q_w
     (setq _q_w (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _q_x
     (setq _q_x (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _q_y
     (setq _q_y (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _q_z
     (setq _q_z (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; bool _lio_state
     (setq _lio_state (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; float64 _imu_speed
     (setq _imu_speed (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; bool _imt_static
     (setq _imt_static (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;;
   self)
  )

(setf (get util::LIOPose :md5sum-) "cc6fba92d9eec750f98200ca1300a109")
(setf (get util::LIOPose :datatype-) "util/LIOPose")
(setf (get util::LIOPose :definition-)
      "# Lidar slam msg
Header header
float64 position_x
float64 position_y
float64 position_z
float64 q_w
float64 q_x
float64 q_y
float64 q_z
bool  lio_state
float64 imu_speed
bool imt_static

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



(provide :util/LIOPose "cc6fba92d9eec750f98200ca1300a109")


