; Auto-generated. Do not edit!


(cl:in-package util-msg)


;//! \htmlinclude PlanningMapPoint.msg.html

(cl:defclass <PlanningMapPoint> (roslisp-msg-protocol:ros-message)
  ((header
    :reader header
    :initarg :header
    :type std_msgs-msg:Header
    :initform (cl:make-instance 'std_msgs-msg:Header))
   (global_x
    :reader global_x
    :initarg :global_x
    :type cl:float
    :initform 0.0)
   (global_y
    :reader global_y
    :initarg :global_y
    :type cl:float
    :initform 0.0)
   (obstacle_status
    :reader obstacle_status
    :initarg :obstacle_status
    :type cl:boolean
    :initform cl:nil)
   (track_status
    :reader track_status
    :initarg :track_status
    :type cl:boolean
    :initform cl:nil))
)

(cl:defclass PlanningMapPoint (<PlanningMapPoint>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <PlanningMapPoint>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'PlanningMapPoint)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name util-msg:<PlanningMapPoint> is deprecated: use util-msg:PlanningMapPoint instead.")))

(cl:ensure-generic-function 'header-val :lambda-list '(m))
(cl:defmethod header-val ((m <PlanningMapPoint>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:header-val is deprecated.  Use util-msg:header instead.")
  (header m))

(cl:ensure-generic-function 'global_x-val :lambda-list '(m))
(cl:defmethod global_x-val ((m <PlanningMapPoint>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:global_x-val is deprecated.  Use util-msg:global_x instead.")
  (global_x m))

(cl:ensure-generic-function 'global_y-val :lambda-list '(m))
(cl:defmethod global_y-val ((m <PlanningMapPoint>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:global_y-val is deprecated.  Use util-msg:global_y instead.")
  (global_y m))

(cl:ensure-generic-function 'obstacle_status-val :lambda-list '(m))
(cl:defmethod obstacle_status-val ((m <PlanningMapPoint>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:obstacle_status-val is deprecated.  Use util-msg:obstacle_status instead.")
  (obstacle_status m))

(cl:ensure-generic-function 'track_status-val :lambda-list '(m))
(cl:defmethod track_status-val ((m <PlanningMapPoint>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:track_status-val is deprecated.  Use util-msg:track_status instead.")
  (track_status m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <PlanningMapPoint>) ostream)
  "Serializes a message object of type '<PlanningMapPoint>"
  (roslisp-msg-protocol:serialize (cl:slot-value msg 'header) ostream)
  (cl:let ((bits (roslisp-utils:encode-double-float-bits (cl:slot-value msg 'global_x))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 32) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 40) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 48) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 56) bits) ostream))
  (cl:let ((bits (roslisp-utils:encode-double-float-bits (cl:slot-value msg 'global_y))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 32) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 40) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 48) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 56) bits) ostream))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'obstacle_status) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'track_status) 1 0)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <PlanningMapPoint>) istream)
  "Deserializes a message object of type '<PlanningMapPoint>"
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
    (cl:setf (cl:slot-value msg 'global_x) (roslisp-utils:decode-double-float-bits bits)))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 32) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 40) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 48) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 56) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'global_y) (roslisp-utils:decode-double-float-bits bits)))
    (cl:setf (cl:slot-value msg 'obstacle_status) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:slot-value msg 'track_status) (cl:not (cl:zerop (cl:read-byte istream))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<PlanningMapPoint>)))
  "Returns string type for a message object of type '<PlanningMapPoint>"
  "util/PlanningMapPoint")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'PlanningMapPoint)))
  "Returns string type for a message object of type 'PlanningMapPoint"
  "util/PlanningMapPoint")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<PlanningMapPoint>)))
  "Returns md5sum for a message object of type '<PlanningMapPoint>"
  "ba8804762a910657104542279037ab6a")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'PlanningMapPoint)))
  "Returns md5sum for a message object of type 'PlanningMapPoint"
  "ba8804762a910657104542279037ab6a")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<PlanningMapPoint>)))
  "Returns full string definition for message of type '<PlanningMapPoint>"
  (cl:format cl:nil "Header header~%~%# Global_Map_Point~%float64 global_x~%float64 global_y~%bool obstacle_status~%bool track_status~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'PlanningMapPoint)))
  "Returns full string definition for message of type 'PlanningMapPoint"
  (cl:format cl:nil "Header header~%~%# Global_Map_Point~%float64 global_x~%float64 global_y~%bool obstacle_status~%bool track_status~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <PlanningMapPoint>))
  (cl:+ 0
     (roslisp-msg-protocol:serialization-length (cl:slot-value msg 'header))
     8
     8
     1
     1
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <PlanningMapPoint>))
  "Converts a ROS message object to a list"
  (cl:list 'PlanningMapPoint
    (cl:cons ':header (header msg))
    (cl:cons ':global_x (global_x msg))
    (cl:cons ':global_y (global_y msg))
    (cl:cons ':obstacle_status (obstacle_status msg))
    (cl:cons ':track_status (track_status msg))
))
