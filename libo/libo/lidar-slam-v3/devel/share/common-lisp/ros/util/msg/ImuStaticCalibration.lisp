; Auto-generated. Do not edit!


(cl:in-package util-msg)


;//! \htmlinclude ImuStaticCalibration.msg.html

(cl:defclass <ImuStaticCalibration> (roslisp-msg-protocol:ros-message)
  ((header
    :reader header
    :initarg :header
    :type std_msgs-msg:Header
    :initform (cl:make-instance 'std_msgs-msg:Header))
   (valid
    :reader valid
    :initarg :valid
    :type cl:boolean
    :initform cl:nil)
   (lio_generation
    :reader lio_generation
    :initarg :lio_generation
    :type cl:real
    :initform 0)
   (imu_topic
    :reader imu_topic
    :initarg :imu_topic
    :type cl:string
    :initform "")
   (duration_sec
    :reader duration_sec
    :initarg :duration_sec
    :type cl:float
    :initform 0.0)
   (sample_count
    :reader sample_count
    :initarg :sample_count
    :type cl:integer
    :initform 0)
   (mean_acceleration_mps2
    :reader mean_acceleration_mps2
    :initarg :mean_acceleration_mps2
    :type (cl:vector cl:float)
   :initform (cl:make-array 3 :element-type 'cl:float :initial-element 0.0))
   (mean_angular_velocity_radps
    :reader mean_angular_velocity_radps
    :initarg :mean_angular_velocity_radps
    :type (cl:vector cl:float)
   :initform (cl:make-array 3 :element-type 'cl:float :initial-element 0.0))
   (acceleration_variance
    :reader acceleration_variance
    :initarg :acceleration_variance
    :type (cl:vector cl:float)
   :initform (cl:make-array 3 :element-type 'cl:float :initial-element 0.0))
   (angular_velocity_variance
    :reader angular_velocity_variance
    :initarg :angular_velocity_variance
    :type (cl:vector cl:float)
   :initform (cl:make-array 3 :element-type 'cl:float :initial-element 0.0))
   (max_angular_velocity_norm_radps
    :reader max_angular_velocity_norm_radps
    :initarg :max_angular_velocity_norm_radps
    :type cl:float
    :initform 0.0))
)

(cl:defclass ImuStaticCalibration (<ImuStaticCalibration>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <ImuStaticCalibration>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'ImuStaticCalibration)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name util-msg:<ImuStaticCalibration> is deprecated: use util-msg:ImuStaticCalibration instead.")))

(cl:ensure-generic-function 'header-val :lambda-list '(m))
(cl:defmethod header-val ((m <ImuStaticCalibration>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:header-val is deprecated.  Use util-msg:header instead.")
  (header m))

(cl:ensure-generic-function 'valid-val :lambda-list '(m))
(cl:defmethod valid-val ((m <ImuStaticCalibration>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:valid-val is deprecated.  Use util-msg:valid instead.")
  (valid m))

(cl:ensure-generic-function 'lio_generation-val :lambda-list '(m))
(cl:defmethod lio_generation-val ((m <ImuStaticCalibration>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:lio_generation-val is deprecated.  Use util-msg:lio_generation instead.")
  (lio_generation m))

(cl:ensure-generic-function 'imu_topic-val :lambda-list '(m))
(cl:defmethod imu_topic-val ((m <ImuStaticCalibration>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:imu_topic-val is deprecated.  Use util-msg:imu_topic instead.")
  (imu_topic m))

(cl:ensure-generic-function 'duration_sec-val :lambda-list '(m))
(cl:defmethod duration_sec-val ((m <ImuStaticCalibration>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:duration_sec-val is deprecated.  Use util-msg:duration_sec instead.")
  (duration_sec m))

(cl:ensure-generic-function 'sample_count-val :lambda-list '(m))
(cl:defmethod sample_count-val ((m <ImuStaticCalibration>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:sample_count-val is deprecated.  Use util-msg:sample_count instead.")
  (sample_count m))

(cl:ensure-generic-function 'mean_acceleration_mps2-val :lambda-list '(m))
(cl:defmethod mean_acceleration_mps2-val ((m <ImuStaticCalibration>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:mean_acceleration_mps2-val is deprecated.  Use util-msg:mean_acceleration_mps2 instead.")
  (mean_acceleration_mps2 m))

(cl:ensure-generic-function 'mean_angular_velocity_radps-val :lambda-list '(m))
(cl:defmethod mean_angular_velocity_radps-val ((m <ImuStaticCalibration>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:mean_angular_velocity_radps-val is deprecated.  Use util-msg:mean_angular_velocity_radps instead.")
  (mean_angular_velocity_radps m))

(cl:ensure-generic-function 'acceleration_variance-val :lambda-list '(m))
(cl:defmethod acceleration_variance-val ((m <ImuStaticCalibration>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:acceleration_variance-val is deprecated.  Use util-msg:acceleration_variance instead.")
  (acceleration_variance m))

(cl:ensure-generic-function 'angular_velocity_variance-val :lambda-list '(m))
(cl:defmethod angular_velocity_variance-val ((m <ImuStaticCalibration>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:angular_velocity_variance-val is deprecated.  Use util-msg:angular_velocity_variance instead.")
  (angular_velocity_variance m))

(cl:ensure-generic-function 'max_angular_velocity_norm_radps-val :lambda-list '(m))
(cl:defmethod max_angular_velocity_norm_radps-val ((m <ImuStaticCalibration>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:max_angular_velocity_norm_radps-val is deprecated.  Use util-msg:max_angular_velocity_norm_radps instead.")
  (max_angular_velocity_norm_radps m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <ImuStaticCalibration>) ostream)
  "Serializes a message object of type '<ImuStaticCalibration>"
  (roslisp-msg-protocol:serialize (cl:slot-value msg 'header) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'valid) 1 0)) ostream)
  (cl:let ((__sec (cl:floor (cl:slot-value msg 'lio_generation)))
        (__nsec (cl:round (cl:* 1e9 (cl:- (cl:slot-value msg 'lio_generation) (cl:floor (cl:slot-value msg 'lio_generation)))))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __sec) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __sec) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __sec) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __sec) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 0) __nsec) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __nsec) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __nsec) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __nsec) ostream))
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'imu_topic))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'imu_topic))
  (cl:let ((bits (roslisp-utils:encode-double-float-bits (cl:slot-value msg 'duration_sec))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 32) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 40) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 48) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 56) bits) ostream))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'sample_count)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'sample_count)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'sample_count)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'sample_count)) ostream)
  (cl:map cl:nil #'(cl:lambda (ele) (cl:let ((bits (roslisp-utils:encode-double-float-bits ele)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 32) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 40) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 48) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 56) bits) ostream)))
   (cl:slot-value msg 'mean_acceleration_mps2))
  (cl:map cl:nil #'(cl:lambda (ele) (cl:let ((bits (roslisp-utils:encode-double-float-bits ele)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 32) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 40) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 48) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 56) bits) ostream)))
   (cl:slot-value msg 'mean_angular_velocity_radps))
  (cl:map cl:nil #'(cl:lambda (ele) (cl:let ((bits (roslisp-utils:encode-double-float-bits ele)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 32) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 40) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 48) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 56) bits) ostream)))
   (cl:slot-value msg 'acceleration_variance))
  (cl:map cl:nil #'(cl:lambda (ele) (cl:let ((bits (roslisp-utils:encode-double-float-bits ele)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 32) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 40) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 48) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 56) bits) ostream)))
   (cl:slot-value msg 'angular_velocity_variance))
  (cl:let ((bits (roslisp-utils:encode-double-float-bits (cl:slot-value msg 'max_angular_velocity_norm_radps))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 32) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 40) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 48) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 56) bits) ostream))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <ImuStaticCalibration>) istream)
  "Deserializes a message object of type '<ImuStaticCalibration>"
  (roslisp-msg-protocol:deserialize (cl:slot-value msg 'header) istream)
    (cl:setf (cl:slot-value msg 'valid) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:let ((__sec 0) (__nsec 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __sec) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __sec) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __sec) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __sec) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 0) __nsec) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __nsec) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __nsec) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __nsec) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'lio_generation) (cl:+ (cl:coerce __sec 'cl:double-float) (cl:/ __nsec 1e9))))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'imu_topic) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'imu_topic) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 32) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 40) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 48) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 56) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'duration_sec) (roslisp-utils:decode-double-float-bits bits)))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'sample_count)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'sample_count)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'sample_count)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'sample_count)) (cl:read-byte istream))
  (cl:setf (cl:slot-value msg 'mean_acceleration_mps2) (cl:make-array 3))
  (cl:let ((vals (cl:slot-value msg 'mean_acceleration_mps2)))
    (cl:dotimes (i 3)
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 32) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 40) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 48) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 56) bits) (cl:read-byte istream))
    (cl:setf (cl:aref vals i) (roslisp-utils:decode-double-float-bits bits)))))
  (cl:setf (cl:slot-value msg 'mean_angular_velocity_radps) (cl:make-array 3))
  (cl:let ((vals (cl:slot-value msg 'mean_angular_velocity_radps)))
    (cl:dotimes (i 3)
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 32) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 40) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 48) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 56) bits) (cl:read-byte istream))
    (cl:setf (cl:aref vals i) (roslisp-utils:decode-double-float-bits bits)))))
  (cl:setf (cl:slot-value msg 'acceleration_variance) (cl:make-array 3))
  (cl:let ((vals (cl:slot-value msg 'acceleration_variance)))
    (cl:dotimes (i 3)
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 32) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 40) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 48) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 56) bits) (cl:read-byte istream))
    (cl:setf (cl:aref vals i) (roslisp-utils:decode-double-float-bits bits)))))
  (cl:setf (cl:slot-value msg 'angular_velocity_variance) (cl:make-array 3))
  (cl:let ((vals (cl:slot-value msg 'angular_velocity_variance)))
    (cl:dotimes (i 3)
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 32) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 40) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 48) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 56) bits) (cl:read-byte istream))
    (cl:setf (cl:aref vals i) (roslisp-utils:decode-double-float-bits bits)))))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 32) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 40) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 48) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 56) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'max_angular_velocity_norm_radps) (roslisp-utils:decode-double-float-bits bits)))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<ImuStaticCalibration>)))
  "Returns string type for a message object of type '<ImuStaticCalibration>"
  "util/ImuStaticCalibration")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'ImuStaticCalibration)))
  "Returns string type for a message object of type 'ImuStaticCalibration"
  "util/ImuStaticCalibration")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<ImuStaticCalibration>)))
  "Returns md5sum for a message object of type '<ImuStaticCalibration>"
  "18a255ee627c55f5a95de2c4231b1512")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'ImuStaticCalibration)))
  "Returns md5sum for a message object of type 'ImuStaticCalibration"
  "18a255ee627c55f5a95de2c4231b1512")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<ImuStaticCalibration>)))
  "Returns full string definition for message of type '<ImuStaticCalibration>"
  (cl:format cl:nil "# FAST-LIO权威静止初始化窗口统计。消息由/laserMapping锁存发布；~%# header.stamp是该静止窗口最后一帧IMU的传感器时间，header.frame_id是原始IMU坐标系。~%Header header~%bool valid~%time lio_generation~%string imu_topic~%float64 duration_sec~%uint32 sample_count~%float64[3] mean_acceleration_mps2~%float64[3] mean_angular_velocity_radps~%float64[3] acceleration_variance~%float64[3] angular_velocity_variance~%float64 max_angular_velocity_norm_radps~%~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'ImuStaticCalibration)))
  "Returns full string definition for message of type 'ImuStaticCalibration"
  (cl:format cl:nil "# FAST-LIO权威静止初始化窗口统计。消息由/laserMapping锁存发布；~%# header.stamp是该静止窗口最后一帧IMU的传感器时间，header.frame_id是原始IMU坐标系。~%Header header~%bool valid~%time lio_generation~%string imu_topic~%float64 duration_sec~%uint32 sample_count~%float64[3] mean_acceleration_mps2~%float64[3] mean_angular_velocity_radps~%float64[3] acceleration_variance~%float64[3] angular_velocity_variance~%float64 max_angular_velocity_norm_radps~%~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <ImuStaticCalibration>))
  (cl:+ 0
     (roslisp-msg-protocol:serialization-length (cl:slot-value msg 'header))
     1
     8
     4 (cl:length (cl:slot-value msg 'imu_topic))
     8
     4
     0 (cl:reduce #'cl:+ (cl:slot-value msg 'mean_acceleration_mps2) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 8)))
     0 (cl:reduce #'cl:+ (cl:slot-value msg 'mean_angular_velocity_radps) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 8)))
     0 (cl:reduce #'cl:+ (cl:slot-value msg 'acceleration_variance) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 8)))
     0 (cl:reduce #'cl:+ (cl:slot-value msg 'angular_velocity_variance) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 8)))
     8
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <ImuStaticCalibration>))
  "Converts a ROS message object to a list"
  (cl:list 'ImuStaticCalibration
    (cl:cons ':header (header msg))
    (cl:cons ':valid (valid msg))
    (cl:cons ':lio_generation (lio_generation msg))
    (cl:cons ':imu_topic (imu_topic msg))
    (cl:cons ':duration_sec (duration_sec msg))
    (cl:cons ':sample_count (sample_count msg))
    (cl:cons ':mean_acceleration_mps2 (mean_acceleration_mps2 msg))
    (cl:cons ':mean_angular_velocity_radps (mean_angular_velocity_radps msg))
    (cl:cons ':acceleration_variance (acceleration_variance msg))
    (cl:cons ':angular_velocity_variance (angular_velocity_variance msg))
    (cl:cons ':max_angular_velocity_norm_radps (max_angular_velocity_norm_radps msg))
))
