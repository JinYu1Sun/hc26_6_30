;; Auto-generated. Do not edit!


(when (boundp 'util::ObstacleList)
  (if (not (find-package "UTIL"))
    (make-package "UTIL"))
  (shadow 'ObstacleList (find-package "UTIL")))
(unless (find-package "UTIL::OBSTACLELIST")
  (make-package "UTIL::OBSTACLELIST"))

(in-package "ROS")
;;//! \htmlinclude ObstacleList.msg.html
(if (not (find-package "STD_MSGS"))
  (ros::roseus-add-msgs "std_msgs"))


(defclass util::ObstacleList
  :super ros::object
  :slots (_header _obstacles ))

(defmethod util::ObstacleList
  (:init
   (&key
    ((:header __header) (instance std_msgs::Header :init))
    ((:obstacles __obstacles) ())
    )
   (send-super :init)
   (setq _header __header)
   (setq _obstacles __obstacles)
   self)
  (:header
   (&rest __header)
   (if (keywordp (car __header))
       (send* _header __header)
     (progn
       (if __header (setq _header (car __header)))
       _header)))
  (:obstacles
   (&rest __obstacles)
   (if (keywordp (car __obstacles))
       (send* _obstacles __obstacles)
     (progn
       (if __obstacles (setq _obstacles (car __obstacles)))
       _obstacles)))
  (:serialization-length
   ()
   (+
    ;; std_msgs/Header _header
    (send _header :serialization-length)
    ;; util/Obstacle[] _obstacles
    (apply #'+ (send-all _obstacles :serialization-length)) 4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; std_msgs/Header _header
       (send _header :serialize s)
     ;; util/Obstacle[] _obstacles
     (write-long (length _obstacles) s)
     (dolist (elem _obstacles)
       (send elem :serialize s)
       )
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; std_msgs/Header _header
     (send _header :deserialize buf ptr-) (incf ptr- (send _header :serialization-length))
   ;; util/Obstacle[] _obstacles
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _obstacles (let (r) (dotimes (i n) (push (instance util::Obstacle :init) r)) r))
     (dolist (elem- _obstacles)
     (send elem- :deserialize buf ptr-) (incf ptr- (send elem- :serialization-length))
     ))
   ;;
   self)
  )

(setf (get util::ObstacleList :md5sum-) "ed84bcdb508e0ed6125f51a05240a67b")
(setf (get util::ObstacleList :datatype-) "util/ObstacleList")
(setf (get util::ObstacleList :definition-)
      "Header header
Obstacle[] obstacles

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

================================================================================
MSG: util/Obstacle
Header header

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

")



(provide :util/ObstacleList "ed84bcdb508e0ed6125f51a05240a67b")


