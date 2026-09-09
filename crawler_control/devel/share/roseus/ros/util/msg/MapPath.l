;; Auto-generated. Do not edit!


(when (boundp 'util::MapPath)
  (if (not (find-package "UTIL"))
    (make-package "UTIL"))
  (shadow 'MapPath (find-package "UTIL")))
(unless (find-package "UTIL::MAPPATH")
  (make-package "UTIL::MAPPATH"))

(in-package "ROS")
;;//! \htmlinclude MapPath.msg.html
(if (not (find-package "GEOMETRY_MSGS"))
  (ros::roseus-add-msgs "geometry_msgs"))


(defclass util::MapPath
  :super ros::object
  :slots (_path_points ))

(defmethod util::MapPath
  (:init
   (&key
    ((:path_points __path_points) ())
    )
   (send-super :init)
   (setq _path_points __path_points)
   self)
  (:path_points
   (&rest __path_points)
   (if (keywordp (car __path_points))
       (send* _path_points __path_points)
     (progn
       (if __path_points (setq _path_points (car __path_points)))
       _path_points)))
  (:serialization-length
   ()
   (+
    ;; geometry_msgs/Point32[] _path_points
    (apply #'+ (send-all _path_points :serialization-length)) 4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; geometry_msgs/Point32[] _path_points
     (write-long (length _path_points) s)
     (dolist (elem _path_points)
       (send elem :serialize s)
       )
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; geometry_msgs/Point32[] _path_points
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _path_points (let (r) (dotimes (i n) (push (instance geometry_msgs::Point32 :init) r)) r))
     (dolist (elem- _path_points)
     (send elem- :deserialize buf ptr-) (incf ptr- (send elem- :serialization-length))
     ))
   ;;
   self)
  )

(setf (get util::MapPath :md5sum-) "a2e1cc73e7e7fe23e7e24d49c45f4f1a")
(setf (get util::MapPath :datatype-) "util/MapPath")
(setf (get util::MapPath :definition-)
      "#Paths between different maps
geometry_msgs/Point32[] path_points

================================================================================
MSG: geometry_msgs/Point32
# This contains the position of a point in free space(with 32 bits of precision).
# It is recommeded to use Point wherever possible instead of Point32.  
# 
# This recommendation is to promote interoperability.  
#
# This message is designed to take up less space when sending
# lots of points at once, as in the case of a PointCloud.  

float32 x
float32 y
float32 z
")



(provide :util/MapPath "a2e1cc73e7e7fe23e7e24d49c45f4f1a")


