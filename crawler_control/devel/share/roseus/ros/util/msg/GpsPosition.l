;; Auto-generated. Do not edit!


(when (boundp 'util::GpsPosition)
  (if (not (find-package "UTIL"))
    (make-package "UTIL"))
  (shadow 'GpsPosition (find-package "UTIL")))
(unless (find-package "UTIL::GPSPOSITION")
  (make-package "UTIL::GPSPOSITION"))

(in-package "ROS")
;;//! \htmlinclude GpsPosition.msg.html
(if (not (find-package "STD_MSGS"))
  (ros::roseus-add-msgs "std_msgs"))


(defclass util::GpsPosition
  :super ros::object
  :slots (_header _plat_id _error_code _gps_flag _positionStatus _gps_week _gps_millisecond _longitude _latitude _height _gaussX _gaussY _pitch _roll _azimuth _acc_x _acc_y _acc_z _rot_x _rot_y _rot_z _northVelocity _eastVelocity _upVelocity _gps_confidence _INS_GpsFlag_Pos _INS_NumSV _INS_GpsFlag_Heading _INS_Gps_Age _INS_Car_Status _INS_Status _INS_VehicleAlign _INS_Std_Lat _INS_Std_Lon _INS_Std_LocatHeight _INS_Std_Heading ))

(defmethod util::GpsPosition
  (:init
   (&key
    ((:header __header) (instance std_msgs::Header :init))
    ((:plat_id __plat_id) 0)
    ((:error_code __error_code) 0)
    ((:gps_flag __gps_flag) 0)
    ((:positionStatus __positionStatus) 0)
    ((:gps_week __gps_week) 0)
    ((:gps_millisecond __gps_millisecond) 0)
    ((:longitude __longitude) 0.0)
    ((:latitude __latitude) 0.0)
    ((:height __height) 0.0)
    ((:gaussX __gaussX) 0.0)
    ((:gaussY __gaussY) 0.0)
    ((:pitch __pitch) 0.0)
    ((:roll __roll) 0.0)
    ((:azimuth __azimuth) 0.0)
    ((:acc_x __acc_x) 0.0)
    ((:acc_y __acc_y) 0.0)
    ((:acc_z __acc_z) 0.0)
    ((:rot_x __rot_x) 0.0)
    ((:rot_y __rot_y) 0.0)
    ((:rot_z __rot_z) 0.0)
    ((:northVelocity __northVelocity) 0.0)
    ((:eastVelocity __eastVelocity) 0.0)
    ((:upVelocity __upVelocity) 0.0)
    ((:gps_confidence __gps_confidence) 0)
    ((:INS_GpsFlag_Pos __INS_GpsFlag_Pos) 0)
    ((:INS_NumSV __INS_NumSV) 0)
    ((:INS_GpsFlag_Heading __INS_GpsFlag_Heading) 0)
    ((:INS_Gps_Age __INS_Gps_Age) 0)
    ((:INS_Car_Status __INS_Car_Status) 0)
    ((:INS_Status __INS_Status) 0)
    ((:INS_VehicleAlign __INS_VehicleAlign) 0)
    ((:INS_Std_Lat __INS_Std_Lat) 0.0)
    ((:INS_Std_Lon __INS_Std_Lon) 0.0)
    ((:INS_Std_LocatHeight __INS_Std_LocatHeight) 0.0)
    ((:INS_Std_Heading __INS_Std_Heading) 0.0)
    )
   (send-super :init)
   (setq _header __header)
   (setq _plat_id (round __plat_id))
   (setq _error_code (round __error_code))
   (setq _gps_flag (round __gps_flag))
   (setq _positionStatus (round __positionStatus))
   (setq _gps_week (round __gps_week))
   (setq _gps_millisecond (round __gps_millisecond))
   (setq _longitude (float __longitude))
   (setq _latitude (float __latitude))
   (setq _height (float __height))
   (setq _gaussX (float __gaussX))
   (setq _gaussY (float __gaussY))
   (setq _pitch (float __pitch))
   (setq _roll (float __roll))
   (setq _azimuth (float __azimuth))
   (setq _acc_x (float __acc_x))
   (setq _acc_y (float __acc_y))
   (setq _acc_z (float __acc_z))
   (setq _rot_x (float __rot_x))
   (setq _rot_y (float __rot_y))
   (setq _rot_z (float __rot_z))
   (setq _northVelocity (float __northVelocity))
   (setq _eastVelocity (float __eastVelocity))
   (setq _upVelocity (float __upVelocity))
   (setq _gps_confidence (round __gps_confidence))
   (setq _INS_GpsFlag_Pos (round __INS_GpsFlag_Pos))
   (setq _INS_NumSV (round __INS_NumSV))
   (setq _INS_GpsFlag_Heading (round __INS_GpsFlag_Heading))
   (setq _INS_Gps_Age (round __INS_Gps_Age))
   (setq _INS_Car_Status (round __INS_Car_Status))
   (setq _INS_Status (round __INS_Status))
   (setq _INS_VehicleAlign (round __INS_VehicleAlign))
   (setq _INS_Std_Lat (float __INS_Std_Lat))
   (setq _INS_Std_Lon (float __INS_Std_Lon))
   (setq _INS_Std_LocatHeight (float __INS_Std_LocatHeight))
   (setq _INS_Std_Heading (float __INS_Std_Heading))
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
  (:gps_flag
   (&optional __gps_flag)
   (if __gps_flag (setq _gps_flag __gps_flag)) _gps_flag)
  (:positionStatus
   (&optional __positionStatus)
   (if __positionStatus (setq _positionStatus __positionStatus)) _positionStatus)
  (:gps_week
   (&optional __gps_week)
   (if __gps_week (setq _gps_week __gps_week)) _gps_week)
  (:gps_millisecond
   (&optional __gps_millisecond)
   (if __gps_millisecond (setq _gps_millisecond __gps_millisecond)) _gps_millisecond)
  (:longitude
   (&optional __longitude)
   (if __longitude (setq _longitude __longitude)) _longitude)
  (:latitude
   (&optional __latitude)
   (if __latitude (setq _latitude __latitude)) _latitude)
  (:height
   (&optional __height)
   (if __height (setq _height __height)) _height)
  (:gaussX
   (&optional __gaussX)
   (if __gaussX (setq _gaussX __gaussX)) _gaussX)
  (:gaussY
   (&optional __gaussY)
   (if __gaussY (setq _gaussY __gaussY)) _gaussY)
  (:pitch
   (&optional __pitch)
   (if __pitch (setq _pitch __pitch)) _pitch)
  (:roll
   (&optional __roll)
   (if __roll (setq _roll __roll)) _roll)
  (:azimuth
   (&optional __azimuth)
   (if __azimuth (setq _azimuth __azimuth)) _azimuth)
  (:acc_x
   (&optional __acc_x)
   (if __acc_x (setq _acc_x __acc_x)) _acc_x)
  (:acc_y
   (&optional __acc_y)
   (if __acc_y (setq _acc_y __acc_y)) _acc_y)
  (:acc_z
   (&optional __acc_z)
   (if __acc_z (setq _acc_z __acc_z)) _acc_z)
  (:rot_x
   (&optional __rot_x)
   (if __rot_x (setq _rot_x __rot_x)) _rot_x)
  (:rot_y
   (&optional __rot_y)
   (if __rot_y (setq _rot_y __rot_y)) _rot_y)
  (:rot_z
   (&optional __rot_z)
   (if __rot_z (setq _rot_z __rot_z)) _rot_z)
  (:northVelocity
   (&optional __northVelocity)
   (if __northVelocity (setq _northVelocity __northVelocity)) _northVelocity)
  (:eastVelocity
   (&optional __eastVelocity)
   (if __eastVelocity (setq _eastVelocity __eastVelocity)) _eastVelocity)
  (:upVelocity
   (&optional __upVelocity)
   (if __upVelocity (setq _upVelocity __upVelocity)) _upVelocity)
  (:gps_confidence
   (&optional __gps_confidence)
   (if __gps_confidence (setq _gps_confidence __gps_confidence)) _gps_confidence)
  (:INS_GpsFlag_Pos
   (&optional __INS_GpsFlag_Pos)
   (if __INS_GpsFlag_Pos (setq _INS_GpsFlag_Pos __INS_GpsFlag_Pos)) _INS_GpsFlag_Pos)
  (:INS_NumSV
   (&optional __INS_NumSV)
   (if __INS_NumSV (setq _INS_NumSV __INS_NumSV)) _INS_NumSV)
  (:INS_GpsFlag_Heading
   (&optional __INS_GpsFlag_Heading)
   (if __INS_GpsFlag_Heading (setq _INS_GpsFlag_Heading __INS_GpsFlag_Heading)) _INS_GpsFlag_Heading)
  (:INS_Gps_Age
   (&optional __INS_Gps_Age)
   (if __INS_Gps_Age (setq _INS_Gps_Age __INS_Gps_Age)) _INS_Gps_Age)
  (:INS_Car_Status
   (&optional __INS_Car_Status)
   (if __INS_Car_Status (setq _INS_Car_Status __INS_Car_Status)) _INS_Car_Status)
  (:INS_Status
   (&optional __INS_Status)
   (if __INS_Status (setq _INS_Status __INS_Status)) _INS_Status)
  (:INS_VehicleAlign
   (&optional __INS_VehicleAlign)
   (if __INS_VehicleAlign (setq _INS_VehicleAlign __INS_VehicleAlign)) _INS_VehicleAlign)
  (:INS_Std_Lat
   (&optional __INS_Std_Lat)
   (if __INS_Std_Lat (setq _INS_Std_Lat __INS_Std_Lat)) _INS_Std_Lat)
  (:INS_Std_Lon
   (&optional __INS_Std_Lon)
   (if __INS_Std_Lon (setq _INS_Std_Lon __INS_Std_Lon)) _INS_Std_Lon)
  (:INS_Std_LocatHeight
   (&optional __INS_Std_LocatHeight)
   (if __INS_Std_LocatHeight (setq _INS_Std_LocatHeight __INS_Std_LocatHeight)) _INS_Std_LocatHeight)
  (:INS_Std_Heading
   (&optional __INS_Std_Heading)
   (if __INS_Std_Heading (setq _INS_Std_Heading __INS_Std_Heading)) _INS_Std_Heading)
  (:serialization-length
   ()
   (+
    ;; std_msgs/Header _header
    (send _header :serialization-length)
    ;; int32 _plat_id
    4
    ;; int32 _error_code
    4
    ;; int32 _gps_flag
    4
    ;; int32 _positionStatus
    4
    ;; uint32 _gps_week
    4
    ;; uint32 _gps_millisecond
    4
    ;; float64 _longitude
    8
    ;; float64 _latitude
    8
    ;; float64 _height
    8
    ;; float64 _gaussX
    8
    ;; float64 _gaussY
    8
    ;; float64 _pitch
    8
    ;; float64 _roll
    8
    ;; float64 _azimuth
    8
    ;; float64 _acc_x
    8
    ;; float64 _acc_y
    8
    ;; float64 _acc_z
    8
    ;; float64 _rot_x
    8
    ;; float64 _rot_y
    8
    ;; float64 _rot_z
    8
    ;; float64 _northVelocity
    8
    ;; float64 _eastVelocity
    8
    ;; float64 _upVelocity
    8
    ;; int32 _gps_confidence
    4
    ;; uint8 _INS_GpsFlag_Pos
    1
    ;; uint8 _INS_NumSV
    1
    ;; uint8 _INS_GpsFlag_Heading
    1
    ;; uint8 _INS_Gps_Age
    1
    ;; uint8 _INS_Car_Status
    1
    ;; uint8 _INS_Status
    1
    ;; uint8 _INS_VehicleAlign
    1
    ;; float64 _INS_Std_Lat
    8
    ;; float64 _INS_Std_Lon
    8
    ;; float64 _INS_Std_LocatHeight
    8
    ;; float64 _INS_Std_Heading
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
     ;; int32 _gps_flag
       (write-long _gps_flag s)
     ;; int32 _positionStatus
       (write-long _positionStatus s)
     ;; uint32 _gps_week
       (write-long _gps_week s)
     ;; uint32 _gps_millisecond
       (write-long _gps_millisecond s)
     ;; float64 _longitude
       (sys::poke _longitude (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _latitude
       (sys::poke _latitude (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _height
       (sys::poke _height (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _gaussX
       (sys::poke _gaussX (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _gaussY
       (sys::poke _gaussY (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _pitch
       (sys::poke _pitch (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _roll
       (sys::poke _roll (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _azimuth
       (sys::poke _azimuth (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _acc_x
       (sys::poke _acc_x (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _acc_y
       (sys::poke _acc_y (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _acc_z
       (sys::poke _acc_z (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _rot_x
       (sys::poke _rot_x (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _rot_y
       (sys::poke _rot_y (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _rot_z
       (sys::poke _rot_z (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _northVelocity
       (sys::poke _northVelocity (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _eastVelocity
       (sys::poke _eastVelocity (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _upVelocity
       (sys::poke _upVelocity (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; int32 _gps_confidence
       (write-long _gps_confidence s)
     ;; uint8 _INS_GpsFlag_Pos
       (write-byte _INS_GpsFlag_Pos s)
     ;; uint8 _INS_NumSV
       (write-byte _INS_NumSV s)
     ;; uint8 _INS_GpsFlag_Heading
       (write-byte _INS_GpsFlag_Heading s)
     ;; uint8 _INS_Gps_Age
       (write-byte _INS_Gps_Age s)
     ;; uint8 _INS_Car_Status
       (write-byte _INS_Car_Status s)
     ;; uint8 _INS_Status
       (write-byte _INS_Status s)
     ;; uint8 _INS_VehicleAlign
       (write-byte _INS_VehicleAlign s)
     ;; float64 _INS_Std_Lat
       (sys::poke _INS_Std_Lat (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _INS_Std_Lon
       (sys::poke _INS_Std_Lon (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _INS_Std_LocatHeight
       (sys::poke _INS_Std_LocatHeight (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _INS_Std_Heading
       (sys::poke _INS_Std_Heading (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
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
   ;; int32 _gps_flag
     (setq _gps_flag (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; int32 _positionStatus
     (setq _positionStatus (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; uint32 _gps_week
     (setq _gps_week (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; uint32 _gps_millisecond
     (setq _gps_millisecond (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; float64 _longitude
     (setq _longitude (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _latitude
     (setq _latitude (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _height
     (setq _height (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _gaussX
     (setq _gaussX (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _gaussY
     (setq _gaussY (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _pitch
     (setq _pitch (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _roll
     (setq _roll (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _azimuth
     (setq _azimuth (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _acc_x
     (setq _acc_x (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _acc_y
     (setq _acc_y (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _acc_z
     (setq _acc_z (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _rot_x
     (setq _rot_x (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _rot_y
     (setq _rot_y (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _rot_z
     (setq _rot_z (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _northVelocity
     (setq _northVelocity (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _eastVelocity
     (setq _eastVelocity (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _upVelocity
     (setq _upVelocity (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; int32 _gps_confidence
     (setq _gps_confidence (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; uint8 _INS_GpsFlag_Pos
     (setq _INS_GpsFlag_Pos (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _INS_NumSV
     (setq _INS_NumSV (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _INS_GpsFlag_Heading
     (setq _INS_GpsFlag_Heading (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _INS_Gps_Age
     (setq _INS_Gps_Age (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _INS_Car_Status
     (setq _INS_Car_Status (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _INS_Status
     (setq _INS_Status (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _INS_VehicleAlign
     (setq _INS_VehicleAlign (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; float64 _INS_Std_Lat
     (setq _INS_Std_Lat (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _INS_Std_Lon
     (setq _INS_Std_Lon (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _INS_Std_LocatHeight
     (setq _INS_Std_LocatHeight (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _INS_Std_Heading
     (setq _INS_Std_Heading (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;;
   self)
  )

(setf (get util::GpsPosition :md5sum-) "f75c3f5644c5aad5a6320ced80a0264c")
(setf (get util::GpsPosition :datatype-) "util/GpsPosition")
(setf (get util::GpsPosition :definition-)
      "#GpsPosition
Header header
int32 plat_id			#标识此消息的平台id
int32 error_code		#错误标识码
int32 gps_flag			#GPS状态
int32 positionStatus		#系统运行状态

uint32 gps_week			#GPS Week

uint32 gps_millisecond		#GPS millisecond in a week

float64 longitude		#经纬度，单位为度
float64 latitude
float64 height			#海拔,单位为m

float64 gaussX			#高斯投影位置,cm
float64 gaussY


float64 pitch			#俯仰角,x轴方向(正右方)右手定则四指方向为正,单位为0.01度
float64 roll			#翻滚角,y轴方向(正前方)右手定则四指方向为正,单位为0.01度
float64 azimuth			#航向角，单位为0.01度,向东为零度，逆时针0-360

float64 acc_x # m/s2
float64 acc_y
float64 acc_z

float64 rot_x # deg/s
float64 rot_y
float64 rot_z

float64 northVelocity		#north速度，单位为cm/s
float64 eastVelocity		#east速度,单位为cm/s
float64 upVelocity		#up速度,单位为cm/s

int32 gps_confidence	#gps定位精度置信度,根据卫星数量和INS,POS解算状态综合得出, 8-10为良好,6-7为一般,小于等于5为信号差,0为没有信号

uint8 INS_GpsFlag_Pos
uint8 INS_NumSV
uint8 INS_GpsFlag_Heading
uint8 INS_Gps_Age
uint8 INS_Car_Status
uint8 INS_Status
uint8 INS_VehicleAlign
float64 INS_Std_Lat
float64 INS_Std_Lon
float64 INS_Std_LocatHeight
float64 INS_Std_Heading
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



(provide :util/GpsPosition "f75c3f5644c5aad5a6320ced80a0264c")


