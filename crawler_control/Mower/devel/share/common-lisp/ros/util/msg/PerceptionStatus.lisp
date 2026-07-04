; Auto-generated. Do not edit!


(cl:in-package util-msg)


;//! \htmlinclude PerceptionStatus.msg.html

(cl:defclass <PerceptionStatus> (roslisp-msg-protocol:ros-message)
  ((header
    :reader header
    :initarg :header
    :type std_msgs-msg:Header
    :initform (cl:make-instance 'std_msgs-msg:Header))
   (lidar2BaselinkStatus
    :reader lidar2BaselinkStatus
    :initarg :lidar2BaselinkStatus
    :type cl:boolean
    :initform cl:nil)
   (lidar2CameraStatus
    :reader lidar2CameraStatus
    :initarg :lidar2CameraStatus
    :type cl:boolean
    :initform cl:nil)
   (cameraIntrinsicStatus
    :reader cameraIntrinsicStatus
    :initarg :cameraIntrinsicStatus
    :type cl:boolean
    :initform cl:nil)
   (distcoeffStatus
    :reader distcoeffStatus
    :initarg :distcoeffStatus
    :type cl:boolean
    :initform cl:nil)
   (pointcloudStatus
    :reader pointcloudStatus
    :initarg :pointcloudStatus
    :type cl:boolean
    :initform cl:nil)
   (poseStatus
    :reader poseStatus
    :initarg :poseStatus
    :type cl:boolean
    :initform cl:nil)
   (yoloStatus
    :reader yoloStatus
    :initarg :yoloStatus
    :type cl:boolean
    :initform cl:nil))
)

(cl:defclass PerceptionStatus (<PerceptionStatus>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <PerceptionStatus>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'PerceptionStatus)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name util-msg:<PerceptionStatus> is deprecated: use util-msg:PerceptionStatus instead.")))

(cl:ensure-generic-function 'header-val :lambda-list '(m))
(cl:defmethod header-val ((m <PerceptionStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:header-val is deprecated.  Use util-msg:header instead.")
  (header m))

(cl:ensure-generic-function 'lidar2BaselinkStatus-val :lambda-list '(m))
(cl:defmethod lidar2BaselinkStatus-val ((m <PerceptionStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:lidar2BaselinkStatus-val is deprecated.  Use util-msg:lidar2BaselinkStatus instead.")
  (lidar2BaselinkStatus m))

(cl:ensure-generic-function 'lidar2CameraStatus-val :lambda-list '(m))
(cl:defmethod lidar2CameraStatus-val ((m <PerceptionStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:lidar2CameraStatus-val is deprecated.  Use util-msg:lidar2CameraStatus instead.")
  (lidar2CameraStatus m))

(cl:ensure-generic-function 'cameraIntrinsicStatus-val :lambda-list '(m))
(cl:defmethod cameraIntrinsicStatus-val ((m <PerceptionStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:cameraIntrinsicStatus-val is deprecated.  Use util-msg:cameraIntrinsicStatus instead.")
  (cameraIntrinsicStatus m))

(cl:ensure-generic-function 'distcoeffStatus-val :lambda-list '(m))
(cl:defmethod distcoeffStatus-val ((m <PerceptionStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:distcoeffStatus-val is deprecated.  Use util-msg:distcoeffStatus instead.")
  (distcoeffStatus m))

(cl:ensure-generic-function 'pointcloudStatus-val :lambda-list '(m))
(cl:defmethod pointcloudStatus-val ((m <PerceptionStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:pointcloudStatus-val is deprecated.  Use util-msg:pointcloudStatus instead.")
  (pointcloudStatus m))

(cl:ensure-generic-function 'poseStatus-val :lambda-list '(m))
(cl:defmethod poseStatus-val ((m <PerceptionStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:poseStatus-val is deprecated.  Use util-msg:poseStatus instead.")
  (poseStatus m))

(cl:ensure-generic-function 'yoloStatus-val :lambda-list '(m))
(cl:defmethod yoloStatus-val ((m <PerceptionStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:yoloStatus-val is deprecated.  Use util-msg:yoloStatus instead.")
  (yoloStatus m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <PerceptionStatus>) ostream)
  "Serializes a message object of type '<PerceptionStatus>"
  (roslisp-msg-protocol:serialize (cl:slot-value msg 'header) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'lidar2BaselinkStatus) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'lidar2CameraStatus) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'cameraIntrinsicStatus) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'distcoeffStatus) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'pointcloudStatus) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'poseStatus) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'yoloStatus) 1 0)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <PerceptionStatus>) istream)
  "Deserializes a message object of type '<PerceptionStatus>"
  (roslisp-msg-protocol:deserialize (cl:slot-value msg 'header) istream)
    (cl:setf (cl:slot-value msg 'lidar2BaselinkStatus) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:slot-value msg 'lidar2CameraStatus) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:slot-value msg 'cameraIntrinsicStatus) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:slot-value msg 'distcoeffStatus) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:slot-value msg 'pointcloudStatus) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:slot-value msg 'poseStatus) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:slot-value msg 'yoloStatus) (cl:not (cl:zerop (cl:read-byte istream))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<PerceptionStatus>)))
  "Returns string type for a message object of type '<PerceptionStatus>"
  "util/PerceptionStatus")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'PerceptionStatus)))
  "Returns string type for a message object of type 'PerceptionStatus"
  "util/PerceptionStatus")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<PerceptionStatus>)))
  "Returns md5sum for a message object of type '<PerceptionStatus>"
  "ab035d44aff8214d3f26c118b7695eb3")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'PerceptionStatus)))
  "Returns md5sum for a message object of type 'PerceptionStatus"
  "ab035d44aff8214d3f26c118b7695eb3")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<PerceptionStatus>)))
  "Returns full string definition for message of type '<PerceptionStatus>"
  (cl:format cl:nil "Header header~%bool lidar2BaselinkStatus~%bool lidar2CameraStatus~%bool cameraIntrinsicStatus~%bool distcoeffStatus~%bool pointcloudStatus~%bool poseStatus~%bool yoloStatus~%~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'PerceptionStatus)))
  "Returns full string definition for message of type 'PerceptionStatus"
  (cl:format cl:nil "Header header~%bool lidar2BaselinkStatus~%bool lidar2CameraStatus~%bool cameraIntrinsicStatus~%bool distcoeffStatus~%bool pointcloudStatus~%bool poseStatus~%bool yoloStatus~%~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <PerceptionStatus>))
  (cl:+ 0
     (roslisp-msg-protocol:serialization-length (cl:slot-value msg 'header))
     1
     1
     1
     1
     1
     1
     1
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <PerceptionStatus>))
  "Converts a ROS message object to a list"
  (cl:list 'PerceptionStatus
    (cl:cons ':header (header msg))
    (cl:cons ':lidar2BaselinkStatus (lidar2BaselinkStatus msg))
    (cl:cons ':lidar2CameraStatus (lidar2CameraStatus msg))
    (cl:cons ':cameraIntrinsicStatus (cameraIntrinsicStatus msg))
    (cl:cons ':distcoeffStatus (distcoeffStatus msg))
    (cl:cons ':pointcloudStatus (pointcloudStatus msg))
    (cl:cons ':poseStatus (poseStatus msg))
    (cl:cons ':yoloStatus (yoloStatus msg))
))
