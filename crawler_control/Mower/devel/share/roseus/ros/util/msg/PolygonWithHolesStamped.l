;; Auto-generated. Do not edit!


(when (boundp 'util::PolygonWithHolesStamped)
  (if (not (find-package "UTIL"))
    (make-package "UTIL"))
  (shadow 'PolygonWithHolesStamped (find-package "UTIL")))
(unless (find-package "UTIL::POLYGONWITHHOLESSTAMPED")
  (make-package "UTIL::POLYGONWITHHOLESSTAMPED"))

(in-package "ROS")
;;//! \htmlinclude PolygonWithHolesStamped.msg.html
(if (not (find-package "STD_MSGS"))
  (ros::roseus-add-msgs "std_msgs"))


(defclass util::PolygonWithHolesStamped
  :super ros::object
  :slots (_header _polygon ))

(defmethod util::PolygonWithHolesStamped
  (:init
   (&key
    ((:header __header) (instance std_msgs::Header :init))
    ((:polygon __polygon) (instance util::PolygonWithHoles :init))
    )
   (send-super :init)
   (setq _header __header)
   (setq _polygon __polygon)
   self)
  (:header
   (&rest __header)
   (if (keywordp (car __header))
       (send* _header __header)
     (progn
       (if __header (setq _header (car __header)))
       _header)))
  (:polygon
   (&rest __polygon)
   (if (keywordp (car __polygon))
       (send* _polygon __polygon)
     (progn
       (if __polygon (setq _polygon (car __polygon)))
       _polygon)))
  (:serialization-length
   ()
   (+
    ;; std_msgs/Header _header
    (send _header :serialization-length)
    ;; util/PolygonWithHoles _polygon
    (send _polygon :serialization-length)
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; std_msgs/Header _header
       (send _header :serialize s)
     ;; util/PolygonWithHoles _polygon
       (send _polygon :serialize s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; std_msgs/Header _header
     (send _header :deserialize buf ptr-) (incf ptr- (send _header :serialization-length))
   ;; util/PolygonWithHoles _polygon
     (send _polygon :deserialize buf ptr-) (incf ptr- (send _polygon :serialization-length))
   ;;
   self)
  )

(setf (get util::PolygonWithHolesStamped :md5sum-) "11ecbf0df20ddfcc1b6166cf645d78f1")
(setf (get util::PolygonWithHolesStamped :datatype-) "util/PolygonWithHolesStamped")
(setf (get util::PolygonWithHolesStamped :definition-)
      "# A stamped polygon with holes.
std_msgs/Header header
util/PolygonWithHoles polygon

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
MSG: util/PolygonWithHoles
# A message to define a polygon with holes.
geometry_msgs/Polygon hull
geometry_msgs/Polygon[] holes

================================================================================
MSG: geometry_msgs/Polygon
#A specification of a polygon where the first and last points are assumed to be connected
Point32[] points

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



(provide :util/PolygonWithHolesStamped "11ecbf0df20ddfcc1b6166cf645d78f1")


