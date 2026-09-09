; Auto-generated. Do not edit!


(cl:in-package mower_msgs-msg)


;//! \htmlinclude ControlOk.msg.html

(cl:defclass <ControlOk> (roslisp-msg-protocol:ros-message)
  ((header
    :reader header
    :initarg :header
    :type std_msgs-msg:Header
    :initform (cl:make-instance 'std_msgs-msg:Header))
   (is_control_ok
    :reader is_control_ok
    :initarg :is_control_ok
    :type cl:boolean
    :initform cl:nil)
   (finished
    :reader finished
    :initarg :finished
    :type cl:boolean
    :initform cl:nil))
)

(cl:defclass ControlOk (<ControlOk>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <ControlOk>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'ControlOk)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name mower_msgs-msg:<ControlOk> is deprecated: use mower_msgs-msg:ControlOk instead.")))

(cl:ensure-generic-function 'header-val :lambda-list '(m))
(cl:defmethod header-val ((m <ControlOk>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:header-val is deprecated.  Use mower_msgs-msg:header instead.")
  (header m))

(cl:ensure-generic-function 'is_control_ok-val :lambda-list '(m))
(cl:defmethod is_control_ok-val ((m <ControlOk>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:is_control_ok-val is deprecated.  Use mower_msgs-msg:is_control_ok instead.")
  (is_control_ok m))

(cl:ensure-generic-function 'finished-val :lambda-list '(m))
(cl:defmethod finished-val ((m <ControlOk>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:finished-val is deprecated.  Use mower_msgs-msg:finished instead.")
  (finished m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <ControlOk>) ostream)
  "Serializes a message object of type '<ControlOk>"
  (roslisp-msg-protocol:serialize (cl:slot-value msg 'header) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'is_control_ok) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'finished) 1 0)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <ControlOk>) istream)
  "Deserializes a message object of type '<ControlOk>"
  (roslisp-msg-protocol:deserialize (cl:slot-value msg 'header) istream)
    (cl:setf (cl:slot-value msg 'is_control_ok) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:slot-value msg 'finished) (cl:not (cl:zerop (cl:read-byte istream))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<ControlOk>)))
  "Returns string type for a message object of type '<ControlOk>"
  "mower_msgs/ControlOk")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'ControlOk)))
  "Returns string type for a message object of type 'ControlOk"
  "mower_msgs/ControlOk")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<ControlOk>)))
  "Returns md5sum for a message object of type '<ControlOk>"
  "b1b519c21944b4cf1e0569f6c2cb3a72")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'ControlOk)))
  "Returns md5sum for a message object of type 'ControlOk"
  "b1b519c21944b4cf1e0569f6c2cb3a72")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<ControlOk>)))
  "Returns full string definition for message of type '<ControlOk>"
  (cl:format cl:nil "Header header~%~%~%bool is_control_ok~%bool finished~%~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'ControlOk)))
  "Returns full string definition for message of type 'ControlOk"
  (cl:format cl:nil "Header header~%~%~%bool is_control_ok~%bool finished~%~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <ControlOk>))
  (cl:+ 0
     (roslisp-msg-protocol:serialization-length (cl:slot-value msg 'header))
     1
     1
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <ControlOk>))
  "Converts a ROS message object to a list"
  (cl:list 'ControlOk
    (cl:cons ':header (header msg))
    (cl:cons ':is_control_ok (is_control_ok msg))
    (cl:cons ':finished (finished msg))
))
