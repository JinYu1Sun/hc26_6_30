; Auto-generated. Do not edit!


(cl:in-package mower_msgs-msg)


;//! \htmlinclude ControlState.msg.html

(cl:defclass <ControlState> (roslisp-msg-protocol:ros-message)
  ((header
    :reader header
    :initarg :header
    :type std_msgs-msg:Header
    :initform (cl:make-instance 'std_msgs-msg:Header))
   (control_state
    :reader control_state
    :initarg :control_state
    :type cl:fixnum
    :initform 0))
)

(cl:defclass ControlState (<ControlState>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <ControlState>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'ControlState)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name mower_msgs-msg:<ControlState> is deprecated: use mower_msgs-msg:ControlState instead.")))

(cl:ensure-generic-function 'header-val :lambda-list '(m))
(cl:defmethod header-val ((m <ControlState>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:header-val is deprecated.  Use mower_msgs-msg:header instead.")
  (header m))

(cl:ensure-generic-function 'control_state-val :lambda-list '(m))
(cl:defmethod control_state-val ((m <ControlState>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:control_state-val is deprecated.  Use mower_msgs-msg:control_state instead.")
  (control_state m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <ControlState>) ostream)
  "Serializes a message object of type '<ControlState>"
  (roslisp-msg-protocol:serialize (cl:slot-value msg 'header) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'control_state)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <ControlState>) istream)
  "Deserializes a message object of type '<ControlState>"
  (roslisp-msg-protocol:deserialize (cl:slot-value msg 'header) istream)
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'control_state)) (cl:read-byte istream))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<ControlState>)))
  "Returns string type for a message object of type '<ControlState>"
  "mower_msgs/ControlState")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'ControlState)))
  "Returns string type for a message object of type 'ControlState"
  "mower_msgs/ControlState")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<ControlState>)))
  "Returns md5sum for a message object of type '<ControlState>"
  "15251f5c1c2c764e6732e712efff289a")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'ControlState)))
  "Returns md5sum for a message object of type 'ControlState"
  "15251f5c1c2c764e6732e712efff289a")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<ControlState>)))
  "Returns full string definition for message of type '<ControlState>"
  (cl:format cl:nil "Header header~%~%uint8 control_state~%~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'ControlState)))
  "Returns full string definition for message of type 'ControlState"
  (cl:format cl:nil "Header header~%~%uint8 control_state~%~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <ControlState>))
  (cl:+ 0
     (roslisp-msg-protocol:serialization-length (cl:slot-value msg 'header))
     1
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <ControlState>))
  "Converts a ROS message object to a list"
  (cl:list 'ControlState
    (cl:cons ':header (header msg))
    (cl:cons ':control_state (control_state msg))
))
