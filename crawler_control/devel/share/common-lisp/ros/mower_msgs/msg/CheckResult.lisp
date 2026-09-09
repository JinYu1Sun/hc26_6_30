; Auto-generated. Do not edit!


(cl:in-package mower_msgs-msg)


;//! \htmlinclude CheckResult.msg.html

(cl:defclass <CheckResult> (roslisp-msg-protocol:ros-message)
  ((is_checkresult_ok
    :reader is_checkresult_ok
    :initarg :is_checkresult_ok
    :type cl:boolean
    :initform cl:nil))
)

(cl:defclass CheckResult (<CheckResult>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <CheckResult>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'CheckResult)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name mower_msgs-msg:<CheckResult> is deprecated: use mower_msgs-msg:CheckResult instead.")))

(cl:ensure-generic-function 'is_checkresult_ok-val :lambda-list '(m))
(cl:defmethod is_checkresult_ok-val ((m <CheckResult>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:is_checkresult_ok-val is deprecated.  Use mower_msgs-msg:is_checkresult_ok instead.")
  (is_checkresult_ok m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <CheckResult>) ostream)
  "Serializes a message object of type '<CheckResult>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'is_checkresult_ok) 1 0)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <CheckResult>) istream)
  "Deserializes a message object of type '<CheckResult>"
    (cl:setf (cl:slot-value msg 'is_checkresult_ok) (cl:not (cl:zerop (cl:read-byte istream))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<CheckResult>)))
  "Returns string type for a message object of type '<CheckResult>"
  "mower_msgs/CheckResult")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'CheckResult)))
  "Returns string type for a message object of type 'CheckResult"
  "mower_msgs/CheckResult")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<CheckResult>)))
  "Returns md5sum for a message object of type '<CheckResult>"
  "d65d3e1f019288ca453b865d8cb24468")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'CheckResult)))
  "Returns md5sum for a message object of type 'CheckResult"
  "d65d3e1f019288ca453b865d8cb24468")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<CheckResult>)))
  "Returns full string definition for message of type '<CheckResult>"
  (cl:format cl:nil "bool is_checkresult_ok~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'CheckResult)))
  "Returns full string definition for message of type 'CheckResult"
  (cl:format cl:nil "bool is_checkresult_ok~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <CheckResult>))
  (cl:+ 0
     1
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <CheckResult>))
  "Converts a ROS message object to a list"
  (cl:list 'CheckResult
    (cl:cons ':is_checkresult_ok (is_checkresult_ok msg))
))
