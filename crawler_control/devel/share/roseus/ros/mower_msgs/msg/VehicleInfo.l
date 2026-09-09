;; Auto-generated. Do not edit!


(when (boundp 'mower_msgs::VehicleInfo)
  (if (not (find-package "MOWER_MSGS"))
    (make-package "MOWER_MSGS"))
  (shadow 'VehicleInfo (find-package "MOWER_MSGS")))
(unless (find-package "MOWER_MSGS::VEHICLEINFO")
  (make-package "MOWER_MSGS::VEHICLEINFO"))

(in-package "ROS")
;;//! \htmlinclude VehicleInfo.msg.html
(if (not (find-package "STD_MSGS"))
  (ros::roseus-add-msgs "std_msgs"))


(intern "*P_GEAR*" (find-package "MOWER_MSGS::VEHICLEINFO"))
(shadow '*P_GEAR* (find-package "MOWER_MSGS::VEHICLEINFO"))
(defconstant mower_msgs::VehicleInfo::*P_GEAR* 0)
(intern "*R_GEAR*" (find-package "MOWER_MSGS::VEHICLEINFO"))
(shadow '*R_GEAR* (find-package "MOWER_MSGS::VEHICLEINFO"))
(defconstant mower_msgs::VehicleInfo::*R_GEAR* 1)
(intern "*N_GEAR*" (find-package "MOWER_MSGS::VEHICLEINFO"))
(shadow '*N_GEAR* (find-package "MOWER_MSGS::VEHICLEINFO"))
(defconstant mower_msgs::VehicleInfo::*N_GEAR* 2)
(intern "*D_GEAR*" (find-package "MOWER_MSGS::VEHICLEINFO"))
(shadow '*D_GEAR* (find-package "MOWER_MSGS::VEHICLEINFO"))
(defconstant mower_msgs::VehicleInfo::*D_GEAR* 3)
(intern "*MANUALCONTROL_MODE*" (find-package "MOWER_MSGS::VEHICLEINFO"))
(shadow '*MANUALCONTROL_MODE* (find-package "MOWER_MSGS::VEHICLEINFO"))
(defconstant mower_msgs::VehicleInfo::*MANUALCONTROL_MODE* 0)
(intern "*AUTOMATICCONTROL_MODE*" (find-package "MOWER_MSGS::VEHICLEINFO"))
(shadow '*AUTOMATICCONTROL_MODE* (find-package "MOWER_MSGS::VEHICLEINFO"))
(defconstant mower_msgs::VehicleInfo::*AUTOMATICCONTROL_MODE* 1)
(intern "*REMOTECONTROL_MODE*" (find-package "MOWER_MSGS::VEHICLEINFO"))
(shadow '*REMOTECONTROL_MODE* (find-package "MOWER_MSGS::VEHICLEINFO"))
(defconstant mower_msgs::VehicleInfo::*REMOTECONTROL_MODE* 2)
(intern "*LOW_MODE*" (find-package "MOWER_MSGS::VEHICLEINFO"))
(shadow '*LOW_MODE* (find-package "MOWER_MSGS::VEHICLEINFO"))
(defconstant mower_msgs::VehicleInfo::*LOW_MODE* 2)
(intern "*MEDIUM_MODE*" (find-package "MOWER_MSGS::VEHICLEINFO"))
(shadow '*MEDIUM_MODE* (find-package "MOWER_MSGS::VEHICLEINFO"))
(defconstant mower_msgs::VehicleInfo::*MEDIUM_MODE* 1)
(intern "*HIGH_MODE*" (find-package "MOWER_MSGS::VEHICLEINFO"))
(shadow '*HIGH_MODE* (find-package "MOWER_MSGS::VEHICLEINFO"))
(defconstant mower_msgs::VehicleInfo::*HIGH_MODE* 0)

(defun mower_msgs::VehicleInfo-to-symbol (const)
  (cond
        ((= const 0) 'mower_msgs::VehicleInfo::*P_Gear*)
        ((= const 1) 'mower_msgs::VehicleInfo::*R_Gear*)
        ((= const 2) 'mower_msgs::VehicleInfo::*N_Gear*)
        ((= const 3) 'mower_msgs::VehicleInfo::*D_Gear*)
        ((= const 0) 'mower_msgs::VehicleInfo::*Manualcontrol_mode*)
        ((= const 1) 'mower_msgs::VehicleInfo::*Automaticcontrol_mode*)
        ((= const 2) 'mower_msgs::VehicleInfo::*Remotecontrol_mode*)
        ((= const 2) 'mower_msgs::VehicleInfo::*Low_mode*)
        ((= const 1) 'mower_msgs::VehicleInfo::*Medium_mode*)
        ((= const 0) 'mower_msgs::VehicleInfo::*High_mode*)
        (t nil)))

(defclass mower_msgs::VehicleInfo
  :super ros::object
  :slots (_header _speed_status _leftw_speed _rightw_speed _gear_model _drive_mode _mow_height _battery_soc ))

(defmethod mower_msgs::VehicleInfo
  (:init
   (&key
    ((:header __header) (instance std_msgs::Header :init))
    ((:speed_status __speed_status) 0.0)
    ((:leftw_speed __leftw_speed) 0.0)
    ((:rightw_speed __rightw_speed) 0.0)
    ((:gear_model __gear_model) 0)
    ((:drive_mode __drive_mode) 0)
    ((:mow_height __mow_height) 0)
    ((:battery_soc __battery_soc) 0)
    )
   (send-super :init)
   (setq _header __header)
   (setq _speed_status (float __speed_status))
   (setq _leftw_speed (float __leftw_speed))
   (setq _rightw_speed (float __rightw_speed))
   (setq _gear_model (round __gear_model))
   (setq _drive_mode (round __drive_mode))
   (setq _mow_height (round __mow_height))
   (setq _battery_soc (round __battery_soc))
   self)
  (:header
   (&rest __header)
   (if (keywordp (car __header))
       (send* _header __header)
     (progn
       (if __header (setq _header (car __header)))
       _header)))
  (:speed_status
   (&optional __speed_status)
   (if __speed_status (setq _speed_status __speed_status)) _speed_status)
  (:leftw_speed
   (&optional __leftw_speed)
   (if __leftw_speed (setq _leftw_speed __leftw_speed)) _leftw_speed)
  (:rightw_speed
   (&optional __rightw_speed)
   (if __rightw_speed (setq _rightw_speed __rightw_speed)) _rightw_speed)
  (:gear_model
   (&optional __gear_model)
   (if __gear_model (setq _gear_model __gear_model)) _gear_model)
  (:drive_mode
   (&optional __drive_mode)
   (if __drive_mode (setq _drive_mode __drive_mode)) _drive_mode)
  (:mow_height
   (&optional __mow_height)
   (if __mow_height (setq _mow_height __mow_height)) _mow_height)
  (:battery_soc
   (&optional __battery_soc)
   (if __battery_soc (setq _battery_soc __battery_soc)) _battery_soc)
  (:serialization-length
   ()
   (+
    ;; std_msgs/Header _header
    (send _header :serialization-length)
    ;; float64 _speed_status
    8
    ;; float64 _leftw_speed
    8
    ;; float64 _rightw_speed
    8
    ;; uint8 _gear_model
    1
    ;; uint8 _drive_mode
    1
    ;; uint8 _mow_height
    1
    ;; uint8 _battery_soc
    1
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; std_msgs/Header _header
       (send _header :serialize s)
     ;; float64 _speed_status
       (sys::poke _speed_status (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _leftw_speed
       (sys::poke _leftw_speed (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _rightw_speed
       (sys::poke _rightw_speed (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; uint8 _gear_model
       (write-byte _gear_model s)
     ;; uint8 _drive_mode
       (write-byte _drive_mode s)
     ;; uint8 _mow_height
       (write-byte _mow_height s)
     ;; uint8 _battery_soc
       (write-byte _battery_soc s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; std_msgs/Header _header
     (send _header :deserialize buf ptr-) (incf ptr- (send _header :serialization-length))
   ;; float64 _speed_status
     (setq _speed_status (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _leftw_speed
     (setq _leftw_speed (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _rightw_speed
     (setq _rightw_speed (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; uint8 _gear_model
     (setq _gear_model (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _drive_mode
     (setq _drive_mode (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _mow_height
     (setq _mow_height (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _battery_soc
     (setq _battery_soc (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;;
   self)
  )

(setf (get mower_msgs::VehicleInfo :md5sum-) "8a2a4b2a095bc9355e18d7be4b50f3fd")
(setf (get mower_msgs::VehicleInfo :datatype-) "mower_msgs/VehicleInfo")
(setf (get mower_msgs::VehicleInfo :definition-)
      "Header header

# AGVStatus 0x301  现履带车底控无反馈，均不成立
float64 speed_status  
float64 leftw_speed   
float64 rightw_speed
uint8 gear_model      
uint8 drive_mode
uint8 mow_height
uint8 battery_soc

#gear_model
uint8 P_Gear=0 # 0x00： P 档
uint8 R_Gear=1 # 0x01： 倒档R
uint8 N_Gear=2 # 0x02： N 档
uint8 D_Gear=3 # 0x03： D 档
#drive_model
uint8 Manualcontrol_mode=0 # 0x00：手动控制模式
uint8 Automaticcontrol_mode=1 # 0x01：自动控制模式
uint8 Remotecontrol_mode=2 # 0x02：遥控器调试模式
#mow_height
uint8 Low_mode=2 # 0x02：割草高度为低
uint8 Medium_mode=1 # 0x01：割草高度为中
uint8 High_mode=0 # 0x00：割草高度为高


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



(provide :mower_msgs/VehicleInfo "8a2a4b2a095bc9355e18d7be4b50f3fd")


