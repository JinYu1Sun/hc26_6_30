; Auto-generated. Do not edit!


(cl:in-package util-msg)


;//! \htmlinclude VehicleInfoTest.msg.html

(cl:defclass <VehicleInfoTest> (roslisp-msg-protocol:ros-message)
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
   (angle_status
    :reader angle_status
    :initarg :angle_status
    :type cl:float
    :initform 0.0))
)

(cl:defclass VehicleInfoTest (<VehicleInfoTest>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <VehicleInfoTest>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'VehicleInfoTest)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name util-msg:<VehicleInfoTest> is deprecated: use util-msg:VehicleInfoTest instead.")))

(cl:ensure-generic-function 'header-val :lambda-list '(m))
(cl:defmethod header-val ((m <VehicleInfoTest>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:header-val is deprecated.  Use util-msg:header instead.")
  (header m))

(cl:ensure-generic-function 'speed_status-val :lambda-list '(m))
(cl:defmethod speed_status-val ((m <VehicleInfoTest>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:speed_status-val is deprecated.  Use util-msg:speed_status instead.")
  (speed_status m))

(cl:ensure-generic-function 'angle_status-val :lambda-list '(m))
(cl:defmethod angle_status-val ((m <VehicleInfoTest>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:angle_status-val is deprecated.  Use util-msg:angle_status instead.")
  (angle_status m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <VehicleInfoTest>) ostream)
  "Serializes a message object of type '<VehicleInfoTest>"
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
  (cl:let ((bits (roslisp-utils:encode-double-float-bits (cl:slot-value msg 'angle_status))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 32) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 40) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 48) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 56) bits) ostream))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <VehicleInfoTest>) istream)
  "Deserializes a message object of type '<VehicleInfoTest>"
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
    (cl:setf (cl:slot-value msg 'angle_status) (roslisp-utils:decode-double-float-bits bits)))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<VehicleInfoTest>)))
  "Returns string type for a message object of type '<VehicleInfoTest>"
  "util/VehicleInfoTest")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'VehicleInfoTest)))
  "Returns string type for a message object of type 'VehicleInfoTest"
  "util/VehicleInfoTest")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<VehicleInfoTest>)))
  "Returns md5sum for a message object of type '<VehicleInfoTest>"
  "9c82651e7ec09ce256b0ec03b6f2bab7")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'VehicleInfoTest)))
  "Returns md5sum for a message object of type 'VehicleInfoTest"
  "9c82651e7ec09ce256b0ec03b6f2bab7")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<VehicleInfoTest>)))
  "Returns full string definition for message of type '<VehicleInfoTest>"
  (cl:format cl:nil "Header header~%~%# AGVStatus 0x201~%float64 speed_status~%float64 angle_status~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'VehicleInfoTest)))
  "Returns full string definition for message of type 'VehicleInfoTest"
  (cl:format cl:nil "Header header~%~%# AGVStatus 0x201~%float64 speed_status~%float64 angle_status~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <VehicleInfoTest>))
  (cl:+ 0
     (roslisp-msg-protocol:serialization-length (cl:slot-value msg 'header))
     8
     8
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <VehicleInfoTest>))
  "Converts a ROS message object to a list"
  (cl:list 'VehicleInfoTest
    (cl:cons ':header (header msg))
    (cl:cons ':speed_status (speed_status msg))
    (cl:cons ':angle_status (angle_status msg))
))
