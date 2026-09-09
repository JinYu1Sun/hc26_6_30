; Auto-generated. Do not edit!


(cl:in-package mower_msgs-msg)


;//! \htmlinclude Monitor.msg.html

(cl:defclass <Monitor> (roslisp-msg-protocol:ros-message)
  ((node_normal
    :reader node_normal
    :initarg :node_normal
    :type (cl:vector cl:boolean)
   :initform (cl:make-array 13 :element-type 'cl:boolean :initial-element cl:nil)))
)

(cl:defclass Monitor (<Monitor>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <Monitor>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'Monitor)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name mower_msgs-msg:<Monitor> is deprecated: use mower_msgs-msg:Monitor instead.")))

(cl:ensure-generic-function 'node_normal-val :lambda-list '(m))
(cl:defmethod node_normal-val ((m <Monitor>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:node_normal-val is deprecated.  Use mower_msgs-msg:node_normal instead.")
  (node_normal m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <Monitor>) ostream)
  "Serializes a message object of type '<Monitor>"
  (cl:map cl:nil #'(cl:lambda (ele) (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if ele 1 0)) ostream))
   (cl:slot-value msg 'node_normal))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <Monitor>) istream)
  "Deserializes a message object of type '<Monitor>"
  (cl:setf (cl:slot-value msg 'node_normal) (cl:make-array 13))
  (cl:let ((vals (cl:slot-value msg 'node_normal)))
    (cl:dotimes (i 13)
    (cl:setf (cl:aref vals i) (cl:not (cl:zerop (cl:read-byte istream))))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<Monitor>)))
  "Returns string type for a message object of type '<Monitor>"
  "mower_msgs/Monitor")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'Monitor)))
  "Returns string type for a message object of type 'Monitor"
  "mower_msgs/Monitor")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<Monitor>)))
  "Returns md5sum for a message object of type '<Monitor>"
  "8fa716a375e1d424b9eff8c8c0b57ef5")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'Monitor)))
  "Returns md5sum for a message object of type 'Monitor"
  "8fa716a375e1d424b9eff8c8c0b57ef5")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<Monitor>)))
  "Returns full string definition for message of type '<Monitor>"
  (cl:format cl:nil "bool[13] node_normal~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'Monitor)))
  "Returns full string definition for message of type 'Monitor"
  (cl:format cl:nil "bool[13] node_normal~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <Monitor>))
  (cl:+ 0
     0 (cl:reduce #'cl:+ (cl:slot-value msg 'node_normal) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 1)))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <Monitor>))
  "Converts a ROS message object to a list"
  (cl:list 'Monitor
    (cl:cons ':node_normal (node_normal msg))
))
