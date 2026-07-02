; Auto-generated. Do not edit!


(cl:in-package util-msg)


;//! \htmlinclude PlanningMapPointList.msg.html

(cl:defclass <PlanningMapPointList> (roslisp-msg-protocol:ros-message)
  ((header
    :reader header
    :initarg :header
    :type std_msgs-msg:Header
    :initform (cl:make-instance 'std_msgs-msg:Header))
   (global_point_list
    :reader global_point_list
    :initarg :global_point_list
    :type (cl:vector util-msg:PlanningMapPoint)
   :initform (cl:make-array 0 :element-type 'util-msg:PlanningMapPoint :initial-element (cl:make-instance 'util-msg:PlanningMapPoint))))
)

(cl:defclass PlanningMapPointList (<PlanningMapPointList>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <PlanningMapPointList>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'PlanningMapPointList)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name util-msg:<PlanningMapPointList> is deprecated: use util-msg:PlanningMapPointList instead.")))

(cl:ensure-generic-function 'header-val :lambda-list '(m))
(cl:defmethod header-val ((m <PlanningMapPointList>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:header-val is deprecated.  Use util-msg:header instead.")
  (header m))

(cl:ensure-generic-function 'global_point_list-val :lambda-list '(m))
(cl:defmethod global_point_list-val ((m <PlanningMapPointList>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:global_point_list-val is deprecated.  Use util-msg:global_point_list instead.")
  (global_point_list m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <PlanningMapPointList>) ostream)
  "Serializes a message object of type '<PlanningMapPointList>"
  (roslisp-msg-protocol:serialize (cl:slot-value msg 'header) ostream)
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'global_point_list))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (roslisp-msg-protocol:serialize ele ostream))
   (cl:slot-value msg 'global_point_list))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <PlanningMapPointList>) istream)
  "Deserializes a message object of type '<PlanningMapPointList>"
  (roslisp-msg-protocol:deserialize (cl:slot-value msg 'header) istream)
  (cl:let ((__ros_arr_len 0))
    (cl:setf (cl:ldb (cl:byte 8 0) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) __ros_arr_len) (cl:read-byte istream))
  (cl:setf (cl:slot-value msg 'global_point_list) (cl:make-array __ros_arr_len))
  (cl:let ((vals (cl:slot-value msg 'global_point_list)))
    (cl:dotimes (i __ros_arr_len)
    (cl:setf (cl:aref vals i) (cl:make-instance 'util-msg:PlanningMapPoint))
  (roslisp-msg-protocol:deserialize (cl:aref vals i) istream))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<PlanningMapPointList>)))
  "Returns string type for a message object of type '<PlanningMapPointList>"
  "util/PlanningMapPointList")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'PlanningMapPointList)))
  "Returns string type for a message object of type 'PlanningMapPointList"
  "util/PlanningMapPointList")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<PlanningMapPointList>)))
  "Returns md5sum for a message object of type '<PlanningMapPointList>"
  "e74b8cb2e54b9905429e9e007420b366")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'PlanningMapPointList)))
  "Returns md5sum for a message object of type 'PlanningMapPointList"
  "e74b8cb2e54b9905429e9e007420b366")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<PlanningMapPointList>)))
  "Returns full string definition for message of type '<PlanningMapPointList>"
  (cl:format cl:nil "Header header~%PlanningMapPoint[] global_point_list~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%================================================================================~%MSG: util/PlanningMapPoint~%Header header~%~%# Global_Map_Point~%float64 global_x~%float64 global_y~%bool obstacle_status~%bool track_status~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'PlanningMapPointList)))
  "Returns full string definition for message of type 'PlanningMapPointList"
  (cl:format cl:nil "Header header~%PlanningMapPoint[] global_point_list~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%================================================================================~%MSG: util/PlanningMapPoint~%Header header~%~%# Global_Map_Point~%float64 global_x~%float64 global_y~%bool obstacle_status~%bool track_status~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <PlanningMapPointList>))
  (cl:+ 0
     (roslisp-msg-protocol:serialization-length (cl:slot-value msg 'header))
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'global_point_list) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ (roslisp-msg-protocol:serialization-length ele))))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <PlanningMapPointList>))
  "Converts a ROS message object to a list"
  (cl:list 'PlanningMapPointList
    (cl:cons ':header (header msg))
    (cl:cons ':global_point_list (global_point_list msg))
))
