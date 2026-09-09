; Auto-generated. Do not edit!


(cl:in-package mower_msgs-msg)


;//! \htmlinclude CameraState.msg.html

(cl:defclass <CameraState> (roslisp-msg-protocol:ros-message)
  ((is_camera_ok
    :reader is_camera_ok
    :initarg :is_camera_ok
    :type cl:boolean
    :initform cl:nil))
)

(cl:defclass CameraState (<CameraState>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <CameraState>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'CameraState)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name mower_msgs-msg:<CameraState> is deprecated: use mower_msgs-msg:CameraState instead.")))

(cl:ensure-generic-function 'is_camera_ok-val :lambda-list '(m))
(cl:defmethod is_camera_ok-val ((m <CameraState>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:is_camera_ok-val is deprecated.  Use mower_msgs-msg:is_camera_ok instead.")
  (is_camera_ok m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <CameraState>) ostream)
  "Serializes a message object of type '<CameraState>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'is_camera_ok) 1 0)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <CameraState>) istream)
  "Deserializes a message object of type '<CameraState>"
    (cl:setf (cl:slot-value msg 'is_camera_ok) (cl:not (cl:zerop (cl:read-byte istream))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<CameraState>)))
  "Returns string type for a message object of type '<CameraState>"
  "mower_msgs/CameraState")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'CameraState)))
  "Returns string type for a message object of type 'CameraState"
  "mower_msgs/CameraState")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<CameraState>)))
  "Returns md5sum for a message object of type '<CameraState>"
  "56198635b7732d260a91b05411870558")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'CameraState)))
  "Returns md5sum for a message object of type 'CameraState"
  "56198635b7732d260a91b05411870558")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<CameraState>)))
  "Returns full string definition for message of type '<CameraState>"
  (cl:format cl:nil "bool is_camera_ok~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'CameraState)))
  "Returns full string definition for message of type 'CameraState"
  (cl:format cl:nil "bool is_camera_ok~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <CameraState>))
  (cl:+ 0
     1
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <CameraState>))
  "Converts a ROS message object to a list"
  (cl:list 'CameraState
    (cl:cons ':is_camera_ok (is_camera_ok msg))
))
