; Auto-generated. Do not edit!


(cl:in-package mower_msgs-msg)


;//! \htmlinclude VslamState.msg.html

(cl:defclass <VslamState> (roslisp-msg-protocol:ros-message)
  ((is_vslam_ok
    :reader is_vslam_ok
    :initarg :is_vslam_ok
    :type cl:boolean
    :initform cl:nil))
)

(cl:defclass VslamState (<VslamState>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <VslamState>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'VslamState)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name mower_msgs-msg:<VslamState> is deprecated: use mower_msgs-msg:VslamState instead.")))

(cl:ensure-generic-function 'is_vslam_ok-val :lambda-list '(m))
(cl:defmethod is_vslam_ok-val ((m <VslamState>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:is_vslam_ok-val is deprecated.  Use mower_msgs-msg:is_vslam_ok instead.")
  (is_vslam_ok m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <VslamState>) ostream)
  "Serializes a message object of type '<VslamState>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'is_vslam_ok) 1 0)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <VslamState>) istream)
  "Deserializes a message object of type '<VslamState>"
    (cl:setf (cl:slot-value msg 'is_vslam_ok) (cl:not (cl:zerop (cl:read-byte istream))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<VslamState>)))
  "Returns string type for a message object of type '<VslamState>"
  "mower_msgs/VslamState")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'VslamState)))
  "Returns string type for a message object of type 'VslamState"
  "mower_msgs/VslamState")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<VslamState>)))
  "Returns md5sum for a message object of type '<VslamState>"
  "d58db3b3d9624dbcc385f025b40a612c")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'VslamState)))
  "Returns md5sum for a message object of type 'VslamState"
  "d58db3b3d9624dbcc385f025b40a612c")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<VslamState>)))
  "Returns full string definition for message of type '<VslamState>"
  (cl:format cl:nil "bool is_vslam_ok~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'VslamState)))
  "Returns full string definition for message of type 'VslamState"
  (cl:format cl:nil "bool is_vslam_ok~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <VslamState>))
  (cl:+ 0
     1
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <VslamState>))
  "Converts a ROS message object to a list"
  (cl:list 'VslamState
    (cl:cons ':is_vslam_ok (is_vslam_ok msg))
))
