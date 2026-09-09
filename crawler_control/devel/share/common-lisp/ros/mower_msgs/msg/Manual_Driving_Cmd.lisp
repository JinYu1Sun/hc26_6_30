; Auto-generated. Do not edit!


(cl:in-package mower_msgs-msg)


;//! \htmlinclude Manual_Driving_Cmd.msg.html

(cl:defclass <Manual_Driving_Cmd> (roslisp-msg-protocol:ros-message)
  ((ad_control_enable
    :reader ad_control_enable
    :initarg :ad_control_enable
    :type cl:fixnum
    :initform 0)
   (turn_value
    :reader turn_value
    :initarg :turn_value
    :type cl:fixnum
    :initform 0)
   (drive_value
    :reader drive_value
    :initarg :drive_value
    :type cl:fixnum
    :initform 0)
   (gear_model
    :reader gear_model
    :initarg :gear_model
    :type cl:fixnum
    :initform 0)
   (mover_bool
    :reader mover_bool
    :initarg :mover_bool
    :type cl:fixnum
    :initform 0)
   (mow_height
    :reader mow_height
    :initarg :mow_height
    :type cl:fixnum
    :initform 0))
)

(cl:defclass Manual_Driving_Cmd (<Manual_Driving_Cmd>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <Manual_Driving_Cmd>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'Manual_Driving_Cmd)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name mower_msgs-msg:<Manual_Driving_Cmd> is deprecated: use mower_msgs-msg:Manual_Driving_Cmd instead.")))

(cl:ensure-generic-function 'ad_control_enable-val :lambda-list '(m))
(cl:defmethod ad_control_enable-val ((m <Manual_Driving_Cmd>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:ad_control_enable-val is deprecated.  Use mower_msgs-msg:ad_control_enable instead.")
  (ad_control_enable m))

(cl:ensure-generic-function 'turn_value-val :lambda-list '(m))
(cl:defmethod turn_value-val ((m <Manual_Driving_Cmd>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:turn_value-val is deprecated.  Use mower_msgs-msg:turn_value instead.")
  (turn_value m))

(cl:ensure-generic-function 'drive_value-val :lambda-list '(m))
(cl:defmethod drive_value-val ((m <Manual_Driving_Cmd>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:drive_value-val is deprecated.  Use mower_msgs-msg:drive_value instead.")
  (drive_value m))

(cl:ensure-generic-function 'gear_model-val :lambda-list '(m))
(cl:defmethod gear_model-val ((m <Manual_Driving_Cmd>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:gear_model-val is deprecated.  Use mower_msgs-msg:gear_model instead.")
  (gear_model m))

(cl:ensure-generic-function 'mover_bool-val :lambda-list '(m))
(cl:defmethod mover_bool-val ((m <Manual_Driving_Cmd>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:mover_bool-val is deprecated.  Use mower_msgs-msg:mover_bool instead.")
  (mover_bool m))

(cl:ensure-generic-function 'mow_height-val :lambda-list '(m))
(cl:defmethod mow_height-val ((m <Manual_Driving_Cmd>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader mower_msgs-msg:mow_height-val is deprecated.  Use mower_msgs-msg:mow_height instead.")
  (mow_height m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <Manual_Driving_Cmd>) ostream)
  "Serializes a message object of type '<Manual_Driving_Cmd>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'ad_control_enable)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'turn_value)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'turn_value)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'drive_value)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'drive_value)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'gear_model)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'mover_bool)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'mow_height)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <Manual_Driving_Cmd>) istream)
  "Deserializes a message object of type '<Manual_Driving_Cmd>"
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'ad_control_enable)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'turn_value)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'turn_value)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'drive_value)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'drive_value)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'gear_model)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'mover_bool)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'mow_height)) (cl:read-byte istream))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<Manual_Driving_Cmd>)))
  "Returns string type for a message object of type '<Manual_Driving_Cmd>"
  "mower_msgs/Manual_Driving_Cmd")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'Manual_Driving_Cmd)))
  "Returns string type for a message object of type 'Manual_Driving_Cmd"
  "mower_msgs/Manual_Driving_Cmd")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<Manual_Driving_Cmd>)))
  "Returns md5sum for a message object of type '<Manual_Driving_Cmd>"
  "2904fb773f7150886bc5690b6fc3e977")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'Manual_Driving_Cmd)))
  "Returns md5sum for a message object of type 'Manual_Driving_Cmd"
  "2904fb773f7150886bc5690b6fc3e977")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<Manual_Driving_Cmd>)))
  "Returns full string definition for message of type '<Manual_Driving_Cmd>"
  (cl:format cl:nil "#Manual_Driving_Cmd~%uint8 ad_control_enable~%uint16 turn_value~%uint16 drive_value~%uint8 gear_model~%uint8 mover_bool~%uint8 mow_height~%~%~%~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'Manual_Driving_Cmd)))
  "Returns full string definition for message of type 'Manual_Driving_Cmd"
  (cl:format cl:nil "#Manual_Driving_Cmd~%uint8 ad_control_enable~%uint16 turn_value~%uint16 drive_value~%uint8 gear_model~%uint8 mover_bool~%uint8 mow_height~%~%~%~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <Manual_Driving_Cmd>))
  (cl:+ 0
     1
     2
     2
     1
     1
     1
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <Manual_Driving_Cmd>))
  "Converts a ROS message object to a list"
  (cl:list 'Manual_Driving_Cmd
    (cl:cons ':ad_control_enable (ad_control_enable msg))
    (cl:cons ':turn_value (turn_value msg))
    (cl:cons ':drive_value (drive_value msg))
    (cl:cons ':gear_model (gear_model msg))
    (cl:cons ':mover_bool (mover_bool msg))
    (cl:cons ':mow_height (mow_height msg))
))
