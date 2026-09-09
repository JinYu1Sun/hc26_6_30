; Auto-generated. Do not edit!


(cl:in-package pure_pursuit-srv)


;//! \htmlinclude TurnCompleted-request.msg.html

(cl:defclass <TurnCompleted-request> (roslisp-msg-protocol:ros-message)
  ((x
    :reader x
    :initarg :x
    :type cl:float
    :initform 0.0)
   (y
    :reader y
    :initarg :y
    :type cl:float
    :initform 0.0))
)

(cl:defclass TurnCompleted-request (<TurnCompleted-request>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <TurnCompleted-request>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'TurnCompleted-request)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name pure_pursuit-srv:<TurnCompleted-request> is deprecated: use pure_pursuit-srv:TurnCompleted-request instead.")))

(cl:ensure-generic-function 'x-val :lambda-list '(m))
(cl:defmethod x-val ((m <TurnCompleted-request>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader pure_pursuit-srv:x-val is deprecated.  Use pure_pursuit-srv:x instead.")
  (x m))

(cl:ensure-generic-function 'y-val :lambda-list '(m))
(cl:defmethod y-val ((m <TurnCompleted-request>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader pure_pursuit-srv:y-val is deprecated.  Use pure_pursuit-srv:y instead.")
  (y m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <TurnCompleted-request>) ostream)
  "Serializes a message object of type '<TurnCompleted-request>"
  (cl:let ((bits (roslisp-utils:encode-double-float-bits (cl:slot-value msg 'x))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 32) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 40) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 48) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 56) bits) ostream))
  (cl:let ((bits (roslisp-utils:encode-double-float-bits (cl:slot-value msg 'y))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 32) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 40) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 48) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 56) bits) ostream))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <TurnCompleted-request>) istream)
  "Deserializes a message object of type '<TurnCompleted-request>"
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 32) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 40) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 48) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 56) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'x) (roslisp-utils:decode-double-float-bits bits)))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 32) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 40) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 48) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 56) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'y) (roslisp-utils:decode-double-float-bits bits)))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<TurnCompleted-request>)))
  "Returns string type for a service object of type '<TurnCompleted-request>"
  "pure_pursuit/TurnCompletedRequest")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'TurnCompleted-request)))
  "Returns string type for a service object of type 'TurnCompleted-request"
  "pure_pursuit/TurnCompletedRequest")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<TurnCompleted-request>)))
  "Returns md5sum for a message object of type '<TurnCompleted-request>"
  "5677d53f5f233f7e8f08a8788d1eb1c6")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'TurnCompleted-request)))
  "Returns md5sum for a message object of type 'TurnCompleted-request"
  "5677d53f5f233f7e8f08a8788d1eb1c6")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<TurnCompleted-request>)))
  "Returns full string definition for message of type '<TurnCompleted-request>"
  (cl:format cl:nil "float64 x~%float64 y~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'TurnCompleted-request)))
  "Returns full string definition for message of type 'TurnCompleted-request"
  (cl:format cl:nil "float64 x~%float64 y~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <TurnCompleted-request>))
  (cl:+ 0
     8
     8
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <TurnCompleted-request>))
  "Converts a ROS message object to a list"
  (cl:list 'TurnCompleted-request
    (cl:cons ':x (x msg))
    (cl:cons ':y (y msg))
))
;//! \htmlinclude TurnCompleted-response.msg.html

(cl:defclass <TurnCompleted-response> (roslisp-msg-protocol:ros-message)
  ((turn_finish
    :reader turn_finish
    :initarg :turn_finish
    :type cl:boolean
    :initform cl:nil))
)

(cl:defclass TurnCompleted-response (<TurnCompleted-response>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <TurnCompleted-response>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'TurnCompleted-response)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name pure_pursuit-srv:<TurnCompleted-response> is deprecated: use pure_pursuit-srv:TurnCompleted-response instead.")))

(cl:ensure-generic-function 'turn_finish-val :lambda-list '(m))
(cl:defmethod turn_finish-val ((m <TurnCompleted-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader pure_pursuit-srv:turn_finish-val is deprecated.  Use pure_pursuit-srv:turn_finish instead.")
  (turn_finish m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <TurnCompleted-response>) ostream)
  "Serializes a message object of type '<TurnCompleted-response>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'turn_finish) 1 0)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <TurnCompleted-response>) istream)
  "Deserializes a message object of type '<TurnCompleted-response>"
    (cl:setf (cl:slot-value msg 'turn_finish) (cl:not (cl:zerop (cl:read-byte istream))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<TurnCompleted-response>)))
  "Returns string type for a service object of type '<TurnCompleted-response>"
  "pure_pursuit/TurnCompletedResponse")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'TurnCompleted-response)))
  "Returns string type for a service object of type 'TurnCompleted-response"
  "pure_pursuit/TurnCompletedResponse")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<TurnCompleted-response>)))
  "Returns md5sum for a message object of type '<TurnCompleted-response>"
  "5677d53f5f233f7e8f08a8788d1eb1c6")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'TurnCompleted-response)))
  "Returns md5sum for a message object of type 'TurnCompleted-response"
  "5677d53f5f233f7e8f08a8788d1eb1c6")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<TurnCompleted-response>)))
  "Returns full string definition for message of type '<TurnCompleted-response>"
  (cl:format cl:nil "bool turn_finish~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'TurnCompleted-response)))
  "Returns full string definition for message of type 'TurnCompleted-response"
  (cl:format cl:nil "bool turn_finish~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <TurnCompleted-response>))
  (cl:+ 0
     1
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <TurnCompleted-response>))
  "Converts a ROS message object to a list"
  (cl:list 'TurnCompleted-response
    (cl:cons ':turn_finish (turn_finish msg))
))
(cl:defmethod roslisp-msg-protocol:service-request-type ((msg (cl:eql 'TurnCompleted)))
  'TurnCompleted-request)
(cl:defmethod roslisp-msg-protocol:service-response-type ((msg (cl:eql 'TurnCompleted)))
  'TurnCompleted-response)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'TurnCompleted)))
  "Returns string type for a service object of type '<TurnCompleted>"
  "pure_pursuit/TurnCompleted")