; Auto-generated. Do not edit!


(cl:in-package mower_msgs-srv)


;//! \htmlinclude Intervene-request.msg.html

(cl:defclass <Intervene-request> (roslisp-msg-protocol:ros-message)
  ((node_id
    :reader node_id
    :initarg :node_id
    :type cl:integer
    :initform 0)
   (node_status
    :reader node_status
    :initarg :node_status
    :type cl:integer
    :initform 0)
   (msg
    :reader msg
    :initarg :msg
    :type cl:string
    :initform ""))
)

(cl:defclass Intervene-request (<Intervene-request>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <Intervene-request>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'Intervene-request)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name mower_msgs-srv:<Intervene-request> is deprecated: use mower_msgs-srv:Intervene-request instead.")))

(cl:ensure-generic-function 'node_id-val :lambda-list '(m))
(cl:defmethod node_id-val ((m <Intervene-request>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-srv:node_id-val is deprecated.  Use mower_msgs-srv:node_id instead.")
  (node_id m))

(cl:ensure-generic-function 'node_status-val :lambda-list '(m))
(cl:defmethod node_status-val ((m <Intervene-request>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-srv:node_status-val is deprecated.  Use mower_msgs-srv:node_status instead.")
  (node_status m))

(cl:ensure-generic-function 'msg-val :lambda-list '(m))
(cl:defmethod msg-val ((m <Intervene-request>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-srv:msg-val is deprecated.  Use mower_msgs-srv:msg instead.")
  (msg m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <Intervene-request>) ostream)
  "Serializes a message object of type '<Intervene-request>"
  (cl:let* ((signed (cl:slot-value msg 'node_id)) (unsigned (cl:if (cl:< signed 0) (cl:+ signed 4294967296) signed)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) unsigned) ostream)
    )
  (cl:let* ((signed (cl:slot-value msg 'node_status)) (unsigned (cl:if (cl:< signed 0) (cl:+ signed 4294967296) signed)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) unsigned) ostream)
    )
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'msg))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'msg))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <Intervene-request>) istream)
  "Deserializes a message object of type '<Intervene-request>"
    (cl:let ((unsigned 0))
      (cl:setf (cl:ldb (cl:byte 8 0) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) unsigned) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'node_id) (cl:if (cl:< unsigned 2147483648) unsigned (cl:- unsigned 4294967296))))
    (cl:let ((unsigned 0))
      (cl:setf (cl:ldb (cl:byte 8 0) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) unsigned) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'node_status) (cl:if (cl:< unsigned 2147483648) unsigned (cl:- unsigned 4294967296))))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'msg) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'msg) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<Intervene-request>)))
  "Returns string type for a service object of type '<Intervene-request>"
  "mower_msgs/InterveneRequest")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'Intervene-request)))
  "Returns string type for a service object of type 'Intervene-request"
  "mower_msgs/InterveneRequest")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<Intervene-request>)))
  "Returns md5sum for a message object of type '<Intervene-request>"
  "15f1f9695525704904993d3c41cc8ed1")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'Intervene-request)))
  "Returns md5sum for a message object of type 'Intervene-request"
  "15f1f9695525704904993d3c41cc8ed1")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<Intervene-request>)))
  "Returns full string definition for message of type '<Intervene-request>"
  (cl:format cl:nil "int32  node_id~%int32  node_status~%string msg~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'Intervene-request)))
  "Returns full string definition for message of type 'Intervene-request"
  (cl:format cl:nil "int32  node_id~%int32  node_status~%string msg~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <Intervene-request>))
  (cl:+ 0
     4
     4
     4 (cl:length (cl:slot-value msg 'msg))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <Intervene-request>))
  "Converts a ROS message object to a list"
  (cl:list 'Intervene-request
    (cl:cons ':node_id (node_id msg))
    (cl:cons ':node_status (node_status msg))
    (cl:cons ':msg (msg msg))
))
;//! \htmlinclude Intervene-response.msg.html

(cl:defclass <Intervene-response> (roslisp-msg-protocol:ros-message)
  ((node_event
    :reader node_event
    :initarg :node_event
    :type cl:integer
    :initform 0))
)

(cl:defclass Intervene-response (<Intervene-response>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <Intervene-response>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'Intervene-response)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name mower_msgs-srv:<Intervene-response> is deprecated: use mower_msgs-srv:Intervene-response instead.")))

(cl:ensure-generic-function 'node_event-val :lambda-list '(m))
(cl:defmethod node_event-val ((m <Intervene-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-srv:node_event-val is deprecated.  Use mower_msgs-srv:node_event instead.")
  (node_event m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <Intervene-response>) ostream)
  "Serializes a message object of type '<Intervene-response>"
  (cl:let* ((signed (cl:slot-value msg 'node_event)) (unsigned (cl:if (cl:< signed 0) (cl:+ signed 4294967296) signed)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) unsigned) ostream)
    )
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <Intervene-response>) istream)
  "Deserializes a message object of type '<Intervene-response>"
    (cl:let ((unsigned 0))
      (cl:setf (cl:ldb (cl:byte 8 0) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) unsigned) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'node_event) (cl:if (cl:< unsigned 2147483648) unsigned (cl:- unsigned 4294967296))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<Intervene-response>)))
  "Returns string type for a service object of type '<Intervene-response>"
  "mower_msgs/InterveneResponse")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'Intervene-response)))
  "Returns string type for a service object of type 'Intervene-response"
  "mower_msgs/InterveneResponse")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<Intervene-response>)))
  "Returns md5sum for a message object of type '<Intervene-response>"
  "15f1f9695525704904993d3c41cc8ed1")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'Intervene-response)))
  "Returns md5sum for a message object of type 'Intervene-response"
  "15f1f9695525704904993d3c41cc8ed1")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<Intervene-response>)))
  "Returns full string definition for message of type '<Intervene-response>"
  (cl:format cl:nil "int32  node_event~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'Intervene-response)))
  "Returns full string definition for message of type 'Intervene-response"
  (cl:format cl:nil "int32  node_event~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <Intervene-response>))
  (cl:+ 0
     4
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <Intervene-response>))
  "Converts a ROS message object to a list"
  (cl:list 'Intervene-response
    (cl:cons ':node_event (node_event msg))
))
(cl:defmethod roslisp-msg-protocol:service-request-type ((msg (cl:eql 'Intervene)))
  'Intervene-request)
(cl:defmethod roslisp-msg-protocol:service-response-type ((msg (cl:eql 'Intervene)))
  'Intervene-response)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'Intervene)))
  "Returns string type for a service object of type '<Intervene>"
  "mower_msgs/Intervene")