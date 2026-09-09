; Auto-generated. Do not edit!


(cl:in-package mower_msgs-msg)


;//! \htmlinclude Manual_Set.msg.html

(cl:defclass <Manual_Set> (roslisp-msg-protocol:ros-message)
  ((drive_mode
    :reader drive_mode
    :initarg :drive_mode
    :type cl:fixnum
    :initform 0)
   (speed_max
    :reader speed_max
    :initarg :speed_max
    :type cl:float
    :initform 0.0)
   (mow_height
    :reader mow_height
    :initarg :mow_height
    :type cl:fixnum
    :initform 0))
)

(cl:defclass Manual_Set (<Manual_Set>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <Manual_Set>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'Manual_Set)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name mower_msgs-msg:<Manual_Set> is deprecated: use mower_msgs-msg:Manual_Set instead.")))

(cl:ensure-generic-function 'drive_mode-val :lambda-list '(m))
(cl:defmethod drive_mode-val ((m <Manual_Set>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:drive_mode-val is deprecated.  Use mower_msgs-msg:drive_mode instead.")
  (drive_mode m))

(cl:ensure-generic-function 'speed_max-val :lambda-list '(m))
(cl:defmethod speed_max-val ((m <Manual_Set>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:speed_max-val is deprecated.  Use mower_msgs-msg:speed_max instead.")
  (speed_max m))

(cl:ensure-generic-function 'mow_height-val :lambda-list '(m))
(cl:defmethod mow_height-val ((m <Manual_Set>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:mow_height-val is deprecated.  Use mower_msgs-msg:mow_height instead.")
  (mow_height m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <Manual_Set>) ostream)
  "Serializes a message object of type '<Manual_Set>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'drive_mode)) ostream)
  (cl:let ((bits (roslisp-utils:encode-single-float-bits (cl:slot-value msg 'speed_max))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'mow_height)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <Manual_Set>) istream)
  "Deserializes a message object of type '<Manual_Set>"
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'drive_mode)) (cl:read-byte istream))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'speed_max) (roslisp-utils:decode-single-float-bits bits)))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'mow_height)) (cl:read-byte istream))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<Manual_Set>)))
  "Returns string type for a message object of type '<Manual_Set>"
  "mower_msgs/Manual_Set")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'Manual_Set)))
  "Returns string type for a message object of type 'Manual_Set"
  "mower_msgs/Manual_Set")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<Manual_Set>)))
  "Returns md5sum for a message object of type '<Manual_Set>"
  "232d61a03b6fa3c9aa8838703e52b5f4")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'Manual_Set)))
  "Returns md5sum for a message object of type 'Manual_Set"
  "232d61a03b6fa3c9aa8838703e52b5f4")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<Manual_Set>)))
  "Returns full string definition for message of type '<Manual_Set>"
  (cl:format cl:nil "uint8 drive_mode~%float32 speed_max~%uint8 mow_height~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'Manual_Set)))
  "Returns full string definition for message of type 'Manual_Set"
  (cl:format cl:nil "uint8 drive_mode~%float32 speed_max~%uint8 mow_height~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <Manual_Set>))
  (cl:+ 0
     1
     4
     1
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <Manual_Set>))
  "Converts a ROS message object to a list"
  (cl:list 'Manual_Set
    (cl:cons ':drive_mode (drive_mode msg))
    (cl:cons ':speed_max (speed_max msg))
    (cl:cons ':mow_height (mow_height msg))
))
