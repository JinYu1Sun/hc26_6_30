; Auto-generated. Do not edit!


(cl:in-package mower_msgs-msg)


;//! \htmlinclude Direct_Control.msg.html

(cl:defclass <Direct_Control> (roslisp-msg-protocol:ros-message)
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

(cl:defclass Direct_Control (<Direct_Control>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <Direct_Control>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'Direct_Control)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name mower_msgs-msg:<Direct_Control> is deprecated: use mower_msgs-msg:Direct_Control instead.")))

(cl:ensure-generic-function 'drive_mode-val :lambda-list '(m))
(cl:defmethod drive_mode-val ((m <Direct_Control>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:drive_mode-val is deprecated.  Use mower_msgs-msg:drive_mode instead.")
  (drive_mode m))

(cl:ensure-generic-function 'speed_max-val :lambda-list '(m))
(cl:defmethod speed_max-val ((m <Direct_Control>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:speed_max-val is deprecated.  Use mower_msgs-msg:speed_max instead.")
  (speed_max m))

(cl:ensure-generic-function 'mow_height-val :lambda-list '(m))
(cl:defmethod mow_height-val ((m <Direct_Control>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:mow_height-val is deprecated.  Use mower_msgs-msg:mow_height instead.")
  (mow_height m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <Direct_Control>) ostream)
  "Serializes a message object of type '<Direct_Control>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'drive_mode)) ostream)
  (cl:let ((bits (roslisp-utils:encode-double-float-bits (cl:slot-value msg 'speed_max))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 32) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 40) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 48) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 56) bits) ostream))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'mow_height)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <Direct_Control>) istream)
  "Deserializes a message object of type '<Direct_Control>"
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'drive_mode)) (cl:read-byte istream))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 32) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 40) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 48) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 56) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'speed_max) (roslisp-utils:decode-double-float-bits bits)))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'mow_height)) (cl:read-byte istream))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<Direct_Control>)))
  "Returns string type for a message object of type '<Direct_Control>"
  "mower_msgs/Direct_Control")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'Direct_Control)))
  "Returns string type for a message object of type 'Direct_Control"
  "mower_msgs/Direct_Control")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<Direct_Control>)))
  "Returns md5sum for a message object of type '<Direct_Control>"
  "10579a0a02d1c1d2b089c0bf3df04393")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'Direct_Control)))
  "Returns md5sum for a message object of type 'Direct_Control"
  "10579a0a02d1c1d2b089c0bf3df04393")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<Direct_Control>)))
  "Returns full string definition for message of type '<Direct_Control>"
  (cl:format cl:nil "uint8 drive_mode~%float64 speed_max~%uint8 mow_height~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'Direct_Control)))
  "Returns full string definition for message of type 'Direct_Control"
  (cl:format cl:nil "uint8 drive_mode~%float64 speed_max~%uint8 mow_height~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <Direct_Control>))
  (cl:+ 0
     1
     8
     1
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <Direct_Control>))
  "Converts a ROS message object to a list"
  (cl:list 'Direct_Control
    (cl:cons ':drive_mode (drive_mode msg))
    (cl:cons ':speed_max (speed_max msg))
    (cl:cons ':mow_height (mow_height msg))
))
