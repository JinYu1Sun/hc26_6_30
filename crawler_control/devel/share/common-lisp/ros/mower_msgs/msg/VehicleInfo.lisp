; Auto-generated. Do not edit!


(cl:in-package mower_msgs-msg)


;//! \htmlinclude VehicleInfo.msg.html

(cl:defclass <VehicleInfo> (roslisp-msg-protocol:ros-message)
  ((header
    :reader header
    :initarg :header
    :type std_msgs-msg:Header
    :initform (cl:make-instance 'std_msgs-msg:Header))
   (speed_status
    :reader speed_status
    :initarg :speed_status
    :type cl:float
    :initform 0.0)
   (leftw_speed
    :reader leftw_speed
    :initarg :leftw_speed
    :type cl:float
    :initform 0.0)
   (rightw_speed
    :reader rightw_speed
    :initarg :rightw_speed
    :type cl:float
    :initform 0.0)
   (gear_model
    :reader gear_model
    :initarg :gear_model
    :type cl:fixnum
    :initform 0)
   (drive_mode
    :reader drive_mode
    :initarg :drive_mode
    :type cl:fixnum
    :initform 0)
   (mow_height
    :reader mow_height
    :initarg :mow_height
    :type cl:fixnum
    :initform 0)
   (battery_soc
    :reader battery_soc
    :initarg :battery_soc
    :type cl:fixnum
    :initform 0))
)

(cl:defclass VehicleInfo (<VehicleInfo>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <VehicleInfo>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'VehicleInfo)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name mower_msgs-msg:<VehicleInfo> is deprecated: use mower_msgs-msg:VehicleInfo instead.")))

(cl:ensure-generic-function 'header-val :lambda-list '(m))
(cl:defmethod header-val ((m <VehicleInfo>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:header-val is deprecated.  Use mower_msgs-msg:header instead.")
  (header m))

(cl:ensure-generic-function 'speed_status-val :lambda-list '(m))
(cl:defmethod speed_status-val ((m <VehicleInfo>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:speed_status-val is deprecated.  Use mower_msgs-msg:speed_status instead.")
  (speed_status m))

(cl:ensure-generic-function 'leftw_speed-val :lambda-list '(m))
(cl:defmethod leftw_speed-val ((m <VehicleInfo>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:leftw_speed-val is deprecated.  Use mower_msgs-msg:leftw_speed instead.")
  (leftw_speed m))

(cl:ensure-generic-function 'rightw_speed-val :lambda-list '(m))
(cl:defmethod rightw_speed-val ((m <VehicleInfo>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:rightw_speed-val is deprecated.  Use mower_msgs-msg:rightw_speed instead.")
  (rightw_speed m))

(cl:ensure-generic-function 'gear_model-val :lambda-list '(m))
(cl:defmethod gear_model-val ((m <VehicleInfo>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:gear_model-val is deprecated.  Use mower_msgs-msg:gear_model instead.")
  (gear_model m))

(cl:ensure-generic-function 'drive_mode-val :lambda-list '(m))
(cl:defmethod drive_mode-val ((m <VehicleInfo>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:drive_mode-val is deprecated.  Use mower_msgs-msg:drive_mode instead.")
  (drive_mode m))

(cl:ensure-generic-function 'mow_height-val :lambda-list '(m))
(cl:defmethod mow_height-val ((m <VehicleInfo>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:mow_height-val is deprecated.  Use mower_msgs-msg:mow_height instead.")
  (mow_height m))

(cl:ensure-generic-function 'battery_soc-val :lambda-list '(m))
(cl:defmethod battery_soc-val ((m <VehicleInfo>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:battery_soc-val is deprecated.  Use mower_msgs-msg:battery_soc instead.")
  (battery_soc m))
(cl:defmethod roslisp-msg-protocol:symbol-codes ((msg-type (cl:eql '<VehicleInfo>)))
    "Constants for message type '<VehicleInfo>"
  '((:P_GEAR . 0)
    (:R_GEAR . 1)
    (:N_GEAR . 2)
    (:D_GEAR . 3)
    (:MANUALCONTROL_MODE . 0)
    (:AUTOMATICCONTROL_MODE . 1)
    (:REMOTECONTROL_MODE . 2)
    (:LOW_MODE . 2)
    (:MEDIUM_MODE . 1)
    (:HIGH_MODE . 0))
)
(cl:defmethod roslisp-msg-protocol:symbol-codes ((msg-type (cl:eql 'VehicleInfo)))
    "Constants for message type 'VehicleInfo"
  '((:P_GEAR . 0)
    (:R_GEAR . 1)
    (:N_GEAR . 2)
    (:D_GEAR . 3)
    (:MANUALCONTROL_MODE . 0)
    (:AUTOMATICCONTROL_MODE . 1)
    (:REMOTECONTROL_MODE . 2)
    (:LOW_MODE . 2)
    (:MEDIUM_MODE . 1)
    (:HIGH_MODE . 0))
)
(cl:defmethod roslisp-msg-protocol:serialize ((msg <VehicleInfo>) ostream)
  "Serializes a message object of type '<VehicleInfo>"
  (roslisp-msg-protocol:serialize (cl:slot-value msg 'header) ostream)
  (cl:let ((bits (roslisp-utils:encode-double-float-bits (cl:slot-value msg 'speed_status))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 32) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 40) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 48) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 56) bits) ostream))
  (cl:let ((bits (roslisp-utils:encode-double-float-bits (cl:slot-value msg 'leftw_speed))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 32) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 40) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 48) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 56) bits) ostream))
  (cl:let ((bits (roslisp-utils:encode-double-float-bits (cl:slot-value msg 'rightw_speed))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 32) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 40) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 48) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 56) bits) ostream))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'gear_model)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'drive_mode)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'mow_height)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'battery_soc)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <VehicleInfo>) istream)
  "Deserializes a message object of type '<VehicleInfo>"
  (roslisp-msg-protocol:deserialize (cl:slot-value msg 'header) istream)
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 32) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 40) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 48) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 56) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'speed_status) (roslisp-utils:decode-double-float-bits bits)))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 32) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 40) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 48) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 56) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'leftw_speed) (roslisp-utils:decode-double-float-bits bits)))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 32) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 40) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 48) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 56) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'rightw_speed) (roslisp-utils:decode-double-float-bits bits)))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'gear_model)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'drive_mode)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'mow_height)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'battery_soc)) (cl:read-byte istream))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<VehicleInfo>)))
  "Returns string type for a message object of type '<VehicleInfo>"
  "mower_msgs/VehicleInfo")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'VehicleInfo)))
  "Returns string type for a message object of type 'VehicleInfo"
  "mower_msgs/VehicleInfo")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<VehicleInfo>)))
  "Returns md5sum for a message object of type '<VehicleInfo>"
  "8a2a4b2a095bc9355e18d7be4b50f3fd")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'VehicleInfo)))
  "Returns md5sum for a message object of type 'VehicleInfo"
  "8a2a4b2a095bc9355e18d7be4b50f3fd")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<VehicleInfo>)))
  "Returns full string definition for message of type '<VehicleInfo>"
  (cl:format cl:nil "Header header~%~%# AGVStatus 0x301  现履带车底控无反馈，均不成立~%float64 speed_status  ~%float64 leftw_speed   ~%float64 rightw_speed~%uint8 gear_model      ~%uint8 drive_mode~%uint8 mow_height~%uint8 battery_soc~%~%#gear_model~%uint8 P_Gear=0 # 0x00： P 档~%uint8 R_Gear=1 # 0x01： 倒档R~%uint8 N_Gear=2 # 0x02： N 档~%uint8 D_Gear=3 # 0x03： D 档~%#drive_model~%uint8 Manualcontrol_mode=0 # 0x00：手动控制模式~%uint8 Automaticcontrol_mode=1 # 0x01：自动控制模式~%uint8 Remotecontrol_mode=2 # 0x02：遥控器调试模式~%#mow_height~%uint8 Low_mode=2 # 0x02：割草高度为低~%uint8 Medium_mode=1 # 0x01：割草高度为中~%uint8 High_mode=0 # 0x00：割草高度为高~%~%~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'VehicleInfo)))
  "Returns full string definition for message of type 'VehicleInfo"
  (cl:format cl:nil "Header header~%~%# AGVStatus 0x301  现履带车底控无反馈，均不成立~%float64 speed_status  ~%float64 leftw_speed   ~%float64 rightw_speed~%uint8 gear_model      ~%uint8 drive_mode~%uint8 mow_height~%uint8 battery_soc~%~%#gear_model~%uint8 P_Gear=0 # 0x00： P 档~%uint8 R_Gear=1 # 0x01： 倒档R~%uint8 N_Gear=2 # 0x02： N 档~%uint8 D_Gear=3 # 0x03： D 档~%#drive_model~%uint8 Manualcontrol_mode=0 # 0x00：手动控制模式~%uint8 Automaticcontrol_mode=1 # 0x01：自动控制模式~%uint8 Remotecontrol_mode=2 # 0x02：遥控器调试模式~%#mow_height~%uint8 Low_mode=2 # 0x02：割草高度为低~%uint8 Medium_mode=1 # 0x01：割草高度为中~%uint8 High_mode=0 # 0x00：割草高度为高~%~%~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <VehicleInfo>))
  (cl:+ 0
     (roslisp-msg-protocol:serialization-length (cl:slot-value msg 'header))
     8
     8
     8
     1
     1
     1
     1
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <VehicleInfo>))
  "Converts a ROS message object to a list"
  (cl:list 'VehicleInfo
    (cl:cons ':header (header msg))
    (cl:cons ':speed_status (speed_status msg))
    (cl:cons ':leftw_speed (leftw_speed msg))
    (cl:cons ':rightw_speed (rightw_speed msg))
    (cl:cons ':gear_model (gear_model msg))
    (cl:cons ':drive_mode (drive_mode msg))
    (cl:cons ':mow_height (mow_height msg))
    (cl:cons ':battery_soc (battery_soc msg))
))
