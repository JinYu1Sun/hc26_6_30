;; Auto-generated. Do not edit!


(when (boundp 'util::Obstacle)
  (if (not (find-package "UTIL"))
    (make-package "UTIL"))
  (shadow 'Obstacle (find-package "UTIL")))
(unless (find-package "UTIL::OBSTACLE")
  (make-package "UTIL::OBSTACLE"))

(in-package "ROS")
;;//! \htmlinclude Obstacle.msg.html
(if (not (find-package "STD_MSGS"))
  (ros::roseus-add-msgs "std_msgs"))


(defclass util::Obstacle
  :super ros::object
  :slots (_header _local_x _local_y _global_x _global_y _x1 _y1 _x2 _y2 _x3 _y3 _x4 _y4 _is_dynamic ))

(defmethod util::Obstacle
  (:init
   (&key
    ((:header __header) (instance std_msgs::Header :init))
    ((:local_x __local_x) 0.0)
    ((:local_y __local_y) 0.0)
    ((:global_x __global_x) 0.0)
    ((:global_y __global_y) 0.0)
    ((:x1 __x1) 0.0)
    ((:y1 __y1) 0.0)
    ((:x2 __x2) 0.0)
    ((:y2 __y2) 0.0)
    ((:x3 __x3) 0.0)
    ((:y3 __y3) 0.0)
    ((:x4 __x4) 0.0)
    ((:y4 __y4) 0.0)
    ((:is_dynamic __is_dynamic) nil)
    )
   (send-super :init)
   (setq _header __header)
   (setq _local_x (float __local_x))
   (setq _local_y (float __local_y))
   (setq _global_x (float __global_x))
   (setq _global_y (float __global_y))
   (setq _x1 (float __x1))
   (setq _y1 (float __y1))
   (setq _x2 (float __x2))
   (setq _y2 (float __y2))
   (setq _x3 (float __x3))
   (setq _y3 (float __y3))
   (setq _x4 (float __x4))
   (setq _y4 (float __y4))
   (setq _is_dynamic __is_dynamic)
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
  (:global_x
   (&optional __global_x)
   (if __global_x (setq _global_x __global_x)) _global_x)
  (:global_y
   (&optional __global_y)
   (if __global_y (setq _global_y __global_y)) _global_y)
  (:x1
   (&optional __x1)
   (if __x1 (setq _x1 __x1)) _x1)
  (:y1
   (&optional __y1)
   (if __y1 (setq _y1 __y1)) _y1)
  (:x2
   (&optional __x2)
   (if __x2 (setq _x2 __x2)) _x2)
  (:y2
   (&optional __y2)
   (if __y2 (setq _y2 __y2)) _y2)
  (:x3
   (&optional __x3)
   (if __x3 (setq _x3 __x3)) _x3)
  (:y3
   (&optional __y3)
   (if __y3 (setq _y3 __y3)) _y3)
  (:x4
   (&optional __x4)
   (if __x4 (setq _x4 __x4)) _x4)
  (:y4
   (&optional __y4)
   (if __y4 (setq _y4 __y4)) _y4)
  (:is_dynamic
   (&optional (__is_dynamic :null))
   (if (not (eq __is_dynamic :null)) (setq _is_dynamic __is_dynamic)) _is_dynamic)
  (:serialization-length
   ()
   (+
    ;; std_msgs/Header _header
    (send _header :serialization-length)
    ;; float64 _local_x
    8
    ;; float64 _local_y
    8
    ;; float64 _global_x
    8
    ;; float64 _global_y
    8
    ;; float32 _x1
    4
    ;; float32 _y1
    4
    ;; float32 _x2
    4
    ;; float32 _y2
    4
    ;; float32 _x3
    4
    ;; float32 _y3
    4
    ;; float32 _x4
    4
    ;; float32 _y4
    4
    ;; bool _is_dynamic
    1
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
     ;; float64 _global_x
       (sys::poke _global_x (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _global_y
       (sys::poke _global_y (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float32 _x1
       (sys::poke _x1 (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _y1
       (sys::poke _y1 (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _x2
       (sys::poke _x2 (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _y2
       (sys::poke _y2 (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _x3
       (sys::poke _x3 (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _y3
       (sys::poke _y3 (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _x4
       (sys::poke _x4 (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _y4
       (sys::poke _y4 (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; bool _is_dynamic
       (if _is_dynamic (write-byte -1 s) (write-byte 0 s))
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
   ;; float64 _global_x
     (setq _global_x (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _global_y
     (setq _global_y (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float32 _x1
     (setq _x1 (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _y1
     (setq _y1 (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _x2
     (setq _x2 (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _y2
     (setq _y2 (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _x3
     (setq _x3 (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _y3
     (setq _y3 (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _x4
     (setq _x4 (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _y4
     (setq _y4 (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; bool _is_dynamic
     (setq _is_dynamic (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;;
   self)
  )

(setf (get util::Obstacle :md5sum-) "9cfcfb9d59a900ca50b3ef55c2eb450b")
(setf (get util::Obstacle :datatype-) "util/Obstacle")
(setf (get util::Obstacle :definition-)
      "Header header

# 2D Box
float64 local_x # m, in map
float64 local_y # m, in map
float64 global_x
float64 global_y
float32 x1 # x_min
float32 y1 # y_max
float32 x2 # x_max
float32 y2 # y_max
float32 x3 # x_max
float32 y3 # y_min
float32 x4 # x_min
float32 y4 # y_min
bool is_dynamic # is_dynamic

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



(provide :util/Obstacle "9cfcfb9d59a900ca50b3ef55c2eb450b")


