; Auto-generated. Do not edit!


(cl:in-package mower_msgs-msg)


;//! \htmlinclude PlaningOK.msg.html

(cl:defclass <PlaningOK> (roslisp-msg-protocol:ros-message)
  ((is_planing_ok
    :reader is_planing_ok
    :initarg :is_planing_ok
    :type cl:boolean
    :initform cl:nil))
)

(cl:defclass PlaningOK (<PlaningOK>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <PlaningOK>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'PlaningOK)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name mower_msgs-msg:<PlaningOK> is deprecated: use mower_msgs-msg:PlaningOK instead.")))

(cl:ensure-generic-function 'is_planing_ok-val :lambda-list '(m))
(cl:defmethod is_planing_ok-val ((m <PlaningOK>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:is_planing_ok-val is deprecated.  Use mower_msgs-msg:is_planing_ok instead.")
  (is_planing_ok m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <PlaningOK>) ostream)
  "Serializes a message object of type '<PlaningOK>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'is_planing_ok) 1 0)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <PlaningOK>) istream)
  "Deserializes a message object of type '<PlaningOK>"
    (cl:setf (cl:slot-value msg 'is_planing_ok) (cl:not (cl:zerop (cl:read-byte istream))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<PlaningOK>)))
  "Returns string type for a message object of type '<PlaningOK>"
  "mower_msgs/PlaningOK")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'PlaningOK)))
  "Returns string type for a message object of type 'PlaningOK"
  "mower_msgs/PlaningOK")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<PlaningOK>)))
  "Returns md5sum for a message object of type '<PlaningOK>"
  "60643f33fa88814b437453da7ce2bc4b")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'PlaningOK)))
  "Returns md5sum for a message object of type 'PlaningOK"
  "60643f33fa88814b437453da7ce2bc4b")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<PlaningOK>)))
  "Returns full string definition for message of type '<PlaningOK>"
  (cl:format cl:nil "bool is_planing_ok~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'PlaningOK)))
  "Returns full string definition for message of type 'PlaningOK"
  (cl:format cl:nil "bool is_planing_ok~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <PlaningOK>))
  (cl:+ 0
     1
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <PlaningOK>))
  "Converts a ROS message object to a list"
  (cl:list 'PlaningOK
    (cl:cons ':is_planing_ok (is_planing_ok msg))
))
