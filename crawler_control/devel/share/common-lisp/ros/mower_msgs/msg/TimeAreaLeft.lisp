; Auto-generated. Do not edit!


(cl:in-package mower_msgs-msg)


;//! \htmlinclude TimeAreaLeft.msg.html

(cl:defclass <TimeAreaLeft> (roslisp-msg-protocol:ros-message)
  ((time_left
    :reader time_left
    :initarg :time_left
    :type cl:float
    :initform 0.0)
   (area_left
    :reader area_left
    :initarg :area_left
    :type cl:float
    :initform 0.0))
)

(cl:defclass TimeAreaLeft (<TimeAreaLeft>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <TimeAreaLeft>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'TimeAreaLeft)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name mower_msgs-msg:<TimeAreaLeft> is deprecated: use mower_msgs-msg:TimeAreaLeft instead.")))

(cl:ensure-generic-function 'time_left-val :lambda-list '(m))
(cl:defmethod time_left-val ((m <TimeAreaLeft>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:time_left-val is deprecated.  Use mower_msgs-msg:time_left instead.")
  (time_left m))

(cl:ensure-generic-function 'area_left-val :lambda-list '(m))
(cl:defmethod area_left-val ((m <TimeAreaLeft>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:area_left-val is deprecated.  Use mower_msgs-msg:area_left instead.")
  (area_left m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <TimeAreaLeft>) ostream)
  "Serializes a message object of type '<TimeAreaLeft>"
  (cl:let ((bits (roslisp-utils:encode-double-float-bits (cl:slot-value msg 'time_left))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 32) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 40) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 48) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 56) bits) ostream))
  (cl:let ((bits (roslisp-utils:encode-double-float-bits (cl:slot-value msg 'area_left))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 32) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 40) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 48) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 56) bits) ostream))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <TimeAreaLeft>) istream)
  "Deserializes a message object of type '<TimeAreaLeft>"
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 32) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 40) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 48) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 56) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'time_left) (roslisp-utils:decode-double-float-bits bits)))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 32) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 40) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 48) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 56) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'area_left) (roslisp-utils:decode-double-float-bits bits)))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<TimeAreaLeft>)))
  "Returns string type for a message object of type '<TimeAreaLeft>"
  "mower_msgs/TimeAreaLeft")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'TimeAreaLeft)))
  "Returns string type for a message object of type 'TimeAreaLeft"
  "mower_msgs/TimeAreaLeft")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<TimeAreaLeft>)))
  "Returns md5sum for a message object of type '<TimeAreaLeft>"
  "3450601933b56d4bc50d61223a1754f6")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'TimeAreaLeft)))
  "Returns md5sum for a message object of type 'TimeAreaLeft"
  "3450601933b56d4bc50d61223a1754f6")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<TimeAreaLeft>)))
  "Returns full string definition for message of type '<TimeAreaLeft>"
  (cl:format cl:nil "float64 time_left~%float64 area_left~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'TimeAreaLeft)))
  "Returns full string definition for message of type 'TimeAreaLeft"
  (cl:format cl:nil "float64 time_left~%float64 area_left~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <TimeAreaLeft>))
  (cl:+ 0
     8
     8
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <TimeAreaLeft>))
  "Converts a ROS message object to a list"
  (cl:list 'TimeAreaLeft
    (cl:cons ':time_left (time_left msg))
    (cl:cons ':area_left (area_left msg))
))
