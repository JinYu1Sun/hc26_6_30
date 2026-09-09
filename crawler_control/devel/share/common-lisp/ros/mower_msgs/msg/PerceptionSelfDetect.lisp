; Auto-generated. Do not edit!


(cl:in-package mower_msgs-msg)


;//! \htmlinclude PerceptionSelfDetect.msg.html

(cl:defclass <PerceptionSelfDetect> (roslisp-msg-protocol:ros-message)
  ((is_perception_ok
    :reader is_perception_ok
    :initarg :is_perception_ok
    :type cl:boolean
    :initform cl:nil))
)

(cl:defclass PerceptionSelfDetect (<PerceptionSelfDetect>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <PerceptionSelfDetect>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'PerceptionSelfDetect)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name mower_msgs-msg:<PerceptionSelfDetect> is deprecated: use mower_msgs-msg:PerceptionSelfDetect instead.")))

(cl:ensure-generic-function 'is_perception_ok-val :lambda-list '(m))
(cl:defmethod is_perception_ok-val ((m <PerceptionSelfDetect>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:is_perception_ok-val is deprecated.  Use mower_msgs-msg:is_perception_ok instead.")
  (is_perception_ok m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <PerceptionSelfDetect>) ostream)
  "Serializes a message object of type '<PerceptionSelfDetect>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'is_perception_ok) 1 0)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <PerceptionSelfDetect>) istream)
  "Deserializes a message object of type '<PerceptionSelfDetect>"
    (cl:setf (cl:slot-value msg 'is_perception_ok) (cl:not (cl:zerop (cl:read-byte istream))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<PerceptionSelfDetect>)))
  "Returns string type for a message object of type '<PerceptionSelfDetect>"
  "mower_msgs/PerceptionSelfDetect")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'PerceptionSelfDetect)))
  "Returns string type for a message object of type 'PerceptionSelfDetect"
  "mower_msgs/PerceptionSelfDetect")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<PerceptionSelfDetect>)))
  "Returns md5sum for a message object of type '<PerceptionSelfDetect>"
  "1e4f85ff5f6ce4dd7e761543975830a4")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'PerceptionSelfDetect)))
  "Returns md5sum for a message object of type 'PerceptionSelfDetect"
  "1e4f85ff5f6ce4dd7e761543975830a4")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<PerceptionSelfDetect>)))
  "Returns full string definition for message of type '<PerceptionSelfDetect>"
  (cl:format cl:nil "bool is_perception_ok~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'PerceptionSelfDetect)))
  "Returns full string definition for message of type 'PerceptionSelfDetect"
  (cl:format cl:nil "bool is_perception_ok~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <PerceptionSelfDetect>))
  (cl:+ 0
     1
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <PerceptionSelfDetect>))
  "Converts a ROS message object to a list"
  (cl:list 'PerceptionSelfDetect
    (cl:cons ':is_perception_ok (is_perception_ok msg))
))
