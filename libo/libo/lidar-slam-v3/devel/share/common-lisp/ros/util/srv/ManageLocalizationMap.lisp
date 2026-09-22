; Auto-generated. Do not edit!


(cl:in-package util-srv)


;//! \htmlinclude ManageLocalizationMap-request.msg.html

(cl:defclass <ManageLocalizationMap-request> (roslisp-msg-protocol:ros-message)
  ((command
    :reader command
    :initarg :command
    :type cl:fixnum
    :initform 0)
   (map_name
    :reader map_name
    :initarg :map_name
    :type cl:string
    :initform "")
   (overwrite
    :reader overwrite
    :initarg :overwrite
    :type cl:boolean
    :initform cl:nil))
)

(cl:defclass ManageLocalizationMap-request (<ManageLocalizationMap-request>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <ManageLocalizationMap-request>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'ManageLocalizationMap-request)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name util-srv:<ManageLocalizationMap-request> is deprecated: use util-srv:ManageLocalizationMap-request instead.")))

(cl:ensure-generic-function 'command-val :lambda-list '(m))
(cl:defmethod command-val ((m <ManageLocalizationMap-request>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-srv:command-val is deprecated.  Use util-srv:command instead.")
  (command m))

(cl:ensure-generic-function 'map_name-val :lambda-list '(m))
(cl:defmethod map_name-val ((m <ManageLocalizationMap-request>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-srv:map_name-val is deprecated.  Use util-srv:map_name instead.")
  (map_name m))

(cl:ensure-generic-function 'overwrite-val :lambda-list '(m))
(cl:defmethod overwrite-val ((m <ManageLocalizationMap-request>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-srv:overwrite-val is deprecated.  Use util-srv:overwrite instead.")
  (overwrite m))
(cl:defmethod roslisp-msg-protocol:symbol-codes ((msg-type (cl:eql '<ManageLocalizationMap-request>)))
    "Constants for message type '<ManageLocalizationMap-request>"
  '((:SAVE_CURRENT_AND_ACTIVATE . 1)
    (:ACTIVATE_EXISTING . 2))
)
(cl:defmethod roslisp-msg-protocol:symbol-codes ((msg-type (cl:eql 'ManageLocalizationMap-request)))
    "Constants for message type 'ManageLocalizationMap-request"
  '((:SAVE_CURRENT_AND_ACTIVATE . 1)
    (:ACTIVATE_EXISTING . 2))
)
(cl:defmethod roslisp-msg-protocol:serialize ((msg <ManageLocalizationMap-request>) ostream)
  "Serializes a message object of type '<ManageLocalizationMap-request>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'command)) ostream)
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'map_name))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'map_name))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'overwrite) 1 0)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <ManageLocalizationMap-request>) istream)
  "Deserializes a message object of type '<ManageLocalizationMap-request>"
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'command)) (cl:read-byte istream))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'map_name) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'map_name) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:setf (cl:slot-value msg 'overwrite) (cl:not (cl:zerop (cl:read-byte istream))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<ManageLocalizationMap-request>)))
  "Returns string type for a service object of type '<ManageLocalizationMap-request>"
  "util/ManageLocalizationMapRequest")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'ManageLocalizationMap-request)))
  "Returns string type for a service object of type 'ManageLocalizationMap-request"
  "util/ManageLocalizationMapRequest")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<ManageLocalizationMap-request>)))
  "Returns md5sum for a message object of type '<ManageLocalizationMap-request>"
  "f6eb786c2817a22f906c5e66a8ae611a")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'ManageLocalizationMap-request)))
  "Returns md5sum for a message object of type 'ManageLocalizationMap-request"
  "f6eb786c2817a22f906c5e66a8ae611a")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<ManageLocalizationMap-request>)))
  "Returns full string definition for message of type '<ManageLocalizationMap-request>"
  (cl:format cl:nil "# Transactional map lifecycle API owned by fusion_4dof.~%uint8 SAVE_CURRENT_AND_ACTIVATE=1~%uint8 ACTIVATE_EXISTING=2~%~%uint8 command~%string map_name~%bool overwrite~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'ManageLocalizationMap-request)))
  "Returns full string definition for message of type 'ManageLocalizationMap-request"
  (cl:format cl:nil "# Transactional map lifecycle API owned by fusion_4dof.~%uint8 SAVE_CURRENT_AND_ACTIVATE=1~%uint8 ACTIVATE_EXISTING=2~%~%uint8 command~%string map_name~%bool overwrite~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <ManageLocalizationMap-request>))
  (cl:+ 0
     1
     4 (cl:length (cl:slot-value msg 'map_name))
     1
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <ManageLocalizationMap-request>))
  "Converts a ROS message object to a list"
  (cl:list 'ManageLocalizationMap-request
    (cl:cons ':command (command msg))
    (cl:cons ':map_name (map_name msg))
    (cl:cons ':overwrite (overwrite msg))
))
;//! \htmlinclude ManageLocalizationMap-response.msg.html

(cl:defclass <ManageLocalizationMap-response> (roslisp-msg-protocol:ros-message)
  ((success
    :reader success
    :initarg :success
    :type cl:boolean
    :initform cl:nil)
   (alignment_preserved
    :reader alignment_preserved
    :initarg :alignment_preserved
    :type cl:boolean
    :initform cl:nil)
   (switch_pending
    :reader switch_pending
    :initarg :switch_pending
    :type cl:boolean
    :initform cl:nil)
   (active_map
    :reader active_map
    :initarg :active_map
    :type cl:string
    :initform "")
   (active_map_uuid
    :reader active_map_uuid
    :initarg :active_map_uuid
    :type cl:string
    :initform "")
   (pending_map
    :reader pending_map
    :initarg :pending_map
    :type cl:string
    :initform "")
   (pending_map_uuid
    :reader pending_map_uuid
    :initarg :pending_map_uuid
    :type cl:string
    :initform "")
   (map_schema_version
    :reader map_schema_version
    :initarg :map_schema_version
    :type cl:integer
    :initform 0)
   (coordinate_frame
    :reader coordinate_frame
    :initarg :coordinate_frame
    :type cl:string
    :initform "")
   (origin_checksum
    :reader origin_checksum
    :initarg :origin_checksum
    :type cl:string
    :initform "")
   (reason
    :reader reason
    :initarg :reason
    :type cl:string
    :initform ""))
)

(cl:defclass ManageLocalizationMap-response (<ManageLocalizationMap-response>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <ManageLocalizationMap-response>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'ManageLocalizationMap-response)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name util-srv:<ManageLocalizationMap-response> is deprecated: use util-srv:ManageLocalizationMap-response instead.")))

(cl:ensure-generic-function 'success-val :lambda-list '(m))
(cl:defmethod success-val ((m <ManageLocalizationMap-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-srv:success-val is deprecated.  Use util-srv:success instead.")
  (success m))

(cl:ensure-generic-function 'alignment_preserved-val :lambda-list '(m))
(cl:defmethod alignment_preserved-val ((m <ManageLocalizationMap-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-srv:alignment_preserved-val is deprecated.  Use util-srv:alignment_preserved instead.")
  (alignment_preserved m))

(cl:ensure-generic-function 'switch_pending-val :lambda-list '(m))
(cl:defmethod switch_pending-val ((m <ManageLocalizationMap-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-srv:switch_pending-val is deprecated.  Use util-srv:switch_pending instead.")
  (switch_pending m))

(cl:ensure-generic-function 'active_map-val :lambda-list '(m))
(cl:defmethod active_map-val ((m <ManageLocalizationMap-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-srv:active_map-val is deprecated.  Use util-srv:active_map instead.")
  (active_map m))

(cl:ensure-generic-function 'active_map_uuid-val :lambda-list '(m))
(cl:defmethod active_map_uuid-val ((m <ManageLocalizationMap-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-srv:active_map_uuid-val is deprecated.  Use util-srv:active_map_uuid instead.")
  (active_map_uuid m))

(cl:ensure-generic-function 'pending_map-val :lambda-list '(m))
(cl:defmethod pending_map-val ((m <ManageLocalizationMap-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-srv:pending_map-val is deprecated.  Use util-srv:pending_map instead.")
  (pending_map m))

(cl:ensure-generic-function 'pending_map_uuid-val :lambda-list '(m))
(cl:defmethod pending_map_uuid-val ((m <ManageLocalizationMap-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-srv:pending_map_uuid-val is deprecated.  Use util-srv:pending_map_uuid instead.")
  (pending_map_uuid m))

(cl:ensure-generic-function 'map_schema_version-val :lambda-list '(m))
(cl:defmethod map_schema_version-val ((m <ManageLocalizationMap-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-srv:map_schema_version-val is deprecated.  Use util-srv:map_schema_version instead.")
  (map_schema_version m))

(cl:ensure-generic-function 'coordinate_frame-val :lambda-list '(m))
(cl:defmethod coordinate_frame-val ((m <ManageLocalizationMap-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-srv:coordinate_frame-val is deprecated.  Use util-srv:coordinate_frame instead.")
  (coordinate_frame m))

(cl:ensure-generic-function 'origin_checksum-val :lambda-list '(m))
(cl:defmethod origin_checksum-val ((m <ManageLocalizationMap-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-srv:origin_checksum-val is deprecated.  Use util-srv:origin_checksum instead.")
  (origin_checksum m))

(cl:ensure-generic-function 'reason-val :lambda-list '(m))
(cl:defmethod reason-val ((m <ManageLocalizationMap-response>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-srv:reason-val is deprecated.  Use util-srv:reason instead.")
  (reason m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <ManageLocalizationMap-response>) ostream)
  "Serializes a message object of type '<ManageLocalizationMap-response>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'success) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'alignment_preserved) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'switch_pending) 1 0)) ostream)
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'active_map))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'active_map))
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'active_map_uuid))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'active_map_uuid))
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'pending_map))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'pending_map))
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'pending_map_uuid))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'pending_map_uuid))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'map_schema_version)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'map_schema_version)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'map_schema_version)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'map_schema_version)) ostream)
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'coordinate_frame))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'coordinate_frame))
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'origin_checksum))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'origin_checksum))
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'reason))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'reason))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <ManageLocalizationMap-response>) istream)
  "Deserializes a message object of type '<ManageLocalizationMap-response>"
    (cl:setf (cl:slot-value msg 'success) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:slot-value msg 'alignment_preserved) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:slot-value msg 'switch_pending) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'active_map) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'active_map) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'active_map_uuid) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'active_map_uuid) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'pending_map) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'pending_map) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'pending_map_uuid) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'pending_map_uuid) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'map_schema_version)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'map_schema_version)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'map_schema_version)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'map_schema_version)) (cl:read-byte istream))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'coordinate_frame) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'coordinate_frame) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'origin_checksum) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'origin_checksum) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'reason) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'reason) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<ManageLocalizationMap-response>)))
  "Returns string type for a service object of type '<ManageLocalizationMap-response>"
  "util/ManageLocalizationMapResponse")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'ManageLocalizationMap-response)))
  "Returns string type for a service object of type 'ManageLocalizationMap-response"
  "util/ManageLocalizationMapResponse")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<ManageLocalizationMap-response>)))
  "Returns md5sum for a message object of type '<ManageLocalizationMap-response>"
  "f6eb786c2817a22f906c5e66a8ae611a")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'ManageLocalizationMap-response)))
  "Returns md5sum for a message object of type 'ManageLocalizationMap-response"
  "f6eb786c2817a22f906c5e66a8ae611a")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<ManageLocalizationMap-response>)))
  "Returns full string definition for message of type '<ManageLocalizationMap-response>"
  (cl:format cl:nil "bool success~%bool alignment_preserved~%bool switch_pending~%string active_map~%string active_map_uuid~%string pending_map~%string pending_map_uuid~%uint32 map_schema_version~%string coordinate_frame~%string origin_checksum~%string reason~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'ManageLocalizationMap-response)))
  "Returns full string definition for message of type 'ManageLocalizationMap-response"
  (cl:format cl:nil "bool success~%bool alignment_preserved~%bool switch_pending~%string active_map~%string active_map_uuid~%string pending_map~%string pending_map_uuid~%uint32 map_schema_version~%string coordinate_frame~%string origin_checksum~%string reason~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <ManageLocalizationMap-response>))
  (cl:+ 0
     1
     1
     1
     4 (cl:length (cl:slot-value msg 'active_map))
     4 (cl:length (cl:slot-value msg 'active_map_uuid))
     4 (cl:length (cl:slot-value msg 'pending_map))
     4 (cl:length (cl:slot-value msg 'pending_map_uuid))
     4
     4 (cl:length (cl:slot-value msg 'coordinate_frame))
     4 (cl:length (cl:slot-value msg 'origin_checksum))
     4 (cl:length (cl:slot-value msg 'reason))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <ManageLocalizationMap-response>))
  "Converts a ROS message object to a list"
  (cl:list 'ManageLocalizationMap-response
    (cl:cons ':success (success msg))
    (cl:cons ':alignment_preserved (alignment_preserved msg))
    (cl:cons ':switch_pending (switch_pending msg))
    (cl:cons ':active_map (active_map msg))
    (cl:cons ':active_map_uuid (active_map_uuid msg))
    (cl:cons ':pending_map (pending_map msg))
    (cl:cons ':pending_map_uuid (pending_map_uuid msg))
    (cl:cons ':map_schema_version (map_schema_version msg))
    (cl:cons ':coordinate_frame (coordinate_frame msg))
    (cl:cons ':origin_checksum (origin_checksum msg))
    (cl:cons ':reason (reason msg))
))
(cl:defmethod roslisp-msg-protocol:service-request-type ((msg (cl:eql 'ManageLocalizationMap)))
  'ManageLocalizationMap-request)
(cl:defmethod roslisp-msg-protocol:service-response-type ((msg (cl:eql 'ManageLocalizationMap)))
  'ManageLocalizationMap-response)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'ManageLocalizationMap)))
  "Returns string type for a service object of type '<ManageLocalizationMap>"
  "util/ManageLocalizationMap")