; Auto-generated. Do not edit!


(cl:in-package util-msg)


;//! \htmlinclude VehicleControl.msg.html

(cl:defclass <VehicleControl> (roslisp-msg-protocol:ros-message)
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

(cl:defclass VehicleControl (<VehicleControl>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <VehicleControl>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'VehicleControl)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name util-msg:<VehicleControl> is deprecated: use util-msg:VehicleControl instead.")))

(cl:ensure-generic-function 'header-val :lambda-list '(m))
(cl:defmethod header-val ((m <VehicleControl>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:header-val is deprecated.  Use util-msg:header instead.")
  (header m))

(cl:ensure-generic-function 'mover_bool-val :lambda-list '(m))
(cl:defmethod mover_bool-val ((m <VehicleControl>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:mover_bool-val is deprecated.  Use util-msg:mover_bool instead.")
  (mover_bool m))

(cl:ensure-generic-function 'ad_control_enable-val :lambda-list '(m))
(cl:defmethod ad_control_enable-val ((m <VehicleControl>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:ad_control_enable-val is deprecated.  Use util-msg:ad_control_enable instead.")
  (ad_control_enable m))

(cl:ensure-generic-function 'gear_model-val :lambda-list '(m))
(cl:defmethod gear_model-val ((m <VehicleControl>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:gear_model-val is deprecated.  Use util-msg:gear_model instead.")
  (gear_model m))

(cl:ensure-generic-function 'turn_value-val :lambda-list '(m))
(cl:defmethod turn_value-val ((m <VehicleControl>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:turn_value-val is deprecated.  Use util-msg:turn_value instead.")
  (turn_value m))

(cl:ensure-generic-function 'drive_value-val :lambda-list '(m))
(cl:defmethod drive_value-val ((m <VehicleControl>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:drive_value-val is deprecated.  Use util-msg:drive_value instead.")
  (drive_value m))

(cl:ensure-generic-function 'mower_height-val :lambda-list '(m))
(cl:defmethod mower_height-val ((m <VehicleControl>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:mower_height-val is deprecated.  Use util-msg:mower_height instead.")
  (mower_height m))

(cl:ensure-generic-function 'left_wheel_speed-val :lambda-list '(m))
(cl:defmethod left_wheel_speed-val ((m <VehicleControl>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:left_wheel_speed-val is deprecated.  Use util-msg:left_wheel_speed instead.")
  (left_wheel_speed m))

(cl:ensure-generic-function 'right_wheel_speed-val :lambda-list '(m))
(cl:defmethod right_wheel_speed-val ((m <VehicleControl>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:right_wheel_speed-val is deprecated.  Use util-msg:right_wheel_speed instead.")
  (right_wheel_speed m))

(cl:ensure-generic-function 'speed_cmd-val :lambda-list '(m))
(cl:defmethod speed_cmd-val ((m <VehicleControl>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:speed_cmd-val is deprecated.  Use util-msg:speed_cmd instead.")
  (speed_cmd m))
(cl:defmethod roslisp-msg-protocol:symbol-codes ((msg-type (cl:eql '<VehicleControl>)))
    "Constants for message type '<VehicleControl>"
  '((:P_GEAR . 0)
    (:R_GEAR . 1)
    (:N_GEAR . 2)
    (:D_GEAR . 3))
)
(cl:defmethod roslisp-msg-protocol:symbol-codes ((msg-type (cl:eql 'VehicleControl)))
    "Constants for message type 'VehicleControl"
  '((:P_GEAR . 0)
    (:R_GEAR . 1)
    (:N_GEAR . 2)
    (:D_GEAR . 3))
)
(cl:defmethod roslisp-msg-protocol:serialize ((msg <VehicleControl>) ostream)
  "Serializes a message object of type '<VehicleControl>"
  (roslisp-msg-protocol:serialize (cl:slot-value msg 'header) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'mover_bool)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'ad_control_enable)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'gear_model)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'turn_value)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'turn_value)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'drive_value)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'drive_value)) ostream)
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
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <VehicleControl>) istream)
  "Deserializes a message object of type '<VehicleControl>"
  (roslisp-msg-protocol:deserialize (cl:slot-value msg 'header) istream)
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'mover_bool)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'ad_control_enable)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'gear_model)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'turn_value)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'turn_value)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'drive_value)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'drive_value)) (cl:read-byte istream))
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
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<VehicleControl>)))
  "Returns string type for a message object of type '<VehicleControl>"
  "util/VehicleControl")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'VehicleControl)))
  "Returns string type for a message object of type 'VehicleControl"
  "util/VehicleControl")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<VehicleControl>)))
  "Returns md5sum for a message object of type '<VehicleControl>"
  "b4cc34e4fcf9d1ca27ccdd1d58a91a49")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'VehicleControl)))
  "Returns md5sum for a message object of type 'VehicleControl"
  "b4cc34e4fcf9d1ca27ccdd1d58a91a49")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<VehicleControl>)))
  "Returns full string definition for message of type '<VehicleControl>"
  (cl:format cl:nil "Header header~%# VehicleCmd~%# Gear_model~%uint8 p_Gear = 0 # 0x00： P 档~%uint8 R_Gear = 1 # 0x01： 无~%uint8 N_Gear = 2 # 0x02： 转弯信号~%uint8 D_Gear = 3 # 0x03： D 前进档~%~%# ADCmd 0x310~%uint8 mover_bool~%uint8 ad_control_enable ~%uint8 gear_model~%uint16 turn_value~%uint16 drive_value~%uint8 mower_height~%~%float64 left_wheel_speed  # 无~%float64 right_wheel_speed # 无~%float64 speed_cmd   # int8 [-128~~127]~%~%~%~%~%~%~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'VehicleControl)))
  "Returns full string definition for message of type 'VehicleControl"
  (cl:format cl:nil "Header header~%# VehicleCmd~%# Gear_model~%uint8 p_Gear = 0 # 0x00： P 档~%uint8 R_Gear = 1 # 0x01： 无~%uint8 N_Gear = 2 # 0x02： 转弯信号~%uint8 D_Gear = 3 # 0x03： D 前进档~%~%# ADCmd 0x310~%uint8 mover_bool~%uint8 ad_control_enable ~%uint8 gear_model~%uint16 turn_value~%uint16 drive_value~%uint8 mower_height~%~%float64 left_wheel_speed  # 无~%float64 right_wheel_speed # 无~%float64 speed_cmd   # int8 [-128~~127]~%~%~%~%~%~%~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <VehicleControl>))
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
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <VehicleControl>))
  "Converts a ROS message object to a list"
  (cl:list 'VehicleControl
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
