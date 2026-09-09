;; Auto-generated. Do not edit!


(when (boundp 'mower_msgs::Vslam)
  (if (not (find-package "MOWER_MSGS"))
    (make-package "MOWER_MSGS"))
  (shadow 'Vslam (find-package "MOWER_MSGS")))
(unless (find-package "MOWER_MSGS::VSLAM")
  (make-package "MOWER_MSGS::VSLAM"))

(in-package "ROS")
;;//! \htmlinclude Vslam.msg.html
(if (not (find-package "STD_MSGS"))
  (ros::roseus-add-msgs "std_msgs"))


(defclass mower_msgs::Vslam
  :super ros::object
  :slots (_header _local_x _local_y _local_z _roll _pitch _yaw ))

(defmethod mower_msgs::Vslam
  (:init
   (&key
    ((:header __header) (instance std_msgs::Header :init))
    ((:local_x __local_x) 0.0)
    ((:local_y __local_y) 0.0)
    ((:local_z __local_z) 0.0)
    ((:roll __roll) 0.0)
    ((:pitch __pitch) 0.0)
    ((:yaw __yaw) 0.0)
    )
   (send-super :init)
   (setq _header __header)
   (setq _local_x (float __local_x))
   (setq _local_y (float __local_y))
   (setq _local_z (float __local_z))
   (setq _roll (float __roll))
   (setq _pitch (float __pitch))
   (setq _yaw (float __yaw))
   self)
  (:header
   (&rest __header)
   (if (keywordp (car __header))
       (send* _header __header)
     (progn
       (if __header (setq _header (car __header)))
       _header)))
  (:local_x
   (&optional __local_x)
   (if __local_x (setq _local_x __local_x)) _local_x)
  (:local_y
   (&optional __local_y)
   (if __local_y (setq _local_y __local_y)) _local_y)
  (:local_z
   (&optional __local_z)
   (if __local_z (setq _local_z __local_z)) _local_z)
  (:roll
   (&optional __roll)
   (if __roll (setq _roll __roll)) _roll)
  (:pitch
   (&optional __pitch)
   (if __pitch (setq _pitch __pitch)) _pitch)
  (:yaw
   (&optional __yaw)
   (if __yaw (setq _yaw __yaw)) _yaw)
  (:serialization-length
   ()
   (+
    ;; std_msgs/Header _header
    (send _header :serialization-length)
    ;; float64 _local_x
    8
    ;; float64 _local_y
    8
    ;; float64 _local_z
    8
    ;; float64 _roll
    8
    ;; float64 _pitch
    8
    ;; float64 _yaw
    8
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; std_msgs/Header _header
       (send _header :serialize s)
     ;; float64 _local_x
       (sys::poke _local_x (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _local_y
       (sys::poke _local_y (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _local_z
       (sys::poke _local_z (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _roll
       (sys::poke _roll (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _pitch
       (sys::poke _pitch (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _yaw
       (sys::poke _yaw (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; std_msgs/Header _header
     (send _header :deserialize buf ptr-) (incf ptr- (send _header :serialization-length))
   ;; float64 _local_x
     (setq _local_x (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _local_y
     (setq _local_y (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _local_z
     (setq _local_z (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _roll
     (setq _roll (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _pitch
     (setq _pitch (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _yaw
     (setq _yaw (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;;
   self)
  )

(setf (get mower_msgs::Vslam :md5sum-) "26eeab7b068d8475f66980c5b18e1bc1")
(setf (get mower_msgs::Vslam :datatype-) "mower_msgs/Vslam")
(setf (get mower_msgs::Vslam :definition-)
      "# VSLAM Position
Header header

float64 local_x
float64 local_y
float64 local_z

float64 roll
float64 pitch
float64 yaw 
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



(provide :mower_msgs/Vslam "26eeab7b068d8475f66980c5b18e1bc1")


