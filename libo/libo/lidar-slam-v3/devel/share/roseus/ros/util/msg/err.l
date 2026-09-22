;; Auto-generated. Do not edit!


(when (boundp 'util::err)
  (if (not (find-package "UTIL"))
    (make-package "UTIL"))
  (shadow 'err (find-package "UTIL")))
(unless (find-package "UTIL::ERR")
  (make-package "UTIL::ERR"))

(in-package "ROS")
;;//! \htmlinclude err.msg.html
(if (not (find-package "STD_MSGS"))
  (ros::roseus-add-msgs "std_msgs"))


(defclass util::err
  :super ros::object
  :slots (_header _ant_e _ant_n _ant_u _ant_yaw _dual_yaw _dual_e _dual_n _dual_u _slam_x _slam_y _err_e _err_n _err_u _err_yaw _err_slame _err_slamn _err_slamu _err_slamyaw ))

(defmethod util::err
  (:init
   (&key
    ((:header __header) (instance std_msgs::Header :init))
    ((:ant_e __ant_e) 0.0)
    ((:ant_n __ant_n) 0.0)
    ((:ant_u __ant_u) 0.0)
    ((:ant_yaw __ant_yaw) 0.0)
    ((:dual_yaw __dual_yaw) 0.0)
    ((:dual_e __dual_e) 0.0)
    ((:dual_n __dual_n) 0.0)
    ((:dual_u __dual_u) 0.0)
    ((:slam_x __slam_x) 0.0)
    ((:slam_y __slam_y) 0.0)
    ((:err_e __err_e) 0.0)
    ((:err_n __err_n) 0.0)
    ((:err_u __err_u) 0.0)
    ((:err_yaw __err_yaw) 0.0)
    ((:err_slame __err_slame) 0.0)
    ((:err_slamn __err_slamn) 0.0)
    ((:err_slamu __err_slamu) 0.0)
    ((:err_slamyaw __err_slamyaw) 0.0)
    )
   (send-super :init)
   (setq _header __header)
   (setq _ant_e (float __ant_e))
   (setq _ant_n (float __ant_n))
   (setq _ant_u (float __ant_u))
   (setq _ant_yaw (float __ant_yaw))
   (setq _dual_yaw (float __dual_yaw))
   (setq _dual_e (float __dual_e))
   (setq _dual_n (float __dual_n))
   (setq _dual_u (float __dual_u))
   (setq _slam_x (float __slam_x))
   (setq _slam_y (float __slam_y))
   (setq _err_e (float __err_e))
   (setq _err_n (float __err_n))
   (setq _err_u (float __err_u))
   (setq _err_yaw (float __err_yaw))
   (setq _err_slame (float __err_slame))
   (setq _err_slamn (float __err_slamn))
   (setq _err_slamu (float __err_slamu))
   (setq _err_slamyaw (float __err_slamyaw))
   self)
  (:header
   (&rest __header)
   (if (keywordp (car __header))
       (send* _header __header)
     (progn
       (if __header (setq _header (car __header)))
       _header)))
  (:ant_e
   (&optional __ant_e)
   (if __ant_e (setq _ant_e __ant_e)) _ant_e)
  (:ant_n
   (&optional __ant_n)
   (if __ant_n (setq _ant_n __ant_n)) _ant_n)
  (:ant_u
   (&optional __ant_u)
   (if __ant_u (setq _ant_u __ant_u)) _ant_u)
  (:ant_yaw
   (&optional __ant_yaw)
   (if __ant_yaw (setq _ant_yaw __ant_yaw)) _ant_yaw)
  (:dual_yaw
   (&optional __dual_yaw)
   (if __dual_yaw (setq _dual_yaw __dual_yaw)) _dual_yaw)
  (:dual_e
   (&optional __dual_e)
   (if __dual_e (setq _dual_e __dual_e)) _dual_e)
  (:dual_n
   (&optional __dual_n)
   (if __dual_n (setq _dual_n __dual_n)) _dual_n)
  (:dual_u
   (&optional __dual_u)
   (if __dual_u (setq _dual_u __dual_u)) _dual_u)
  (:slam_x
   (&optional __slam_x)
   (if __slam_x (setq _slam_x __slam_x)) _slam_x)
  (:slam_y
   (&optional __slam_y)
   (if __slam_y (setq _slam_y __slam_y)) _slam_y)
  (:err_e
   (&optional __err_e)
   (if __err_e (setq _err_e __err_e)) _err_e)
  (:err_n
   (&optional __err_n)
   (if __err_n (setq _err_n __err_n)) _err_n)
  (:err_u
   (&optional __err_u)
   (if __err_u (setq _err_u __err_u)) _err_u)
  (:err_yaw
   (&optional __err_yaw)
   (if __err_yaw (setq _err_yaw __err_yaw)) _err_yaw)
  (:err_slame
   (&optional __err_slame)
   (if __err_slame (setq _err_slame __err_slame)) _err_slame)
  (:err_slamn
   (&optional __err_slamn)
   (if __err_slamn (setq _err_slamn __err_slamn)) _err_slamn)
  (:err_slamu
   (&optional __err_slamu)
   (if __err_slamu (setq _err_slamu __err_slamu)) _err_slamu)
  (:err_slamyaw
   (&optional __err_slamyaw)
   (if __err_slamyaw (setq _err_slamyaw __err_slamyaw)) _err_slamyaw)
  (:serialization-length
   ()
   (+
    ;; std_msgs/Header _header
    (send _header :serialization-length)
    ;; float64 _ant_e
    8
    ;; float64 _ant_n
    8
    ;; float64 _ant_u
    8
    ;; float64 _ant_yaw
    8
    ;; float64 _dual_yaw
    8
    ;; float64 _dual_e
    8
    ;; float64 _dual_n
    8
    ;; float64 _dual_u
    8
    ;; float64 _slam_x
    8
    ;; float64 _slam_y
    8
    ;; float64 _err_e
    8
    ;; float64 _err_n
    8
    ;; float64 _err_u
    8
    ;; float64 _err_yaw
    8
    ;; float64 _err_slame
    8
    ;; float64 _err_slamn
    8
    ;; float64 _err_slamu
    8
    ;; float64 _err_slamyaw
    8
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; std_msgs/Header _header
       (send _header :serialize s)
     ;; float64 _ant_e
       (sys::poke _ant_e (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _ant_n
       (sys::poke _ant_n (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _ant_u
       (sys::poke _ant_u (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _ant_yaw
       (sys::poke _ant_yaw (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _dual_yaw
       (sys::poke _dual_yaw (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _dual_e
       (sys::poke _dual_e (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _dual_n
       (sys::poke _dual_n (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _dual_u
       (sys::poke _dual_u (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _slam_x
       (sys::poke _slam_x (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _slam_y
       (sys::poke _slam_y (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _err_e
       (sys::poke _err_e (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _err_n
       (sys::poke _err_n (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _err_u
       (sys::poke _err_u (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _err_yaw
       (sys::poke _err_yaw (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _err_slame
       (sys::poke _err_slame (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _err_slamn
       (sys::poke _err_slamn (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _err_slamu
       (sys::poke _err_slamu (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _err_slamyaw
       (sys::poke _err_slamyaw (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; std_msgs/Header _header
     (send _header :deserialize buf ptr-) (incf ptr- (send _header :serialization-length))
   ;; float64 _ant_e
     (setq _ant_e (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _ant_n
     (setq _ant_n (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _ant_u
     (setq _ant_u (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _ant_yaw
     (setq _ant_yaw (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _dual_yaw
     (setq _dual_yaw (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _dual_e
     (setq _dual_e (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _dual_n
     (setq _dual_n (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _dual_u
     (setq _dual_u (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _slam_x
     (setq _slam_x (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _slam_y
     (setq _slam_y (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _err_e
     (setq _err_e (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _err_n
     (setq _err_n (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _err_u
     (setq _err_u (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _err_yaw
     (setq _err_yaw (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _err_slame
     (setq _err_slame (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _err_slamn
     (setq _err_slamn (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _err_slamu
     (setq _err_slamu (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _err_slamyaw
     (setq _err_slamyaw (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;;
   self)
  )

(setf (get util::err :md5sum-) "1e17d8731f8e0158fd085ddb259f3fb3")
(setf (get util::err :datatype-) "util/err")
(setf (get util::err :definition-)
      "Header header
float64 ant_e
float64 ant_n
float64 ant_u
float64 ant_yaw
float64 dual_yaw
float64 dual_e
float64 dual_n
float64 dual_u
float64 slam_x
float64 slam_y
float64 err_e 
float64 err_n
float64 err_u
float64 err_yaw
float64 err_slame
float64 err_slamn
float64 err_slamu
float64 err_slamyaw 
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



(provide :util/err "1e17d8731f8e0158fd085ddb259f3fb3")


