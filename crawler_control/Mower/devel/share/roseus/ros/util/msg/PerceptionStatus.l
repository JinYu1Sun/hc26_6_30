;; Auto-generated. Do not edit!


(when (boundp 'util::PerceptionStatus)
  (if (not (find-package "UTIL"))
    (make-package "UTIL"))
  (shadow 'PerceptionStatus (find-package "UTIL")))
(unless (find-package "UTIL::PERCEPTIONSTATUS")
  (make-package "UTIL::PERCEPTIONSTATUS"))

(in-package "ROS")
;;//! \htmlinclude PerceptionStatus.msg.html
(if (not (find-package "STD_MSGS"))
  (ros::roseus-add-msgs "std_msgs"))


(defclass util::PerceptionStatus
  :super ros::object
  :slots (_header _lidar2BaselinkStatus _lidar2CameraStatus _cameraIntrinsicStatus _distcoeffStatus _pointcloudStatus _poseStatus _yoloStatus ))

(defmethod util::PerceptionStatus
  (:init
   (&key
    ((:header __header) (instance std_msgs::Header :init))
    ((:lidar2BaselinkStatus __lidar2BaselinkStatus) nil)
    ((:lidar2CameraStatus __lidar2CameraStatus) nil)
    ((:cameraIntrinsicStatus __cameraIntrinsicStatus) nil)
    ((:distcoeffStatus __distcoeffStatus) nil)
    ((:pointcloudStatus __pointcloudStatus) nil)
    ((:poseStatus __poseStatus) nil)
    ((:yoloStatus __yoloStatus) nil)
    )
   (send-super :init)
   (setq _header __header)
   (setq _lidar2BaselinkStatus __lidar2BaselinkStatus)
   (setq _lidar2CameraStatus __lidar2CameraStatus)
   (setq _cameraIntrinsicStatus __cameraIntrinsicStatus)
   (setq _distcoeffStatus __distcoeffStatus)
   (setq _pointcloudStatus __pointcloudStatus)
   (setq _poseStatus __poseStatus)
   (setq _yoloStatus __yoloStatus)
   self)
  (:header
   (&rest __header)
   (if (keywordp (car __header))
       (send* _header __header)
     (progn
       (if __header (setq _header (car __header)))
       _header)))
  (:lidar2BaselinkStatus
   (&optional (__lidar2BaselinkStatus :null))
   (if (not (eq __lidar2BaselinkStatus :null)) (setq _lidar2BaselinkStatus __lidar2BaselinkStatus)) _lidar2BaselinkStatus)
  (:lidar2CameraStatus
   (&optional (__lidar2CameraStatus :null))
   (if (not (eq __lidar2CameraStatus :null)) (setq _lidar2CameraStatus __lidar2CameraStatus)) _lidar2CameraStatus)
  (:cameraIntrinsicStatus
   (&optional (__cameraIntrinsicStatus :null))
   (if (not (eq __cameraIntrinsicStatus :null)) (setq _cameraIntrinsicStatus __cameraIntrinsicStatus)) _cameraIntrinsicStatus)
  (:distcoeffStatus
   (&optional (__distcoeffStatus :null))
   (if (not (eq __distcoeffStatus :null)) (setq _distcoeffStatus __distcoeffStatus)) _distcoeffStatus)
  (:pointcloudStatus
   (&optional (__pointcloudStatus :null))
   (if (not (eq __pointcloudStatus :null)) (setq _pointcloudStatus __pointcloudStatus)) _pointcloudStatus)
  (:poseStatus
   (&optional (__poseStatus :null))
   (if (not (eq __poseStatus :null)) (setq _poseStatus __poseStatus)) _poseStatus)
  (:yoloStatus
   (&optional (__yoloStatus :null))
   (if (not (eq __yoloStatus :null)) (setq _yoloStatus __yoloStatus)) _yoloStatus)
  (:serialization-length
   ()
   (+
    ;; std_msgs/Header _header
    (send _header :serialization-length)
    ;; bool _lidar2BaselinkStatus
    1
    ;; bool _lidar2CameraStatus
    1
    ;; bool _cameraIntrinsicStatus
    1
    ;; bool _distcoeffStatus
    1
    ;; bool _pointcloudStatus
    1
    ;; bool _poseStatus
    1
    ;; bool _yoloStatus
    1
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; std_msgs/Header _header
       (send _header :serialize s)
     ;; bool _lidar2BaselinkStatus
       (if _lidar2BaselinkStatus (write-byte -1 s) (write-byte 0 s))
     ;; bool _lidar2CameraStatus
       (if _lidar2CameraStatus (write-byte -1 s) (write-byte 0 s))
     ;; bool _cameraIntrinsicStatus
       (if _cameraIntrinsicStatus (write-byte -1 s) (write-byte 0 s))
     ;; bool _distcoeffStatus
       (if _distcoeffStatus (write-byte -1 s) (write-byte 0 s))
     ;; bool _pointcloudStatus
       (if _pointcloudStatus (write-byte -1 s) (write-byte 0 s))
     ;; bool _poseStatus
       (if _poseStatus (write-byte -1 s) (write-byte 0 s))
     ;; bool _yoloStatus
       (if _yoloStatus (write-byte -1 s) (write-byte 0 s))
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; std_msgs/Header _header
     (send _header :deserialize buf ptr-) (incf ptr- (send _header :serialization-length))
   ;; bool _lidar2BaselinkStatus
     (setq _lidar2BaselinkStatus (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; bool _lidar2CameraStatus
     (setq _lidar2CameraStatus (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; bool _cameraIntrinsicStatus
     (setq _cameraIntrinsicStatus (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; bool _distcoeffStatus
     (setq _distcoeffStatus (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; bool _pointcloudStatus
     (setq _pointcloudStatus (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; bool _poseStatus
     (setq _poseStatus (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; bool _yoloStatus
     (setq _yoloStatus (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;;
   self)
  )

(setf (get util::PerceptionStatus :md5sum-) "ab035d44aff8214d3f26c118b7695eb3")
(setf (get util::PerceptionStatus :datatype-) "util/PerceptionStatus")
(setf (get util::PerceptionStatus :definition-)
      "Header header
bool lidar2BaselinkStatus
bool lidar2CameraStatus
bool cameraIntrinsicStatus
bool distcoeffStatus
bool pointcloudStatus
bool poseStatus
bool yoloStatus

================================================================================
MSG: std_msgs/Header
# Standard metadata for higher-level stamped data types.
# This is generally used to communicate timestamped data 
# in a particular coordinate frame.
# 
# sequence ID: consecutively increasing ID 
uint32 seq
#Two-integer timestamp that is expressed as:
# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')
# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')
# time-handling sugar is provided by the client library
time stamp
#Frame this data is associated with
string frame_id

")



(provide :util/PerceptionStatus "ab035d44aff8214d3f26c118b7695eb3")


