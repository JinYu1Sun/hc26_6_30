; Auto-generated. Do not edit!


(cl:in-package mower_msgs-msg)


;//! \htmlinclude LidarSelfDtect.msg.html

(cl:defclass <LidarSelfDtect> (roslisp-msg-protocol:ros-message)
  ((is_lidar_ok
    :reader is_lidar_ok
    :initarg :is_lidar_ok
    :type cl:boolean
    :initform cl:nil))
)

(cl:defclass LidarSelfDtect (<LidarSelfDtect>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <LidarSelfDtect>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'LidarSelfDtect)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name mower_msgs-msg:<LidarSelfDtect> is deprecated: use mower_msgs-msg:LidarSelfDtect instead.")))

(cl:ensure-generic-function 'is_lidar_ok-val :lambda-list '(m))
(cl:defmethod is_lidar_ok-val ((m <LidarSelfDtect>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:is_lidar_ok-val is deprecated.  Use mower_msgs-msg:is_lidar_ok instead.")
  (is_lidar_ok m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <LidarSelfDtect>) ostream)
  "Serializes a message object of type '<LidarSelfDtect>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'is_lidar_ok) 1 0)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <LidarSelfDtect>) istream)
  "Deserializes a message object of type '<LidarSelfDtect>"
    (cl:setf (cl:slot-value msg 'is_lidar_ok) (cl:not (cl:zerop (cl:read-byte istream))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<LidarSelfDtect>)))
  "Returns string type for a message object of type '<LidarSelfDtect>"
  "mower_msgs/LidarSelfDtect")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'LidarSelfDtect)))
  "Returns string type for a message object of type 'LidarSelfDtect"
  "mower_msgs/LidarSelfDtect")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<LidarSelfDtect>)))
  "Returns md5sum for a message object of type '<LidarSelfDtect>"
  "61b1182e9e63e0b66c126403f70148e4")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'LidarSelfDtect)))
  "Returns md5sum for a message object of type 'LidarSelfDtect"
  "61b1182e9e63e0b66c126403f70148e4")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<LidarSelfDtect>)))
  "Returns full string definition for message of type '<LidarSelfDtect>"
  (cl:format cl:nil "bool is_lidar_ok~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'LidarSelfDtect)))
  "Returns full string definition for message of type 'LidarSelfDtect"
  (cl:format cl:nil "bool is_lidar_ok~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <LidarSelfDtect>))
  (cl:+ 0
     1
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <LidarSelfDtect>))
  "Converts a ROS message object to a list"
  (cl:list 'LidarSelfDtect
    (cl:cons ':is_lidar_ok (is_lidar_ok msg))
))
