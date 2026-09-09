; Auto-generated. Do not edit!


(cl:in-package mower_msgs-msg)


;//! \htmlinclude PlanType.msg.html

(cl:defclass <PlanType> (roslisp-msg-protocol:ros-message)
  ((plan_type_state
    :reader plan_type_state
    :initarg :plan_type_state
    :type cl:fixnum
    :initform 0))
)

(cl:defclass PlanType (<PlanType>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <PlanType>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'PlanType)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name mower_msgs-msg:<PlanType> is deprecated: use mower_msgs-msg:PlanType instead.")))

(cl:ensure-generic-function 'plan_type_state-val :lambda-list '(m))
(cl:defmethod plan_type_state-val ((m <PlanType>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:plan_type_state-val is deprecated.  Use mower_msgs-msg:plan_type_state instead.")
  (plan_type_state m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <PlanType>) ostream)
  "Serializes a message object of type '<PlanType>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'plan_type_state)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <PlanType>) istream)
  "Deserializes a message object of type '<PlanType>"
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'plan_type_state)) (cl:read-byte istream))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<PlanType>)))
  "Returns string type for a message object of type '<PlanType>"
  "mower_msgs/PlanType")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'PlanType)))
  "Returns string type for a message object of type 'PlanType"
  "mower_msgs/PlanType")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<PlanType>)))
  "Returns md5sum for a message object of type '<PlanType>"
  "23d6dde9eb5a6d1df00bc1332e5a8aeb")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'PlanType)))
  "Returns md5sum for a message object of type 'PlanType"
  "23d6dde9eb5a6d1df00bc1332e5a8aeb")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<PlanType>)))
  "Returns full string definition for message of type '<PlanType>"
  (cl:format cl:nil "uint8 plan_type_state~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'PlanType)))
  "Returns full string definition for message of type 'PlanType"
  (cl:format cl:nil "uint8 plan_type_state~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <PlanType>))
  (cl:+ 0
     1
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <PlanType>))
  "Converts a ROS message object to a list"
  (cl:list 'PlanType
    (cl:cons ':plan_type_state (plan_type_state msg))
))
