; Auto-generated. Do not edit!


(cl:in-package mower_msgs-msg)


;//! \htmlinclude MultiMapSelfDetect.msg.html

(cl:defclass <MultiMapSelfDetect> (roslisp-msg-protocol:ros-message)
  ((is_multi_map_ok
    :reader is_multi_map_ok
    :initarg :is_multi_map_ok
    :type cl:boolean
    :initform cl:nil))
)

(cl:defclass MultiMapSelfDetect (<MultiMapSelfDetect>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <MultiMapSelfDetect>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'MultiMapSelfDetect)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name mower_msgs-msg:<MultiMapSelfDetect> is deprecated: use mower_msgs-msg:MultiMapSelfDetect instead.")))

(cl:ensure-generic-function 'is_multi_map_ok-val :lambda-list '(m))
(cl:defmethod is_multi_map_ok-val ((m <MultiMapSelfDetect>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:is_multi_map_ok-val is deprecated.  Use mower_msgs-msg:is_multi_map_ok instead.")
  (is_multi_map_ok m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <MultiMapSelfDetect>) ostream)
  "Serializes a message object of type '<MultiMapSelfDetect>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'is_multi_map_ok) 1 0)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <MultiMapSelfDetect>) istream)
  "Deserializes a message object of type '<MultiMapSelfDetect>"
    (cl:setf (cl:slot-value msg 'is_multi_map_ok) (cl:not (cl:zerop (cl:read-byte istream))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<MultiMapSelfDetect>)))
  "Returns string type for a message object of type '<MultiMapSelfDetect>"
  "mower_msgs/MultiMapSelfDetect")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'MultiMapSelfDetect)))
  "Returns string type for a message object of type 'MultiMapSelfDetect"
  "mower_msgs/MultiMapSelfDetect")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<MultiMapSelfDetect>)))
  "Returns md5sum for a message object of type '<MultiMapSelfDetect>"
  "31d7c6c31bbf8db62e247ca761103d55")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'MultiMapSelfDetect)))
  "Returns md5sum for a message object of type 'MultiMapSelfDetect"
  "31d7c6c31bbf8db62e247ca761103d55")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<MultiMapSelfDetect>)))
  "Returns full string definition for message of type '<MultiMapSelfDetect>"
  (cl:format cl:nil "bool is_multi_map_ok~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'MultiMapSelfDetect)))
  "Returns full string definition for message of type 'MultiMapSelfDetect"
  (cl:format cl:nil "bool is_multi_map_ok~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <MultiMapSelfDetect>))
  (cl:+ 0
     1
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <MultiMapSelfDetect>))
  "Converts a ROS message object to a list"
  (cl:list 'MultiMapSelfDetect
    (cl:cons ':is_multi_map_ok (is_multi_map_ok msg))
))
