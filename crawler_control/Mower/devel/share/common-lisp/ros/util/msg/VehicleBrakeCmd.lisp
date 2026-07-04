; Auto-generated. Do not edit!


(cl:in-package util-msg)


;//! \htmlinclude VehicleBrakeCmd.msg.html

(cl:defclass <VehicleBrakeCmd> (roslisp-msg-protocol:ros-message)
  ((header
    :reader header
    :initarg :header
    :type std_msgs-msg:Header
    :initform (cl:make-instance 'std_msgs-msg:Header))
   (BrakeCmd
    :reader BrakeCmd
    :initarg :BrakeCmd
    :type cl:boolean
    :initform cl:nil))
)

(cl:defclass VehicleBrakeCmd (<VehicleBrakeCmd>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <VehicleBrakeCmd>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'VehicleBrakeCmd)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name util-msg:<VehicleBrakeCmd> is deprecated: use util-msg:VehicleBrakeCmd instead.")))

(cl:ensure-generic-function 'header-val :lambda-list '(m))
(cl:defmethod header-val ((m <VehicleBrakeCmd>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:header-val is deprecated.  Use util-msg:header instead.")
  (header m))

(cl:ensure-generic-function 'BrakeCmd-val :lambda-list '(m))
(cl:defmethod BrakeCmd-val ((m <VehicleBrakeCmd>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:BrakeCmd-val is deprecated.  Use util-msg:BrakeCmd instead.")
  (BrakeCmd m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <VehicleBrakeCmd>) ostream)
  "Serializes a message object of type '<VehicleBrakeCmd>"
  (roslisp-msg-protocol:serialize (cl:slot-value msg 'header) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'BrakeCmd) 1 0)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <VehicleBrakeCmd>) istream)
  "Deserializes a message object of type '<VehicleBrakeCmd>"
  (roslisp-msg-protocol:deserialize (cl:slot-value msg 'header) istream)
    (cl:setf (cl:slot-value msg 'BrakeCmd) (cl:not (cl:zerop (cl:read-byte istream))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<VehicleBrakeCmd>)))
  "Returns string type for a message object of type '<VehicleBrakeCmd>"
  "util/VehicleBrakeCmd")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'VehicleBrakeCmd)))
  "Returns string type for a message object of type 'VehicleBrakeCmd"
  "util/VehicleBrakeCmd")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<VehicleBrakeCmd>)))
  "Returns md5sum for a message object of type '<VehicleBrakeCmd>"
  "0effc161a91329ac582a70b1515cb00a")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'VehicleBrakeCmd)))
  "Returns md5sum for a message object of type 'VehicleBrakeCmd"
  "0effc161a91329ac582a70b1515cb00a")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<VehicleBrakeCmd>)))
  "Returns full string definition for message of type '<VehicleBrakeCmd>"
  (cl:format cl:nil "Header header~%~%bool BrakeCmd #刹车信号~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'VehicleBrakeCmd)))
  "Returns full string definition for message of type 'VehicleBrakeCmd"
  (cl:format cl:nil "Header header~%~%bool BrakeCmd #刹车信号~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <VehicleBrakeCmd>))
  (cl:+ 0
     (roslisp-msg-protocol:serialization-length (cl:slot-value msg 'header))
     1
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <VehicleBrakeCmd>))
  "Converts a ROS message object to a list"
  (cl:list 'VehicleBrakeCmd
    (cl:cons ':header (header msg))
    (cl:cons ':BrakeCmd (BrakeCmd msg))
))
