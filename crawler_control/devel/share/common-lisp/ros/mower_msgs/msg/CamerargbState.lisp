; Auto-generated. Do not edit!


(cl:in-package mower_msgs-msg)


;//! \htmlinclude CamerargbState.msg.html

(cl:defclass <CamerargbState> (roslisp-msg-protocol:ros-message)
  ((is_camerargb_ok
    :reader is_camerargb_ok
    :initarg :is_camerargb_ok
    :type cl:boolean
    :initform cl:nil))
)

(cl:defclass CamerargbState (<CamerargbState>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <CamerargbState>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'CamerargbState)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name mower_msgs-msg:<CamerargbState> is deprecated: use mower_msgs-msg:CamerargbState instead.")))

(cl:ensure-generic-function 'is_camerargb_ok-val :lambda-list '(m))
(cl:defmethod is_camerargb_ok-val ((m <CamerargbState>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:is_camerargb_ok-val is deprecated.  Use mower_msgs-msg:is_camerargb_ok instead.")
  (is_camerargb_ok m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <CamerargbState>) ostream)
  "Serializes a message object of type '<CamerargbState>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'is_camerargb_ok) 1 0)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <CamerargbState>) istream)
  "Deserializes a message object of type '<CamerargbState>"
    (cl:setf (cl:slot-value msg 'is_camerargb_ok) (cl:not (cl:zerop (cl:read-byte istream))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<CamerargbState>)))
  "Returns string type for a message object of type '<CamerargbState>"
  "mower_msgs/CamerargbState")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'CamerargbState)))
  "Returns string type for a message object of type 'CamerargbState"
  "mower_msgs/CamerargbState")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<CamerargbState>)))
  "Returns md5sum for a message object of type '<CamerargbState>"
  "8721ee57cc9045b375b087d6e5ab5399")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'CamerargbState)))
  "Returns md5sum for a message object of type 'CamerargbState"
  "8721ee57cc9045b375b087d6e5ab5399")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<CamerargbState>)))
  "Returns full string definition for message of type '<CamerargbState>"
  (cl:format cl:nil "bool is_camerargb_ok~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'CamerargbState)))
  "Returns full string definition for message of type 'CamerargbState"
  (cl:format cl:nil "bool is_camerargb_ok~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <CamerargbState>))
  (cl:+ 0
     1
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <CamerargbState>))
  "Converts a ROS message object to a list"
  (cl:list 'CamerargbState
    (cl:cons ':is_camerargb_ok (is_camerargb_ok msg))
))
