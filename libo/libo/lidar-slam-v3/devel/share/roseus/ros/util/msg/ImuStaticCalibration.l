;; Auto-generated. Do not edit!


(when (boundp 'util::ImuStaticCalibration)
  (if (not (find-package "UTIL"))
    (make-package "UTIL"))
  (shadow 'ImuStaticCalibration (find-package "UTIL")))
(unless (find-package "UTIL::IMUSTATICCALIBRATION")
  (make-package "UTIL::IMUSTATICCALIBRATION"))

(in-package "ROS")
;;//! \htmlinclude ImuStaticCalibration.msg.html
(if (not (find-package "STD_MSGS"))
  (ros::roseus-add-msgs "std_msgs"))


(defclass util::ImuStaticCalibration
  :super ros::object
  :slots (_header _valid _lio_generation _imu_topic _duration_sec _sample_count _mean_acceleration_mps2 _mean_angular_velocity_radps _acceleration_variance _angular_velocity_variance _max_angular_velocity_norm_radps ))

(defmethod util::ImuStaticCalibration
  (:init
   (&key
    ((:header __header) (instance std_msgs::Header :init))
    ((:valid __valid) nil)
    ((:lio_generation __lio_generation) (instance ros::time :init))
    ((:imu_topic __imu_topic) "")
    ((:duration_sec __duration_sec) 0.0)
    ((:sample_count __sample_count) 0)
    ((:mean_acceleration_mps2 __mean_acceleration_mps2) (make-array 3 :initial-element 0.0 :element-type :float))
    ((:mean_angular_velocity_radps __mean_angular_velocity_radps) (make-array 3 :initial-element 0.0 :element-type :float))
    ((:acceleration_variance __acceleration_variance) (make-array 3 :initial-element 0.0 :element-type :float))
    ((:angular_velocity_variance __angular_velocity_variance) (make-array 3 :initial-element 0.0 :element-type :float))
    ((:max_angular_velocity_norm_radps __max_angular_velocity_norm_radps) 0.0)
    )
   (send-super :init)
   (setq _header __header)
   (setq _valid __valid)
   (setq _lio_generation __lio_generation)
   (setq _imu_topic (string __imu_topic))
   (setq _duration_sec (float __duration_sec))
   (setq _sample_count (round __sample_count))
   (setq _mean_acceleration_mps2 __mean_acceleration_mps2)
   (setq _mean_angular_velocity_radps __mean_angular_velocity_radps)
   (setq _acceleration_variance __acceleration_variance)
   (setq _angular_velocity_variance __angular_velocity_variance)
   (setq _max_angular_velocity_norm_radps (float __max_angular_velocity_norm_radps))
   self)
  (:header
   (&rest __header)
   (if (keywordp (car __header))
       (send* _header __header)
     (progn
       (if __header (setq _header (car __header)))
       _header)))
  (:valid
   (&optional (__valid :null))
   (if (not (eq __valid :null)) (setq _valid __valid)) _valid)
  (:lio_generation
   (&optional __lio_generation)
   (if __lio_generation (setq _lio_generation __lio_generation)) _lio_generation)
  (:imu_topic
   (&optional __imu_topic)
   (if __imu_topic (setq _imu_topic __imu_topic)) _imu_topic)
  (:duration_sec
   (&optional __duration_sec)
   (if __duration_sec (setq _duration_sec __duration_sec)) _duration_sec)
  (:sample_count
   (&optional __sample_count)
   (if __sample_count (setq _sample_count __sample_count)) _sample_count)
  (:mean_acceleration_mps2
   (&optional __mean_acceleration_mps2)
   (if __mean_acceleration_mps2 (setq _mean_acceleration_mps2 __mean_acceleration_mps2)) _mean_acceleration_mps2)
  (:mean_angular_velocity_radps
   (&optional __mean_angular_velocity_radps)
   (if __mean_angular_velocity_radps (setq _mean_angular_velocity_radps __mean_angular_velocity_radps)) _mean_angular_velocity_radps)
  (:acceleration_variance
   (&optional __acceleration_variance)
   (if __acceleration_variance (setq _acceleration_variance __acceleration_variance)) _acceleration_variance)
  (:angular_velocity_variance
   (&optional __angular_velocity_variance)
   (if __angular_velocity_variance (setq _angular_velocity_variance __angular_velocity_variance)) _angular_velocity_variance)
  (:max_angular_velocity_norm_radps
   (&optional __max_angular_velocity_norm_radps)
   (if __max_angular_velocity_norm_radps (setq _max_angular_velocity_norm_radps __max_angular_velocity_norm_radps)) _max_angular_velocity_norm_radps)
  (:serialization-length
   ()
   (+
    ;; std_msgs/Header _header
    (send _header :serialization-length)
    ;; bool _valid
    1
    ;; time _lio_generation
    8
    ;; string _imu_topic
    4 (length _imu_topic)
    ;; float64 _duration_sec
    8
    ;; uint32 _sample_count
    4
    ;; float64[3] _mean_acceleration_mps2
    (* 8    3)
    ;; float64[3] _mean_angular_velocity_radps
    (* 8    3)
    ;; float64[3] _acceleration_variance
    (* 8    3)
    ;; float64[3] _angular_velocity_variance
    (* 8    3)
    ;; float64 _max_angular_velocity_norm_radps
    8
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; std_msgs/Header _header
       (send _header :serialize s)
     ;; bool _valid
       (if _valid (write-byte -1 s) (write-byte 0 s))
     ;; time _lio_generation
       (write-long (send _lio_generation :sec) s) (write-long (send _lio_generation :nsec) s)
     ;; string _imu_topic
       (write-long (length _imu_topic) s) (princ _imu_topic s)
     ;; float64 _duration_sec
       (sys::poke _duration_sec (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; uint32 _sample_count
       (write-long _sample_count s)
     ;; float64[3] _mean_acceleration_mps2
     (dotimes (i 3)
       (sys::poke (elt _mean_acceleration_mps2 i) (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
       )
     ;; float64[3] _mean_angular_velocity_radps
     (dotimes (i 3)
       (sys::poke (elt _mean_angular_velocity_radps i) (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
       )
     ;; float64[3] _acceleration_variance
     (dotimes (i 3)
       (sys::poke (elt _acceleration_variance i) (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
       )
     ;; float64[3] _angular_velocity_variance
     (dotimes (i 3)
       (sys::poke (elt _angular_velocity_variance i) (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
       )
     ;; float64 _max_angular_velocity_norm_radps
       (sys::poke _max_angular_velocity_norm_radps (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; std_msgs/Header _header
     (send _header :deserialize buf ptr-) (incf ptr- (send _header :serialization-length))
   ;; bool _valid
     (setq _valid (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; time _lio_generation
     (send _lio_generation :sec (sys::peek buf ptr- :integer)) (incf ptr- 4)  (send _lio_generation :nsec (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; string _imu_topic
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _imu_topic (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; float64 _duration_sec
     (setq _duration_sec (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; uint32 _sample_count
     (setq _sample_count (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; float64[3] _mean_acceleration_mps2
   (dotimes (i (length _mean_acceleration_mps2))
     (setf (elt _mean_acceleration_mps2 i) (sys::peek buf ptr- :double)) (incf ptr- 8)
     )
   ;; float64[3] _mean_angular_velocity_radps
   (dotimes (i (length _mean_angular_velocity_radps))
     (setf (elt _mean_angular_velocity_radps i) (sys::peek buf ptr- :double)) (incf ptr- 8)
     )
   ;; float64[3] _acceleration_variance
   (dotimes (i (length _acceleration_variance))
     (setf (elt _acceleration_variance i) (sys::peek buf ptr- :double)) (incf ptr- 8)
     )
   ;; float64[3] _angular_velocity_variance
   (dotimes (i (length _angular_velocity_variance))
     (setf (elt _angular_velocity_variance i) (sys::peek buf ptr- :double)) (incf ptr- 8)
     )
   ;; float64 _max_angular_velocity_norm_radps
     (setq _max_angular_velocity_norm_radps (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;;
   self)
  )

(setf (get util::ImuStaticCalibration :md5sum-) "18a255ee627c55f5a95de2c4231b1512")
(setf (get util::ImuStaticCalibration :datatype-) "util/ImuStaticCalibration")
(setf (get util::ImuStaticCalibration :definition-)
      "# FAST-LIO权威静止初始化窗口统计。消息由/laserMapping锁存发布；
# header.stamp是该静止窗口最后一帧IMU的传感器时间，header.frame_id是原始IMU坐标系。
Header header
bool valid
time lio_generation
string imu_topic
float64 duration_sec
uint32 sample_count
float64[3] mean_acceleration_mps2
float64[3] mean_angular_velocity_radps
float64[3] acceleration_variance
float64[3] angular_velocity_variance
float64 max_angular_velocity_norm_radps

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



(provide :util/ImuStaticCalibration "18a255ee627c55f5a95de2c4231b1512")


