; Auto-generated. Do not edit!


(cl:in-package util-msg)


;//! \htmlinclude DiagnosticResult.msg.html

(cl:defclass <DiagnosticResult> (roslisp-msg-protocol:ros-message)
  ((header
    :reader header
    :initarg :header
    :type std_msgs-msg:Header
    :initform (cl:make-instance 'std_msgs-msg:Header))
   (fault_code
    :reader fault_code
    :initarg :fault_code
    :type cl:fixnum
    :initform 0))
)

(cl:defclass DiagnosticResult (<DiagnosticResult>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <DiagnosticResult>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'DiagnosticResult)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name util-msg:<DiagnosticResult> is deprecated: use util-msg:DiagnosticResult instead.")))

(cl:ensure-generic-function 'header-val :lambda-list '(m))
(cl:defmethod header-val ((m <DiagnosticResult>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:header-val is deprecated.  Use util-msg:header instead.")
  (header m))

(cl:ensure-generic-function 'fault_code-val :lambda-list '(m))
(cl:defmethod fault_code-val ((m <DiagnosticResult>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:fault_code-val is deprecated.  Use util-msg:fault_code instead.")
  (fault_code m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <DiagnosticResult>) ostream)
  "Serializes a message object of type '<DiagnosticResult>"
  (roslisp-msg-protocol:serialize (cl:slot-value msg 'header) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'fault_code)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <DiagnosticResult>) istream)
  "Deserializes a message object of type '<DiagnosticResult>"
  (roslisp-msg-protocol:deserialize (cl:slot-value msg 'header) istream)
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'fault_code)) (cl:read-byte istream))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<DiagnosticResult>)))
  "Returns string type for a message object of type '<DiagnosticResult>"
  "util/DiagnosticResult")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'DiagnosticResult)))
  "Returns string type for a message object of type 'DiagnosticResult"
  "util/DiagnosticResult")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<DiagnosticResult>)))
  "Returns md5sum for a message object of type '<DiagnosticResult>"
  "d380b94db25b373da53086abf486f7d3")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'DiagnosticResult)))
  "Returns md5sum for a message object of type 'DiagnosticResult"
  "d380b94db25b373da53086abf486f7d3")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<DiagnosticResult>)))
  "Returns full string definition for message of type '<DiagnosticResult>"
  (cl:format cl:nil "Header header~%~%# Diagnostic status code~%# 0: Normal operation~%# 1: No position data (/Mower/position)~%# 2: Stop car command received (/mower/stop_car=true)~%# 3: Emergency stop for obstacle avoidance (/lawn_mower/avoid_state==3)~%# 4: No global path (/lawn_mower/global_path)~%# 5: PID controller node not running (pid_controller node)~%# 6: CAN communication buffer full (fallback check when others are normal)~%uint8 fault_code ~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'DiagnosticResult)))
  "Returns full string definition for message of type 'DiagnosticResult"
  (cl:format cl:nil "Header header~%~%# Diagnostic status code~%# 0: Normal operation~%# 1: No position data (/Mower/position)~%# 2: Stop car command received (/mower/stop_car=true)~%# 3: Emergency stop for obstacle avoidance (/lawn_mower/avoid_state==3)~%# 4: No global path (/lawn_mower/global_path)~%# 5: PID controller node not running (pid_controller node)~%# 6: CAN communication buffer full (fallback check when others are normal)~%uint8 fault_code ~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <DiagnosticResult>))
  (cl:+ 0
     (roslisp-msg-protocol:serialization-length (cl:slot-value msg 'header))
     1
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <DiagnosticResult>))
  "Converts a ROS message object to a list"
  (cl:list 'DiagnosticResult
    (cl:cons ':header (header msg))
    (cl:cons ':fault_code (fault_code msg))
))
