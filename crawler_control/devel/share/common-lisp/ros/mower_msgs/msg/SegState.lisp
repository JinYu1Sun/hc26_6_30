; Auto-generated. Do not edit!


(cl:in-package mower_msgs-msg)


;//! \htmlinclude SegState.msg.html

(cl:defclass <SegState> (roslisp-msg-protocol:ros-message)
  ((is_seg_ok
    :reader is_seg_ok
    :initarg :is_seg_ok
    :type cl:boolean
    :initform cl:nil))
)

(cl:defclass SegState (<SegState>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <SegState>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'SegState)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name mower_msgs-msg:<SegState> is deprecated: use mower_msgs-msg:SegState instead.")))

(cl:ensure-generic-function 'is_seg_ok-val :lambda-list '(m))
(cl:defmethod is_seg_ok-val ((m <SegState>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:is_seg_ok-val is deprecated.  Use mower_msgs-msg:is_seg_ok instead.")
  (is_seg_ok m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <SegState>) ostream)
  "Serializes a message object of type '<SegState>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'is_seg_ok) 1 0)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <SegState>) istream)
  "Deserializes a message object of type '<SegState>"
    (cl:setf (cl:slot-value msg 'is_seg_ok) (cl:not (cl:zerop (cl:read-byte istream))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<SegState>)))
  "Returns string type for a message object of type '<SegState>"
  "mower_msgs/SegState")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'SegState)))
  "Returns string type for a message object of type 'SegState"
  "mower_msgs/SegState")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<SegState>)))
  "Returns md5sum for a message object of type '<SegState>"
  "2a12252519f1037241c6783756d61d0f")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'SegState)))
  "Returns md5sum for a message object of type 'SegState"
  "2a12252519f1037241c6783756d61d0f")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<SegState>)))
  "Returns full string definition for message of type '<SegState>"
  (cl:format cl:nil "bool is_seg_ok~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'SegState)))
  "Returns full string definition for message of type 'SegState"
  (cl:format cl:nil "bool is_seg_ok~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <SegState>))
  (cl:+ 0
     1
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <SegState>))
  "Converts a ROS message object to a list"
  (cl:list 'SegState
    (cl:cons ':is_seg_ok (is_seg_ok msg))
))
