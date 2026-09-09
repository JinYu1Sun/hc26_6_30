; Auto-generated. Do not edit!


(cl:in-package mower_msgs-msg)


;//! \htmlinclude GnssOK.msg.html

(cl:defclass <GnssOK> (roslisp-msg-protocol:ros-message)
  ((is_gnss_ok
    :reader is_gnss_ok
    :initarg :is_gnss_ok
    :type cl:boolean
    :initform cl:nil))
)

(cl:defclass GnssOK (<GnssOK>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <GnssOK>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'GnssOK)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name mower_msgs-msg:<GnssOK> is deprecated: use mower_msgs-msg:GnssOK instead.")))

(cl:ensure-generic-function 'is_gnss_ok-val :lambda-list '(m))
(cl:defmethod is_gnss_ok-val ((m <GnssOK>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:is_gnss_ok-val is deprecated.  Use mower_msgs-msg:is_gnss_ok instead.")
  (is_gnss_ok m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <GnssOK>) ostream)
  "Serializes a message object of type '<GnssOK>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'is_gnss_ok) 1 0)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <GnssOK>) istream)
  "Deserializes a message object of type '<GnssOK>"
    (cl:setf (cl:slot-value msg 'is_gnss_ok) (cl:not (cl:zerop (cl:read-byte istream))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<GnssOK>)))
  "Returns string type for a message object of type '<GnssOK>"
  "mower_msgs/GnssOK")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'GnssOK)))
  "Returns string type for a message object of type 'GnssOK"
  "mower_msgs/GnssOK")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<GnssOK>)))
  "Returns md5sum for a message object of type '<GnssOK>"
  "928ba1b0bbd91610cb8761c3d298eeac")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'GnssOK)))
  "Returns md5sum for a message object of type 'GnssOK"
  "928ba1b0bbd91610cb8761c3d298eeac")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<GnssOK>)))
  "Returns full string definition for message of type '<GnssOK>"
  (cl:format cl:nil "bool is_gnss_ok ~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'GnssOK)))
  "Returns full string definition for message of type 'GnssOK"
  (cl:format cl:nil "bool is_gnss_ok ~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <GnssOK>))
  (cl:+ 0
     1
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <GnssOK>))
  "Converts a ROS message object to a list"
  (cl:list 'GnssOK
    (cl:cons ':is_gnss_ok (is_gnss_ok msg))
))
