; Auto-generated. Do not edit!


(cl:in-package mower_msgs-msg)


;//! \htmlinclude TaskStatus.msg.html

(cl:defclass <TaskStatus> (roslisp-msg-protocol:ros-message)
  ((task_status
    :reader task_status
    :initarg :task_status
    :type cl:string
    :initform ""))
)

(cl:defclass TaskStatus (<TaskStatus>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <TaskStatus>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'TaskStatus)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name mower_msgs-msg:<TaskStatus> is deprecated: use mower_msgs-msg:TaskStatus instead.")))

(cl:ensure-generic-function 'task_status-val :lambda-list '(m))
(cl:defmethod task_status-val ((m <TaskStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:task_status-val is deprecated.  Use mower_msgs-msg:task_status instead.")
  (task_status m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <TaskStatus>) ostream)
  "Serializes a message object of type '<TaskStatus>"
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'task_status))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'task_status))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <TaskStatus>) istream)
  "Deserializes a message object of type '<TaskStatus>"
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'task_status) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'task_status) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<TaskStatus>)))
  "Returns string type for a message object of type '<TaskStatus>"
  "mower_msgs/TaskStatus")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'TaskStatus)))
  "Returns string type for a message object of type 'TaskStatus"
  "mower_msgs/TaskStatus")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<TaskStatus>)))
  "Returns md5sum for a message object of type '<TaskStatus>"
  "53109e0b2984975887b08ba65c603762")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'TaskStatus)))
  "Returns md5sum for a message object of type 'TaskStatus"
  "53109e0b2984975887b08ba65c603762")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<TaskStatus>)))
  "Returns full string definition for message of type '<TaskStatus>"
  (cl:format cl:nil "string task_status~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'TaskStatus)))
  "Returns full string definition for message of type 'TaskStatus"
  (cl:format cl:nil "string task_status~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <TaskStatus>))
  (cl:+ 0
     4 (cl:length (cl:slot-value msg 'task_status))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <TaskStatus>))
  "Converts a ROS message object to a list"
  (cl:list 'TaskStatus
    (cl:cons ':task_status (task_status msg))
))
