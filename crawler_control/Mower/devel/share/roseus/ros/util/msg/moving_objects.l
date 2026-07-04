;; Auto-generated. Do not edit!


(when (boundp 'util::moving_objects)
  (if (not (find-package "UTIL"))
    (make-package "UTIL"))
  (shadow 'moving_objects (find-package "UTIL")))
(unless (find-package "UTIL::MOVING_OBJECTS")
  (make-package "UTIL::MOVING_OBJECTS"))

(in-package "ROS")
;;//! \htmlinclude moving_objects.msg.html
(if (not (find-package "STD_MSGS"))
  (ros::roseus-add-msgs "std_msgs"))


(defclass util::moving_objects
  :super ros::object
  :slots (_header _num _moving_obj ))

(defmethod util::moving_objects
  (:init
   (&key
    ((:header __header) (instance std_msgs::Header :init))
    ((:num __num) 0)
    ((:moving_obj __moving_obj) ())
    )
   (send-super :init)
   (setq _header __header)
   (setq _num (round __num))
   (setq _moving_obj __moving_obj)
   self)
  (:header
   (&rest __header)
   (if (keywordp (car __header))
       (send* _header __header)
     (progn
       (if __header (setq _header (car __header)))
       _header)))
  (:num
   (&optional __num)
   (if __num (setq _num __num)) _num)
  (:moving_obj
   (&rest __moving_obj)
   (if (keywordp (car __moving_obj))
       (send* _moving_obj __moving_obj)
     (progn
       (if __moving_obj (setq _moving_obj (car __moving_obj)))
       _moving_obj)))
  (:serialization-length
   ()
   (+
    ;; std_msgs/Header _header
    (send _header :serialization-length)
    ;; int32 _num
    4
    ;; util/moving_object[] _moving_obj
    (apply #'+ (send-all _moving_obj :serialization-length)) 4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; std_msgs/Header _header
       (send _header :serialize s)
     ;; int32 _num
       (write-long _num s)
     ;; util/moving_object[] _moving_obj
     (write-long (length _moving_obj) s)
     (dolist (elem _moving_obj)
       (send elem :serialize s)
       )
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; std_msgs/Header _header
     (send _header :deserialize buf ptr-) (incf ptr- (send _header :serialization-length))
   ;; int32 _num
     (setq _num (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; util/moving_object[] _moving_obj
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _moving_obj (let (r) (dotimes (i n) (push (instance util::moving_object :init) r)) r))
     (dolist (elem- _moving_obj)
     (send elem- :deserialize buf ptr-) (incf ptr- (send elem- :serialization-length))
     ))
   ;;
   self)
  )

(setf (get util::moving_objects :md5sum-) "3c433c9a0e5b7d198518fcd1e6805968")
(setf (get util::moving_objects :datatype-) "util/moving_objects")
(setf (get util::moving_objects :definition-)
      "Header header
int32 num
moving_object[] moving_obj

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
MSG: util/moving_object
#relative position, m
float32 x1 # x_min
float32 y1 # y_min
float32 x2 # x_min
float32 y2 # y_max
float32 x3 # x_max
float32 y3 # y_max
float32 x4 # x_max
float32 y4 # y_min

# height, cm
float32 zmin

float32 zmax

# absolute speed, m/s
float32 speed

# deg
float32 speed_direction

uint32 id
string label

# center point coordinate in localpose frame, m
float32 global_x
float32 global_y

float32 local_x
float32 local_y
")



(provide :util/moving_objects "3c433c9a0e5b7d198518fcd1e6805968")


