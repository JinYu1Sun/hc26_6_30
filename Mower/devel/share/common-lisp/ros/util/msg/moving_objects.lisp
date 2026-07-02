; Auto-generated. Do not edit!


(cl:in-package util-msg)


;//! \htmlinclude moving_objects.msg.html

(cl:defclass <moving_objects> (roslisp-msg-protocol:ros-message)
  ((header
    :reader header
    :initarg :header
    :type std_msgs-msg:Header
    :initform (cl:make-instance 'std_msgs-msg:Header))
   (num
    :reader num
    :initarg :num
    :type cl:integer
    :initform 0)
   (moving_obj
    :reader moving_obj
    :initarg :moving_obj
    :type (cl:vector util-msg:moving_object)
   :initform (cl:make-array 0 :element-type 'util-msg:moving_object :initial-element (cl:make-instance 'util-msg:moving_object))))
)

(cl:defclass moving_objects (<moving_objects>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <moving_objects>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'moving_objects)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name util-msg:<moving_objects> is deprecated: use util-msg:moving_objects instead.")))

(cl:ensure-generic-function 'header-val :lambda-list '(m))
(cl:defmethod header-val ((m <moving_objects>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:header-val is deprecated.  Use util-msg:header instead.")
  (header m))

(cl:ensure-generic-function 'num-val :lambda-list '(m))
(cl:defmethod num-val ((m <moving_objects>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:num-val is deprecated.  Use util-msg:num instead.")
  (num m))

(cl:ensure-generic-function 'moving_obj-val :lambda-list '(m))
(cl:defmethod moving_obj-val ((m <moving_objects>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:moving_obj-val is deprecated.  Use util-msg:moving_obj instead.")
  (moving_obj m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <moving_objects>) ostream)
  "Serializes a message object of type '<moving_objects>"
  (roslisp-msg-protocol:serialize (cl:slot-value msg 'header) ostream)
  (cl:let* ((signed (cl:slot-value msg 'num)) (unsigned (cl:if (cl:< signed 0) (cl:+ signed 4294967296) signed)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) unsigned) ostream)
    )
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'moving_obj))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (roslisp-msg-protocol:serialize ele ostream))
   (cl:slot-value msg 'moving_obj))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <moving_objects>) istream)
  "Deserializes a message object of type '<moving_objects>"
  (roslisp-msg-protocol:deserialize (cl:slot-value msg 'header) istream)
    (cl:let ((unsigned 0))
      (cl:setf (cl:ldb (cl:byte 8 0) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) unsigned) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'num) (cl:if (cl:< unsigned 2147483648) unsigned (cl:- unsigned 4294967296))))
  (cl:let ((__ros_arr_len 0))
    (cl:setf (cl:ldb (cl:byte 8 0) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) __ros_arr_len) (cl:read-byte istream))
  (cl:setf (cl:slot-value msg 'moving_obj) (cl:make-array __ros_arr_len))
  (cl:let ((vals (cl:slot-value msg 'moving_obj)))
    (cl:dotimes (i __ros_arr_len)
    (cl:setf (cl:aref vals i) (cl:make-instance 'util-msg:moving_object))
  (roslisp-msg-protocol:deserialize (cl:aref vals i) istream))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<moving_objects>)))
  "Returns string type for a message object of type '<moving_objects>"
  "util/moving_objects")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'moving_objects)))
  "Returns string type for a message object of type 'moving_objects"
  "util/moving_objects")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<moving_objects>)))
  "Returns md5sum for a message object of type '<moving_objects>"
  "3c433c9a0e5b7d198518fcd1e6805968")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'moving_objects)))
  "Returns md5sum for a message object of type 'moving_objects"
  "3c433c9a0e5b7d198518fcd1e6805968")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<moving_objects>)))
  "Returns full string definition for message of type '<moving_objects>"
  (cl:format cl:nil "Header header~%int32 num~%moving_object[] moving_obj~%~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%================================================================================~%MSG: util/moving_object~%#relative position, m~%float32 x1 # x_min~%float32 y1 # y_min~%float32 x2 # x_min~%float32 y2 # y_max~%float32 x3 # x_max~%float32 y3 # y_max~%float32 x4 # x_max~%float32 y4 # y_min~%~%# height, cm~%float32 zmin~%~%float32 zmax~%~%# absolute speed, m/s~%float32 speed~%~%# deg~%float32 speed_direction~%~%uint32 id~%string label~%~%# center point coordinate in localpose frame, m~%float32 global_x~%float32 global_y~%~%float32 local_x~%float32 local_y~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'moving_objects)))
  "Returns full string definition for message of type 'moving_objects"
  (cl:format cl:nil "Header header~%int32 num~%moving_object[] moving_obj~%~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%================================================================================~%MSG: util/moving_object~%#relative position, m~%float32 x1 # x_min~%float32 y1 # y_min~%float32 x2 # x_min~%float32 y2 # y_max~%float32 x3 # x_max~%float32 y3 # y_max~%float32 x4 # x_max~%float32 y4 # y_min~%~%# height, cm~%float32 zmin~%~%float32 zmax~%~%# absolute speed, m/s~%float32 speed~%~%# deg~%float32 speed_direction~%~%uint32 id~%string label~%~%# center point coordinate in localpose frame, m~%float32 global_x~%float32 global_y~%~%float32 local_x~%float32 local_y~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <moving_objects>))
  (cl:+ 0
     (roslisp-msg-protocol:serialization-length (cl:slot-value msg 'header))
     4
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'moving_obj) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ (roslisp-msg-protocol:serialization-length ele))))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <moving_objects>))
  "Converts a ROS message object to a list"
  (cl:list 'moving_objects
    (cl:cons ':header (header msg))
    (cl:cons ':num (num msg))
    (cl:cons ':moving_obj (moving_obj msg))
))
