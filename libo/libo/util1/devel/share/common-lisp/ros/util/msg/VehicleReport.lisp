; Auto-generated. Do not edit!


(cl:in-package util-msg)


;//! \htmlinclude VehicleReport.msg.html

(cl:defclass <VehicleReport> (roslisp-msg-protocol:ros-message)
  ((header
    :reader header
    :initarg :header
    :type std_msgs-msg:Header
    :initform (cl:make-instance 'std_msgs-msg:Header))
   (vehicle_gear
    :reader vehicle_gear
    :initarg :vehicle_gear
    :type cl:fixnum
    :initform 0)
   (clamping_brake_status
    :reader clamping_brake_status
    :initarg :clamping_brake_status
    :type cl:fixnum
    :initform 0)
   (drive_mode_state
    :reader drive_mode_state
    :initarg :drive_mode_state
    :type cl:fixnum
    :initform 0)
   (vcu_speed_req
    :reader vcu_speed_req
    :initarg :vcu_speed_req
    :type cl:float
    :initform 0.0)
   (vehicle_soc
    :reader vehicle_soc
    :initarg :vehicle_soc
    :type cl:fixnum
    :initform 0)
   (vehicle_status_msgcntr
    :reader vehicle_status_msgcntr
    :initarg :vehicle_status_msgcntr
    :type cl:fixnum
    :initform 0)
   (vehicle_speed
    :reader vehicle_speed
    :initarg :vehicle_speed
    :type cl:float
    :initform 0.0)
   (vehicle_brake_pressure
    :reader vehicle_brake_pressure
    :initarg :vehicle_brake_pressure
    :type cl:float
    :initform 0.0)
   (vehicle_steering_angle
    :reader vehicle_steering_angle
    :initarg :vehicle_steering_angle
    :type cl:float
    :initform 0.0)
   (vehicle_status_2_msgcntr
    :reader vehicle_status_2_msgcntr
    :initarg :vehicle_status_2_msgcntr
    :type cl:float
    :initform 0.0))
)

(cl:defclass VehicleReport (<VehicleReport>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <VehicleReport>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'VehicleReport)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name util-msg:<VehicleReport> is deprecated: use util-msg:VehicleReport instead.")))

(cl:ensure-generic-function 'header-val :lambda-list '(m))
(cl:defmethod header-val ((m <VehicleReport>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:header-val is deprecated.  Use util-msg:header instead.")
  (header m))

(cl:ensure-generic-function 'vehicle_gear-val :lambda-list '(m))
(cl:defmethod vehicle_gear-val ((m <VehicleReport>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:vehicle_gear-val is deprecated.  Use util-msg:vehicle_gear instead.")
  (vehicle_gear m))

(cl:ensure-generic-function 'clamping_brake_status-val :lambda-list '(m))
(cl:defmethod clamping_brake_status-val ((m <VehicleReport>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:clamping_brake_status-val is deprecated.  Use util-msg:clamping_brake_status instead.")
  (clamping_brake_status m))

(cl:ensure-generic-function 'drive_mode_state-val :lambda-list '(m))
(cl:defmethod drive_mode_state-val ((m <VehicleReport>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:drive_mode_state-val is deprecated.  Use util-msg:drive_mode_state instead.")
  (drive_mode_state m))

(cl:ensure-generic-function 'vcu_speed_req-val :lambda-list '(m))
(cl:defmethod vcu_speed_req-val ((m <VehicleReport>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:vcu_speed_req-val is deprecated.  Use util-msg:vcu_speed_req instead.")
  (vcu_speed_req m))

(cl:ensure-generic-function 'vehicle_soc-val :lambda-list '(m))
(cl:defmethod vehicle_soc-val ((m <VehicleReport>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:vehicle_soc-val is deprecated.  Use util-msg:vehicle_soc instead.")
  (vehicle_soc m))

(cl:ensure-generic-function 'vehicle_status_msgcntr-val :lambda-list '(m))
(cl:defmethod vehicle_status_msgcntr-val ((m <VehicleReport>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:vehicle_status_msgcntr-val is deprecated.  Use util-msg:vehicle_status_msgcntr instead.")
  (vehicle_status_msgcntr m))

(cl:ensure-generic-function 'vehicle_speed-val :lambda-list '(m))
(cl:defmethod vehicle_speed-val ((m <VehicleReport>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:vehicle_speed-val is deprecated.  Use util-msg:vehicle_speed instead.")
  (vehicle_speed m))

(cl:ensure-generic-function 'vehicle_brake_pressure-val :lambda-list '(m))
(cl:defmethod vehicle_brake_pressure-val ((m <VehicleReport>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:vehicle_brake_pressure-val is deprecated.  Use util-msg:vehicle_brake_pressure instead.")
  (vehicle_brake_pressure m))

(cl:ensure-generic-function 'vehicle_steering_angle-val :lambda-list '(m))
(cl:defmethod vehicle_steering_angle-val ((m <VehicleReport>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:vehicle_steering_angle-val is deprecated.  Use util-msg:vehicle_steering_angle instead.")
  (vehicle_steering_angle m))

(cl:ensure-generic-function 'vehicle_status_2_msgcntr-val :lambda-list '(m))
(cl:defmethod vehicle_status_2_msgcntr-val ((m <VehicleReport>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:vehicle_status_2_msgcntr-val is deprecated.  Use util-msg:vehicle_status_2_msgcntr instead.")
  (vehicle_status_2_msgcntr m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <VehicleReport>) ostream)
  "Serializes a message object of type '<VehicleReport>"
  (roslisp-msg-protocol:serialize (cl:slot-value msg 'header) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'vehicle_gear)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'clamping_brake_status)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'drive_mode_state)) ostream)
  (cl:let ((bits (roslisp-utils:encode-single-float-bits (cl:slot-value msg 'vcu_speed_req))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'vehicle_soc)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'vehicle_status_msgcntr)) ostream)
  (cl:let ((bits (roslisp-utils:encode-single-float-bits (cl:slot-value msg 'vehicle_speed))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream))
  (cl:let ((bits (roslisp-utils:encode-single-float-bits (cl:slot-value msg 'vehicle_brake_pressure))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream))
  (cl:let ((bits (roslisp-utils:encode-single-float-bits (cl:slot-value msg 'vehicle_steering_angle))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream))
  (cl:let ((bits (roslisp-utils:encode-single-float-bits (cl:slot-value msg 'vehicle_status_2_msgcntr))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <VehicleReport>) istream)
  "Deserializes a message object of type '<VehicleReport>"
  (roslisp-msg-protocol:deserialize (cl:slot-value msg 'header) istream)
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'vehicle_gear)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'clamping_brake_status)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'drive_mode_state)) (cl:read-byte istream))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'vcu_speed_req) (roslisp-utils:decode-single-float-bits bits)))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'vehicle_soc)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'vehicle_status_msgcntr)) (cl:read-byte istream))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'vehicle_speed) (roslisp-utils:decode-single-float-bits bits)))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'vehicle_brake_pressure) (roslisp-utils:decode-single-float-bits bits)))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'vehicle_steering_angle) (roslisp-utils:decode-single-float-bits bits)))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'vehicle_status_2_msgcntr) (roslisp-utils:decode-single-float-bits bits)))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<VehicleReport>)))
  "Returns string type for a message object of type '<VehicleReport>"
  "util/VehicleReport")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'VehicleReport)))
  "Returns string type for a message object of type 'VehicleReport"
  "util/VehicleReport")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<VehicleReport>)))
  "Returns md5sum for a message object of type '<VehicleReport>"
  "8141fbcf1c26eb79107a5ea125eaf0a9")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'VehicleReport)))
  "Returns md5sum for a message object of type 'VehicleReport"
  "8141fbcf1c26eb79107a5ea125eaf0a9")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<VehicleReport>)))
  "Returns full string definition for message of type '<VehicleReport>"
  (cl:format cl:nil "Header header~%~%# 0x303~%uint8 vehicle_gear~%uint8 clamping_brake_status~%uint8 drive_mode_state~%float32 vcu_speed_req~%uint8 vehicle_soc~%uint8 vehicle_status_msgcntr~%~%# 0x304~%float32 vehicle_speed~%float32 vehicle_brake_pressure~%float32 vehicle_steering_angle~%float32 vehicle_status_2_msgcntr~%~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'VehicleReport)))
  "Returns full string definition for message of type 'VehicleReport"
  (cl:format cl:nil "Header header~%~%# 0x303~%uint8 vehicle_gear~%uint8 clamping_brake_status~%uint8 drive_mode_state~%float32 vcu_speed_req~%uint8 vehicle_soc~%uint8 vehicle_status_msgcntr~%~%# 0x304~%float32 vehicle_speed~%float32 vehicle_brake_pressure~%float32 vehicle_steering_angle~%float32 vehicle_status_2_msgcntr~%~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <VehicleReport>))
  (cl:+ 0
     (roslisp-msg-protocol:serialization-length (cl:slot-value msg 'header))
     1
     1
     1
     4
     1
     1
     4
     4
     4
     4
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <VehicleReport>))
  "Converts a ROS message object to a list"
  (cl:list 'VehicleReport
    (cl:cons ':header (header msg))
    (cl:cons ':vehicle_gear (vehicle_gear msg))
    (cl:cons ':clamping_brake_status (clamping_brake_status msg))
    (cl:cons ':drive_mode_state (drive_mode_state msg))
    (cl:cons ':vcu_speed_req (vcu_speed_req msg))
    (cl:cons ':vehicle_soc (vehicle_soc msg))
    (cl:cons ':vehicle_status_msgcntr (vehicle_status_msgcntr msg))
    (cl:cons ':vehicle_speed (vehicle_speed msg))
    (cl:cons ':vehicle_brake_pressure (vehicle_brake_pressure msg))
    (cl:cons ':vehicle_steering_angle (vehicle_steering_angle msg))
    (cl:cons ':vehicle_status_2_msgcntr (vehicle_status_2_msgcntr msg))
))
