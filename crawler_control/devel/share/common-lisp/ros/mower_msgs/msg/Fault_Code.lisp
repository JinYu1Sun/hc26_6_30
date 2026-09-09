; Auto-generated. Do not edit!


(cl:in-package mower_msgs-msg)


;//! \htmlinclude Fault_Code.msg.html

(cl:defclass <Fault_Code> (roslisp-msg-protocol:ros-message)
  ((fault_code
    :reader fault_code
    :initarg :fault_code
    :type (cl:vector cl:string)
   :initform (cl:make-array 13 :element-type 'cl:string :initial-element "")))
)

(cl:defclass Fault_Code (<Fault_Code>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <Fault_Code>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'Fault_Code)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name mower_msgs-msg:<Fault_Code> is deprecated: use mower_msgs-msg:Fault_Code instead.")))

(cl:ensure-generic-function 'fault_code-val :lambda-list '(m))
(cl:defmethod fault_code-val ((m <Fault_Code>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:fault_code-val is deprecated.  Use mower_msgs-msg:fault_code instead.")
  (fault_code m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <Fault_Code>) ostream)
  "Serializes a message object of type '<Fault_Code>"
  (cl:map cl:nil #'(cl:lambda (ele) (cl:let ((__ros_str_len (cl:length ele)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) ele))
   (cl:slot-value msg 'fault_code))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <Fault_Code>) istream)
  "Deserializes a message object of type '<Fault_Code>"
  (cl:setf (cl:slot-value msg 'fault_code) (cl:make-array 13))
  (cl:let ((vals (cl:slot-value msg 'fault_code)))
    (cl:dotimes (i 13)
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:aref vals i) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:aref vals i) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<Fault_Code>)))
  "Returns string type for a message object of type '<Fault_Code>"
  "mower_msgs/Fault_Code")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'Fault_Code)))
  "Returns string type for a message object of type 'Fault_Code"
  "mower_msgs/Fault_Code")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<Fault_Code>)))
  "Returns md5sum for a message object of type '<Fault_Code>"
  "42556c67ec9fdf90479d909abb5c32a1")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'Fault_Code)))
  "Returns md5sum for a message object of type 'Fault_Code"
  "42556c67ec9fdf90479d909abb5c32a1")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<Fault_Code>)))
  "Returns full string definition for message of type '<Fault_Code>"
  (cl:format cl:nil "string[13] fault_code~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'Fault_Code)))
  "Returns full string definition for message of type 'Fault_Code"
  (cl:format cl:nil "string[13] fault_code~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <Fault_Code>))
  (cl:+ 0
     0 (cl:reduce #'cl:+ (cl:slot-value msg 'fault_code) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 4 (cl:length ele))))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <Fault_Code>))
  "Converts a ROS message object to a list"
  (cl:list 'Fault_Code
    (cl:cons ':fault_code (fault_code msg))
))
