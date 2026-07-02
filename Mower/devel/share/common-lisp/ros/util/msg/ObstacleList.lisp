; Auto-generated. Do not edit!


(cl:in-package util-msg)


;//! \htmlinclude ObstacleList.msg.html

(cl:defclass <ObstacleList> (roslisp-msg-protocol:ros-message)
  ((header
    :reader header
    :initarg :header
    :type std_msgs-msg:Header
    :initform (cl:make-instance 'std_msgs-msg:Header))
   (obstacles
    :reader obstacles
    :initarg :obstacles
    :type (cl:vector util-msg:Obstacle)
   :initform (cl:make-array 0 :element-type 'util-msg:Obstacle :initial-element (cl:make-instance 'util-msg:Obstacle))))
)

(cl:defclass ObstacleList (<ObstacleList>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <ObstacleList>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'ObstacleList)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name util-msg:<ObstacleList> is deprecated: use util-msg:ObstacleList instead.")))

(cl:ensure-generic-function 'header-val :lambda-list '(m))
(cl:defmethod header-val ((m <ObstacleList>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:header-val is deprecated.  Use util-msg:header instead.")
  (header m))

(cl:ensure-generic-function 'obstacles-val :lambda-list '(m))
(cl:defmethod obstacles-val ((m <ObstacleList>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:obstacles-val is deprecated.  Use util-msg:obstacles instead.")
  (obstacles m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <ObstacleList>) ostream)
  "Serializes a message object of type '<ObstacleList>"
  (roslisp-msg-protocol:serialize (cl:slot-value msg 'header) ostream)
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'obstacles))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (roslisp-msg-protocol:serialize ele ostream))
   (cl:slot-value msg 'obstacles))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <ObstacleList>) istream)
  "Deserializes a message object of type '<ObstacleList>"
  (roslisp-msg-protocol:deserialize (cl:slot-value msg 'header) istream)
  (cl:let ((__ros_arr_len 0))
    (cl:setf (cl:ldb (cl:byte 8 0) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) __ros_arr_len) (cl:read-byte istream))
  (cl:setf (cl:slot-value msg 'obstacles) (cl:make-array __ros_arr_len))
  (cl:let ((vals (cl:slot-value msg 'obstacles)))
    (cl:dotimes (i __ros_arr_len)
    (cl:setf (cl:aref vals i) (cl:make-instance 'util-msg:Obstacle))
  (roslisp-msg-protocol:deserialize (cl:aref vals i) istream))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<ObstacleList>)))
  "Returns string type for a message object of type '<ObstacleList>"
  "util/ObstacleList")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'ObstacleList)))
  "Returns string type for a message object of type 'ObstacleList"
  "util/ObstacleList")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<ObstacleList>)))
  "Returns md5sum for a message object of type '<ObstacleList>"
  "ed84bcdb508e0ed6125f51a05240a67b")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'ObstacleList)))
  "Returns md5sum for a message object of type 'ObstacleList"
  "ed84bcdb508e0ed6125f51a05240a67b")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<ObstacleList>)))
  "Returns full string definition for message of type '<ObstacleList>"
  (cl:format cl:nil "Header header~%Obstacle[] obstacles~%~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%================================================================================~%MSG: util/Obstacle~%Header header~%~%# 2D Box~%float64 local_x # m, in map~%float64 local_y # m, in map~%float64 global_x~%float64 global_y~%float32 x1 # x_min~%float32 y1 # y_max~%float32 x2 # x_max~%float32 y2 # y_max~%float32 x3 # x_max~%float32 y3 # y_min~%float32 x4 # x_min~%float32 y4 # y_min~%bool is_dynamic # is_dynamic~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'ObstacleList)))
  "Returns full string definition for message of type 'ObstacleList"
  (cl:format cl:nil "Header header~%Obstacle[] obstacles~%~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%================================================================================~%MSG: util/Obstacle~%Header header~%~%# 2D Box~%float64 local_x # m, in map~%float64 local_y # m, in map~%float64 global_x~%float64 global_y~%float32 x1 # x_min~%float32 y1 # y_max~%float32 x2 # x_max~%float32 y2 # y_max~%float32 x3 # x_max~%float32 y3 # y_min~%float32 x4 # x_min~%float32 y4 # y_min~%bool is_dynamic # is_dynamic~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <ObstacleList>))
  (cl:+ 0
     (roslisp-msg-protocol:serialization-length (cl:slot-value msg 'header))
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'obstacles) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ (roslisp-msg-protocol:serialization-length ele))))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <ObstacleList>))
  "Converts a ROS message object to a list"
  (cl:list 'ObstacleList
    (cl:cons ':header (header msg))
    (cl:cons ':obstacles (obstacles msg))
))
