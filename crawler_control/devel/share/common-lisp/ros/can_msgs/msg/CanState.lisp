; Auto-generated. Do not edit!


(cl:in-package can_msgs-msg)


;//! \htmlinclude CanState.msg.html

(cl:defclass <CanState> (roslisp-msg-protocol:ros-message)
  ((driver_state
    :reader driver_state
    :initarg :driver_state
    :type cl:fixnum
    :initform 0))
)

(cl:defclass CanState (<CanState>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <CanState>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'CanState)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name can_msgs-msg:<CanState> is deprecated: use can_msgs-msg:CanState instead.")))

(cl:ensure-generic-function 'driver_state-val :lambda-list '(m))
(cl:defmethod driver_state-val ((m <CanState>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader can_msgs-msg:driver_state-val is deprecated.  Use can_msgs-msg:driver_state instead.")
  (driver_state m))
(cl:defmethod roslisp-msg-protocol:symbol-codes ((msg-type (cl:eql '<CanState>)))
    "Constants for message type '<CanState>"
  '((:CLOSED . 0)
    (:OPEN . 1)
    (:READY . 2))
)
(cl:defmethod roslisp-msg-protocol:symbol-codes ((msg-type (cl:eql 'CanState)))
    "Constants for message type 'CanState"
  '((:CLOSED . 0)
    (:OPEN . 1)
    (:READY . 2))
)
(cl:defmethod roslisp-msg-protocol:serialize ((msg <CanState>) ostream)
  "Serializes a message object of type '<CanState>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'driver_state)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <CanState>) istream)
  "Deserializes a message object of type '<CanState>"
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'driver_state)) (cl:read-byte istream))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<CanState>)))
  "Returns string type for a message object of type '<CanState>"
  "can_msgs/CanState")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'CanState)))
  "Returns string type for a message object of type 'CanState"
  "can_msgs/CanState")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<CanState>)))
  "Returns md5sum for a message object of type '<CanState>"
  "ae2e3357d3e85e9ae470ea447e6ef3b6")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'CanState)))
  "Returns md5sum for a message object of type 'CanState"
  "ae2e3357d3e85e9ae470ea447e6ef3b6")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<CanState>)))
  "Returns full string definition for message of type '<CanState>"
  (cl:format cl:nil "# CanState.msg~%#~%# Message denoting the state of the CAN Bus~%#~%# @author Joe Adkisson~%# @version 0.1~%~%uint8  driver_state~%~%# enumeration values for status:~%uint8 CLOSED=0~%uint8 OPEN=1~%uint8 READY=2~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'CanState)))
  "Returns full string definition for message of type 'CanState"
  (cl:format cl:nil "# CanState.msg~%#~%# Message denoting the state of the CAN Bus~%#~%# @author Joe Adkisson~%# @version 0.1~%~%uint8  driver_state~%~%# enumeration values for status:~%uint8 CLOSED=0~%uint8 OPEN=1~%uint8 READY=2~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <CanState>))
  (cl:+ 0
     1
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <CanState>))
  "Converts a ROS message object to a list"
  (cl:list 'CanState
    (cl:cons ':driver_state (driver_state msg))
))
