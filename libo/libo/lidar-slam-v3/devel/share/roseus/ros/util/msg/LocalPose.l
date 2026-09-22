;; Auto-generated. Do not edit!


(when (boundp 'util::LocalPose)
  (if (not (find-package "UTIL"))
    (make-package "UTIL"))
  (shadow 'LocalPose (find-package "UTIL")))
(unless (find-package "UTIL::LOCALPOSE")
  (make-package "UTIL::LOCALPOSE"))

(in-package "ROS")
;;//! \htmlinclude LocalPose.msg.html
(if (not (find-package "STD_MSGS"))
  (ros::roseus-add-msgs "std_msgs"))


(defclass util::LocalPose
  :super ros::object
  :slots (_header _plat_id _error_code _gps_week _gps_millisecond _dr_x _dr_y _dr_z _dr_heading _dr_roll _dr_pitch _vehicle_speed _rot_x _rot_y _rot_z _acc_x _acc_y _acc_z ))

(defmethod util::LocalPose
  (:init
   (&key
    ((:header __header) (instance std_msgs::Header :init))
    ((:plat_id __plat_id) 0)
    ((:error_code __error_code) 0)
    ((:gps_week __gps_week) 0)
    ((:gps_millisecond __gps_millisecond) 0)
    ((:dr_x __dr_x) 0.0)
    ((:dr_y __dr_y) 0.0)
    ((:dr_z __dr_z) 0.0)
    ((:dr_heading __dr_heading) 0.0)
    ((:dr_roll __dr_roll) 0.0)
    ((:dr_pitch __dr_pitch) 0.0)
    ((:vehicle_speed __vehicle_speed) 0)
    ((:rot_x __rot_x) 0.0)
    ((:rot_y __rot_y) 0.0)
    ((:rot_z __rot_z) 0.0)
    ((:acc_x __acc_x) 0.0)
    ((:acc_y __acc_y) 0.0)
    ((:acc_z __acc_z) 0.0)
    )
   (send-super :init)
   (setq _header __header)
   (setq _plat_id (round __plat_id))
   (setq _error_code (round __error_code))
   (setq _gps_week (round __gps_week))
   (setq _gps_millisecond (round __gps_millisecond))
   (setq _dr_x (float __dr_x))
   (setq _dr_y (float __dr_y))
   (setq _dr_z (float __dr_z))
   (setq _dr_heading (float __dr_heading))
   (setq _dr_roll (float __dr_roll))
   (setq _dr_pitch (float __dr_pitch))
   (setq _vehicle_speed (round __vehicle_speed))
   (setq _rot_x (float __rot_x))
   (setq _rot_y (float __rot_y))
   (setq _rot_z (float __rot_z))
   (setq _acc_x (float __acc_x))
   (setq _acc_y (float __acc_y))
   (setq _acc_z (float __acc_z))
   self)
  (:header
   (&rest __header)
   (if (keywordp (car __header))
       (send* _header __header)
     (progn
       (if __header (setq _header (car __header)))
       _header)))
  (:plat_id
   (&optional __plat_id)
   (if __plat_id (setq _plat_id __plat_id)) _plat_id)
  (:error_code
   (&optional __error_code)
   (if __error_code (setq _error_code __error_code)) _error_code)
  (:gps_week
   (&optional __gps_week)
   (if __gps_week (setq _gps_week __gps_week)) _gps_week)
  (:gps_millisecond
   (&optional __gps_millisecond)
   (if __gps_millisecond (setq _gps_millisecond __gps_millisecond)) _gps_millisecond)
  (:dr_x
   (&optional __dr_x)
   (if __dr_x (setq _dr_x __dr_x)) _dr_x)
  (:dr_y
   (&optional __dr_y)
   (if __dr_y (setq _dr_y __dr_y)) _dr_y)
  (:dr_z
   (&optional __dr_z)
   (if __dr_z (setq _dr_z __dr_z)) _dr_z)
  (:dr_heading
   (&optional __dr_heading)
   (if __dr_heading (setq _dr_heading __dr_heading)) _dr_heading)
  (:dr_roll
   (&optional __dr_roll)
   (if __dr_roll (setq _dr_roll __dr_roll)) _dr_roll)
  (:dr_pitch
   (&optional __dr_pitch)
   (if __dr_pitch (setq _dr_pitch __dr_pitch)) _dr_pitch)
  (:vehicle_speed
   (&optional __vehicle_speed)
   (if __vehicle_speed (setq _vehicle_speed __vehicle_speed)) _vehicle_speed)
  (:rot_x
   (&optional __rot_x)
   (if __rot_x (setq _rot_x __rot_x)) _rot_x)
  (:rot_y
   (&optional __rot_y)
   (if __rot_y (setq _rot_y __rot_y)) _rot_y)
  (:rot_z
   (&optional __rot_z)
   (if __rot_z (setq _rot_z __rot_z)) _rot_z)
  (:acc_x
   (&optional __acc_x)
   (if __acc_x (setq _acc_x __acc_x)) _acc_x)
  (:acc_y
   (&optional __acc_y)
   (if __acc_y (setq _acc_y __acc_y)) _acc_y)
  (:acc_z
   (&optional __acc_z)
   (if __acc_z (setq _acc_z __acc_z)) _acc_z)
  (:serialization-length
   ()
   (+
    ;; std_msgs/Header _header
    (send _header :serialization-length)
    ;; int32 _plat_id
    4
    ;; int32 _error_code
    4
    ;; int32 _gps_week
    4
    ;; int32 _gps_millisecond
    4
    ;; float64 _dr_x
    8
    ;; float64 _dr_y
    8
    ;; float64 _dr_z
    8
    ;; float64 _dr_heading
    8
    ;; float64 _dr_roll
    8
    ;; float64 _dr_pitch
    8
    ;; int32 _vehicle_speed
    4
    ;; float64 _rot_x
    8
    ;; float64 _rot_y
    8
    ;; float64 _rot_z
    8
    ;; float64 _acc_x
    8
    ;; float64 _acc_y
    8
    ;; float64 _acc_z
    8
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; std_msgs/Header _header
       (send _header :serialize s)
     ;; int32 _plat_id
       (write-long _plat_id s)
     ;; int32 _error_code
       (write-long _error_code s)
     ;; int32 _gps_week
       (write-long _gps_week s)
     ;; int32 _gps_millisecond
       (write-long _gps_millisecond s)
     ;; float64 _dr_x
       (sys::poke _dr_x (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _dr_y
       (sys::poke _dr_y (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _dr_z
       (sys::poke _dr_z (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _dr_heading
       (sys::poke _dr_heading (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _dr_roll
       (sys::poke _dr_roll (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _dr_pitch
       (sys::poke _dr_pitch (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; int32 _vehicle_speed
       (write-long _vehicle_speed s)
     ;; float64 _rot_x
       (sys::poke _rot_x (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _rot_y
       (sys::poke _rot_y (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _rot_z
       (sys::poke _rot_z (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _acc_x
       (sys::poke _acc_x (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _acc_y
       (sys::poke _acc_y (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _acc_z
       (sys::poke _acc_z (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; std_msgs/Header _header
     (send _header :deserialize buf ptr-) (incf ptr- (send _header :serialization-length))
   ;; int32 _plat_id
     (setq _plat_id (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; int32 _error_code
     (setq _error_code (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; int32 _gps_week
     (setq _gps_week (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; int32 _gps_millisecond
     (setq _gps_millisecond (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; float64 _dr_x
     (setq _dr_x (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _dr_y
     (setq _dr_y (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _dr_z
     (setq _dr_z (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _dr_heading
     (setq _dr_heading (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _dr_roll
     (setq _dr_roll (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _dr_pitch
     (setq _dr_pitch (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; int32 _vehicle_speed
     (setq _vehicle_speed (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; float64 _rot_x
     (setq _rot_x (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _rot_y
     (setq _rot_y (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _rot_z
     (setq _rot_z (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _acc_x
     (setq _acc_x (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _acc_y
     (setq _acc_y (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _acc_z
     (setq _acc_z (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;;
   self)
  )

(setf (get util::LocalPose :md5sum-) "89af9872db18309af0c18302f7e3f52e")
(setf (get util::LocalPose :datatype-) "util/LocalPose")
(setf (get util::LocalPose :definition-)
      "#LocalPose
Header header
int32 	plat_id				#标识此消息对应的平台编号

int32 	error_code   		#错误标识码

int32 	gps_week			#GPS Week
int32 	gps_millisecond     #GPS millionsecond in a week ,单位：ms

float64 dr_x           		#航迹推算车辆位置,单位：cm
float64 dr_y
float64 dr_z

float64 dr_heading     		#车体姿态,单位:0.01degree	, 航向角正东方向为零度,逆时针方向为正,0-360度	
float64 dr_roll				
float64 dr_pitch		


int32 	vehicle_speed   	#车速：cm/s  负值表示倒车

#imu三轴陀螺速度 
float64 rot_x       		#x轴方向右手定则,四指方向为正,单位0.01 deg/s
float64 rot_y				#y轴方向右手定则,四指方向为正,单位0.01 deg/s
float64 rot_z				#z轴方向右手定则,四指方向为正,单位0.01 deg/s

#imu三轴加速度 
float64 acc_x				#x轴方向加速度(车体正右方)	单位0.01m/s^2
float64 acc_y				#y轴方向加速度(车体正前方)	单位0.01m/s^2
float64 acc_z				#z轴方向加速度(车体正上方)	单位0.01m/s^2

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



(provide :util/LocalPose "89af9872db18309af0c18302f7e3f52e")


