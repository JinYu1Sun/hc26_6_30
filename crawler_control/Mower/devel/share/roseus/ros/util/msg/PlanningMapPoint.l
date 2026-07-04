;; Auto-generated. Do not edit!


(when (boundp 'util::PlanningMapPoint)
  (if (not (find-package "UTIL"))
    (make-package "UTIL"))
  (shadow 'PlanningMapPoint (find-package "UTIL")))
(unless (find-package "UTIL::PLANNINGMAPPOINT")
  (make-package "UTIL::PLANNINGMAPPOINT"))

(in-package "ROS")
;;//! \htmlinclude PlanningMapPoint.msg.html
(if (not (find-package "STD_MSGS"))
  (ros::roseus-add-msgs "std_msgs"))


(defclass util::PlanningMapPoint
  :super ros::object
  :slots (_header _global_x _global_y _obstacle_status _track_status ))

(defmethod util::PlanningMapPoint
  (:init
   (&key
    ((:header __header) (instance std_msgs::Header :init))
    ((:global_x __global_x) 0.0)
    ((:global_y __global_y) 0.0)
    ((:obstacle_status __obstacle_status) nil)
    ((:track_status __track_status) nil)
    )
   (send-super :init)
   (setq _header __header)
   (setq _global_x (float __global_x))
   (setq _global_y (float __global_y))
   (setq _obstacle_status __obstacle_status)
   (setq _track_status __track_status)
   self)
  (:header
   (&rest __header)
   (if (keywordp (car __header))
       (send* _header __header)
     (progn
       (if __header (setq _header (car __header)))
       _header)))
  (:global_x
   (&optional __global_x)
   (if __global_x (setq _global_x __global_x)) _global_x)
  (:global_y
   (&optional __global_y)
   (if __global_y (setq _global_y __global_y)) _global_y)
  (:obstacle_status
   (&optional (__obstacle_status :null))
   (if (not (eq __obstacle_status :null)) (setq _obstacle_status __obstacle_status)) _obstacle_status)
  (:track_status
   (&optional (__track_status :null))
   (if (not (eq __track_status :null)) (setq _track_status __track_status)) _track_status)
  (:serialization-length
   ()
   (+
    ;; std_msgs/Header _header
    (send _header :serialization-length)
    ;; float64 _global_x
    8
    ;; float64 _global_y
    8
    ;; bool _obstacle_status
    1
    ;; bool _track_status
    1
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; std_msgs/Header _header
       (send _header :serialize s)
     ;; float64 _global_x
       (sys::poke _global_x (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _global_y
       (sys::poke _global_y (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; bool _obstacle_status
       (if _obstacle_status (write-byte -1 s) (write-byte 0 s))
     ;; bool _track_status
       (if _track_status (write-byte -1 s) (write-byte 0 s))
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; std_msgs/Header _header
     (send _header :deserialize buf ptr-) (incf ptr- (send _header :serialization-length))
   ;; float64 _global_x
     (setq _global_x (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _global_y
     (setq _global_y (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; bool _obstacle_status
     (setq _obstacle_status (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; bool _track_status
     (setq _track_status (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;;
   self)
  )

(setf (get util::PlanningMapPoint :md5sum-) "ba8804762a910657104542279037ab6a")
(setf (get util::PlanningMapPoint :datatype-) "util/PlanningMapPoint")
(setf (get util::PlanningMapPoint :definition-)
      "Header header

# Global_Map_Point
float64 global_x
float64 global_y
bool obstacle_status
bool track_status
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



(provide :util/PlanningMapPoint "ba8804762a910657104542279037ab6a")


