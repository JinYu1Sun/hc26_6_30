;; Auto-generated. Do not edit!


(when (boundp 'util::PlanningMapPointList)
  (if (not (find-package "UTIL"))
    (make-package "UTIL"))
  (shadow 'PlanningMapPointList (find-package "UTIL")))
(unless (find-package "UTIL::PLANNINGMAPPOINTLIST")
  (make-package "UTIL::PLANNINGMAPPOINTLIST"))

(in-package "ROS")
;;//! \htmlinclude PlanningMapPointList.msg.html
(if (not (find-package "STD_MSGS"))
  (ros::roseus-add-msgs "std_msgs"))


(defclass util::PlanningMapPointList
  :super ros::object
  :slots (_header _global_point_list ))

(defmethod util::PlanningMapPointList
  (:init
   (&key
    ((:header __header) (instance std_msgs::Header :init))
    ((:global_point_list __global_point_list) ())
    )
   (send-super :init)
   (setq _header __header)
   (setq _global_point_list __global_point_list)
   self)
  (:header
   (&rest __header)
   (if (keywordp (car __header))
       (send* _header __header)
     (progn
       (if __header (setq _header (car __header)))
       _header)))
  (:global_point_list
   (&rest __global_point_list)
   (if (keywordp (car __global_point_list))
       (send* _global_point_list __global_point_list)
     (progn
       (if __global_point_list (setq _global_point_list (car __global_point_list)))
       _global_point_list)))
  (:serialization-length
   ()
   (+
    ;; std_msgs/Header _header
    (send _header :serialization-length)
    ;; util/PlanningMapPoint[] _global_point_list
    (apply #'+ (send-all _global_point_list :serialization-length)) 4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; std_msgs/Header _header
       (send _header :serialize s)
     ;; util/PlanningMapPoint[] _global_point_list
     (write-long (length _global_point_list) s)
     (dolist (elem _global_point_list)
       (send elem :serialize s)
       )
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; std_msgs/Header _header
     (send _header :deserialize buf ptr-) (incf ptr- (send _header :serialization-length))
   ;; util/PlanningMapPoint[] _global_point_list
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _global_point_list (let (r) (dotimes (i n) (push (instance util::PlanningMapPoint :init) r)) r))
     (dolist (elem- _global_point_list)
     (send elem- :deserialize buf ptr-) (incf ptr- (send elem- :serialization-length))
     ))
   ;;
   self)
  )

(setf (get util::PlanningMapPointList :md5sum-) "e74b8cb2e54b9905429e9e007420b366")
(setf (get util::PlanningMapPointList :datatype-) "util/PlanningMapPointList")
(setf (get util::PlanningMapPointList :definition-)
      "Header header
PlanningMapPoint[] global_point_list
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
MSG: util/PlanningMapPoint
Header header

# Global_Map_Point
float64 global_x
float64 global_y
bool obstacle_status
bool track_status
")



(provide :util/PlanningMapPointList "e74b8cb2e54b9905429e9e007420b366")


