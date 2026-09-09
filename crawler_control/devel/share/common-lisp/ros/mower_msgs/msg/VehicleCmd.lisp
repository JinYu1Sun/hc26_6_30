; Auto-generated. Do not edit!


(cl:in-package mower_msgs-msg)


;//! \htmlinclude VehicleCmd.msg.html

(cl:defclass <VehicleCmd> (roslisp-msg-protocol:ros-message)
  ((header
    :reader header
    :initarg :header
    :type std_msgs-msg:Header
    :initform (cl:make-instance 'std_msgs-msg:Header))
   (mover_bool
    :reader mover_bool
    :initarg :mover_bool
    :type cl:fixnum
    :initform 0)
   (ad_control_enable
    :reader ad_control_enable
    :initarg :ad_control_enable
    :type cl:fixnum
    :initform 0)
   (gear_model
    :reader gear_model
    :initarg :gear_model
    :type cl:fixnum
    :initform 0)
   (turn_value
    :reader turn_value
    :initarg :turn_value
    :type cl:fixnum
    :initform 0)
   (drive_value
    :reader drive_value
    :initarg :drive_value
    :type cl:fixnum
    :initform 0)
   (mower_height
    :reader mower_height
    :initarg :mower_height
    :type cl:fixnum
    :initform 0)
   (left_wheel_speed
    :reader left_wheel_speed
    :initarg :left_wheel_speed
    :type cl:float
    :initform 0.0)
   (right_wheel_speed
    :reader right_wheel_speed
    :initarg :right_wheel_speed
    :type cl:float
    :initform 0.0)
   (speed_cmd
    :reader speed_cmd
    :initarg :speed_cmd
    :type cl:float
    :initform 0.0))
)

(cl:defclass VehicleCmd (<VehicleCmd>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <VehicleCmd>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'VehicleCmd)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name mower_msgs-msg:<VehicleCmd> is deprecated: use mower_msgs-msg:VehicleCmd instead.")))

(cl:ensure-generic-function 'header-val :lambda-list '(m))
(cl:defmethod header-val ((m <VehicleCmd>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:header-val is deprecated.  Use mower_msgs-msg:header instead.")
  (header m))

(cl:ensure-generic-function 'mover_bool-val :lambda-list '(m))
(cl:defmethod mover_bool-val ((m <VehicleCmd>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:mover_bool-val is deprecated.  Use mower_msgs-msg:mover_bool instead.")
  (mover_bool m))

(cl:ensure-generic-function 'ad_control_enable-val :lambda-list '(m))
(cl:defmethod ad_control_enable-val ((m <VehicleCmd>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:ad_control_enable-val is deprecated.  Use mower_msgs-msg:ad_control_enable instead.")
  (ad_control_enable m))

(cl:ensure-generic-function 'gear_model-val :lambda-list '(m))
(cl:defmethod gear_model-val ((m <VehicleCmd>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:gear_model-val is deprecated.  Use mower_msgs-msg:gear_model instead.")
  (gear_model m))

(cl:ensure-generic-function 'turn_value-val :lambda-list '(m))
(cl:defmethod turn_value-val ((m <VehicleCmd>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:turn_value-val is deprecated.  Use mower_msgs-msg:turn_value instead.")
  (turn_value m))

(cl:ensure-generic-function 'drive_value-val :lambda-list '(m))
(cl:defmethod drive_value-val ((m <VehicleCmd>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:drive_value-val is deprecated.  Use mower_msgs-msg:drive_value instead.")
  (drive_value m))

(cl:ensure-generic-function 'mower_height-val :lambda-list '(m))
(cl:defmethod mower_height-val ((m <VehicleCmd>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:mower_height-val is deprecated.  Use mower_msgs-msg:mower_height instead.")
  (mower_height m))

(cl:ensure-generic-function 'left_wheel_speed-val :lambda-list '(m))
(cl:defmethod left_wheel_speed-val ((m <VehicleCmd>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:left_wheel_speed-val is deprecated.  Use mower_msgs-msg:left_wheel_speed instead.")
  (left_wheel_speed m))

(cl:ensure-generic-function 'right_wheel_speed-val :lambda-list '(m))
(cl:defmethod right_wheel_speed-val ((m <VehicleCmd>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:right_wheel_speed-val is deprecated.  Use mower_msgs-msg:right_wheel_speed instead.")
  (right_wheel_speed m))

(cl:ensure-generic-function 'speed_cmd-val :lambda-list '(m))
(cl:defmethod speed_cmd-val ((m <VehicleCmd>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:speed_cmd-val is deprecated.  Use mower_msgs-msg:speed_cmd instead.")
  (speed_cmd m))
(cl:defmethod roslisp-msg-protocol:symbol-codes ((msg-type (cl:eql '<VehicleCmd>)))
    "Constants for message type '<VehicleCmd>"
  '((:P_GEAR . 0)
    (:R_GEAR . 1)
    (:N_GEAR . 2)
    (:D_GEAR . 3))
)
(cl:defmethod roslisp-msg-protocol:symbol-codes ((msg-type (cl:eql 'VehicleCmd)))
    "Constants for message type 'VehicleCmd"
  '((:P_GEAR . 0)
    (:R_GEAR . 1)
    (:N_GEAR . 2)
    (:D_GEAR . 3))
)
(cl:defmethod roslisp-msg-protocol:serialize ((msg <VehicleCmd>) ostream)
  "Serializes a message object of type '<VehicleCmd>"
  (roslisp-msg-protocol:serialize (cl:slot-value msg 'header) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'mover_bool)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'ad_control_enable)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'gear_model)) ostream)
  (cl:let* ((signed (cl:slot-value msg 'turn_value)) (unsigned (cl:if (cl:< signed 0) (cl:+ signed 65536) signed)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) unsigned) ostream)
    )
  (cl:let* ((signed (cl:slot-value msg 'drive_value)) (unsigned (cl:if (cl:< signed 0) (cl:+ signed 65536) signed)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) unsigned) ostream)
    )
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'mower_height)) ostream)
  (cl:let ((bits (roslisp-utils:encode-double-float-bits (cl:slot-value msg 'left_wheel_speed))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 32) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 40) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 48) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 56) bits) ostream))
  (cl:let ((bits (roslisp-utils:encode-double-float-bits (cl:slot-value msg 'right_wheel_speed))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 32) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 40) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 48) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 56) bits) ostream))
  (cl:let ((bits (roslisp-utils:encode-double-float-bits (cl:slot-value msg 'speed_cmd))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 32) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 40) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 48) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 56) bits) ostream))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <VehicleCmd>) istream)
  "Deserializes a message object of type '<VehicleCmd>"
  (roslisp-msg-protocol:deserialize (cl:slot-value msg 'header) istream)
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'mover_bool)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'ad_control_enable)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'gear_model)) (cl:read-byte istream))
    (cl:let ((unsigned 0))
      (cl:setf (cl:ldb (cl:byte 8 0) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) unsigned) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'turn_value) (cl:if (cl:< unsigned 32768) unsigned (cl:- unsigned 65536))))
    (cl:let ((unsigned 0))
      (cl:setf (cl:ldb (cl:byte 8 0) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) unsigned) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'drive_value) (cl:if (cl:< unsigned 32768) unsigned (cl:- unsigned 65536))))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'mower_height)) (cl:read-byte istream))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 32) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 40) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 48) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 56) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'left_wheel_speed) (roslisp-utils:decode-double-float-bits bits)))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 32) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 40) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 48) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 56) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'right_wheel_speed) (roslisp-utils:decode-double-float-bits bits)))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 32) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 40) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 48) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 56) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'speed_cmd) (roslisp-utils:decode-double-float-bits bits)))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<VehicleCmd>)))
  "Returns string type for a message object of type '<VehicleCmd>"
  "mower_msgs/VehicleCmd")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'VehicleCmd)))
  "Returns string type for a message object of type 'VehicleCmd"
  "mower_msgs/VehicleCmd")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<VehicleCmd>)))
  "Returns md5sum for a message object of type '<VehicleCmd>"
  "cd0b96d90dfa95f8decc74c9ae3e5d02")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'VehicleCmd)))
  "Returns md5sum for a message object of type 'VehicleCmd"
  "cd0b96d90dfa95f8decc74c9ae3e5d02")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<VehicleCmd>)))
  "Returns full string definition for message of type '<VehicleCmd>"
  (cl:format cl:nil "Header header~%# VehicleCmd~%# Gear_model~%uint8 p_Gear = 0 # 0x00： P 档~%uint8 R_Gear = 1 # 0x01： 无~%uint8 N_Gear = 2 # 0x02： 转弯信号~%uint8 D_Gear = 3 # 0x03： D 前进档~%~%# ADCmd 0x310~%uint8 mover_bool~%uint8 ad_control_enable ~%uint8 gear_model~%int16 turn_value~%int16 drive_value~%uint8 mower_height~%~%float64 left_wheel_speed  # 无~%float64 right_wheel_speed # 无~%float64 speed_cmd   # int8 [-128~~127]~%~%~%~%~%~%~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'VehicleCmd)))
  "Returns full string definition for message of type 'VehicleCmd"
  (cl:format cl:nil "Header header~%# VehicleCmd~%# Gear_model~%uint8 p_Gear = 0 # 0x00： P 档~%uint8 R_Gear = 1 # 0x01： 无~%uint8 N_Gear = 2 # 0x02： 转弯信号~%uint8 D_Gear = 3 # 0x03： D 前进档~%~%# ADCmd 0x310~%uint8 mover_bool~%uint8 ad_control_enable ~%uint8 gear_model~%int16 turn_value~%int16 drive_value~%uint8 mower_height~%~%float64 left_wheel_speed  # 无~%float64 right_wheel_speed # 无~%float64 speed_cmd   # int8 [-128~~127]~%~%~%~%~%~%~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <VehicleCmd>))
  (cl:+ 0
     (roslisp-msg-protocol:serialization-length (cl:slot-value msg 'header))
     1
     1
     1
     2
     2
     1
     8
     8
     8
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <VehicleCmd>))
  "Converts a ROS message object to a list"
  (cl:list 'VehicleCmd
    (cl:cons ':header (header msg))
    (cl:cons ':mover_bool (mover_bool msg))
    (cl:cons ':ad_control_enable (ad_control_enable msg))
    (cl:cons ':gear_model (gear_model msg))
    (cl:cons ':turn_value (turn_value msg))
    (cl:cons ':drive_value (drive_value msg))
    (cl:cons ':mower_height (mower_height msg))
    (cl:cons ':left_wheel_speed (left_wheel_speed msg))
    (cl:cons ':right_wheel_speed (right_wheel_speed msg))
    (cl:cons ':speed_cmd (speed_cmd msg))
))
