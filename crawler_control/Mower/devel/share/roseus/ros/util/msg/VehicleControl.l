;; Auto-generated. Do not edit!


(when (boundp 'util::VehicleControl)
  (if (not (find-package "UTIL"))
    (make-package "UTIL"))
  (shadow 'VehicleControl (find-package "UTIL")))
(unless (find-package "UTIL::VEHICLECONTROL")
  (make-package "UTIL::VEHICLECONTROL"))

(in-package "ROS")
;;//! \htmlinclude VehicleControl.msg.html
(if (not (find-package "STD_MSGS"))
  (ros::roseus-add-msgs "std_msgs"))


(intern "*P_GEAR*" (find-package "UTIL::VEHICLECONTROL"))
(shadow '*P_GEAR* (find-package "UTIL::VEHICLECONTROL"))
(defconstant util::VehicleControl::*P_GEAR* 0)
(intern "*R_GEAR*" (find-package "UTIL::VEHICLECONTROL"))
(shadow '*R_GEAR* (find-package "UTIL::VEHICLECONTROL"))
(defconstant util::VehicleControl::*R_GEAR* 1)
(intern "*N_GEAR*" (find-package "UTIL::VEHICLECONTROL"))
(shadow '*N_GEAR* (find-package "UTIL::VEHICLECONTROL"))
(defconstant util::VehicleControl::*N_GEAR* 2)
(intern "*D_GEAR*" (find-package "UTIL::VEHICLECONTROL"))
(shadow '*D_GEAR* (find-package "UTIL::VEHICLECONTROL"))
(defconstant util::VehicleControl::*D_GEAR* 3)

(defun util::VehicleControl-to-symbol (const)
  (cond
        ((= const 0) 'util::VehicleControl::*p_Gear*)
        ((= const 1) 'util::VehicleControl::*R_Gear*)
        ((= const 2) 'util::VehicleControl::*N_Gear*)
        ((= const 3) 'util::VehicleControl::*D_Gear*)
        (t nil)))

(defclass util::VehicleControl
  :super ros::object
  :slots (_header _mover_bool _ad_control_enable _gear_model _turn_value _drive_value _mower_height _left_wheel_speed _right_wheel_speed _speed_cmd ))

(defmethod util::VehicleControl
  (:init
   (&key
    ((:header __header) (instance std_msgs::Header :init))
    ((:mover_bool __mover_bool) 0)
    ((:ad_control_enable __ad_control_enable) 0)
    ((:gear_model __gear_model) 0)
    ((:turn_value __turn_value) 0)
    ((:drive_value __drive_value) 0)
    ((:mower_height __mower_height) 0)
    ((:left_wheel_speed __left_wheel_speed) 0.0)
    ((:right_wheel_speed __right_wheel_speed) 0.0)
    ((:speed_cmd __speed_cmd) 0.0)
    )
   (send-super :init)
   (setq _header __header)
   (setq _mover_bool (round __mover_bool))
   (setq _ad_control_enable (round __ad_control_enable))
   (setq _gear_model (round __gear_model))
   (setq _turn_value (round __turn_value))
   (setq _drive_value (round __drive_value))
   (setq _mower_height (round __mower_height))
   (setq _left_wheel_speed (float __left_wheel_speed))
   (setq _right_wheel_speed (float __right_wheel_speed))
   (setq _speed_cmd (float __speed_cmd))
   self)
  (:header
   (&rest __header)
   (if (keywordp (car __header))
       (send* _header __header)
     (progn
       (if __header (setq _header (car __header)))
       _header)))
  (:mover_bool
   (&optional __mover_bool)
   (if __mover_bool (setq _mover_bool __mover_bool)) _mover_bool)
  (:ad_control_enable
   (&optional __ad_control_enable)
   (if __ad_control_enable (setq _ad_control_enable __ad_control_enable)) _ad_control_enable)
  (:gear_model
   (&optional __gear_model)
   (if __gear_model (setq _gear_model __gear_model)) _gear_model)
  (:turn_value
   (&optional __turn_value)
   (if __turn_value (setq _turn_value __turn_value)) _turn_value)
  (:drive_value
   (&optional __drive_value)
   (if __drive_value (setq _drive_value __drive_value)) _drive_value)
  (:mower_height
   (&optional __mower_height)
   (if __mower_height (setq _mower_height __mower_height)) _mower_height)
  (:left_wheel_speed
   (&optional __left_wheel_speed)
   (if __left_wheel_speed (setq _left_wheel_speed __left_wheel_speed)) _left_wheel_speed)
  (:right_wheel_speed
   (&optional __right_wheel_speed)
   (if __right_wheel_speed (setq _right_wheel_speed __right_wheel_speed)) _right_wheel_speed)
  (:speed_cmd
   (&optional __speed_cmd)
   (if __speed_cmd (setq _speed_cmd __speed_cmd)) _speed_cmd)
  (:serialization-length
   ()
   (+
    ;; std_msgs/Header _header
    (send _header :serialization-length)
    ;; uint8 _mover_bool
    1
    ;; uint8 _ad_control_enable
    1
    ;; uint8 _gear_model
    1
    ;; uint16 _turn_value
    2
    ;; uint16 _drive_value
    2
    ;; uint8 _mower_height
    1
    ;; float64 _left_wheel_speed
    8
    ;; float64 _right_wheel_speed
    8
    ;; float64 _speed_cmd
    8
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; std_msgs/Header _header
       (send _header :serialize s)
     ;; uint8 _mover_bool
       (write-byte _mover_bool s)
     ;; uint8 _ad_control_enable
       (write-byte _ad_control_enable s)
     ;; uint8 _gear_model
       (write-byte _gear_model s)
     ;; uint16 _turn_value
       (write-word _turn_value s)
     ;; uint16 _drive_value
       (write-word _drive_value s)
     ;; uint8 _mower_height
       (write-byte _mower_height s)
     ;; float64 _left_wheel_speed
       (sys::poke _left_wheel_speed (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _right_wheel_speed
       (sys::poke _right_wheel_speed (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _speed_cmd
       (sys::poke _speed_cmd (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; std_msgs/Header _header
     (send _header :deserialize buf ptr-) (incf ptr- (send _header :serialization-length))
   ;; uint8 _mover_bool
     (setq _mover_bool (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _ad_control_enable
     (setq _ad_control_enable (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _gear_model
     (setq _gear_model (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint16 _turn_value
     (setq _turn_value (sys::peek buf ptr- :short)) (incf ptr- 2)
   ;; uint16 _drive_value
     (setq _drive_value (sys::peek buf ptr- :short)) (incf ptr- 2)
   ;; uint8 _mower_height
     (setq _mower_height (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; float64 _left_wheel_speed
     (setq _left_wheel_speed (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _right_wheel_speed
     (setq _right_wheel_speed (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _speed_cmd
     (setq _speed_cmd (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;;
   self)
  )

(setf (get util::VehicleControl :md5sum-) "b4cc34e4fcf9d1ca27ccdd1d58a91a49")
(setf (get util::VehicleControl :datatype-) "util/VehicleControl")
(setf (get util::VehicleControl :definition-)
      "Header header
# VehicleCmd
# Gear_model
uint8 p_Gear = 0 # 0x00： P 档
uint8 R_Gear = 1 # 0x01： 无
uint8 N_Gear = 2 # 0x02： 转弯信号
uint8 D_Gear = 3 # 0x03： D 前进档

# ADCmd 0x310
uint8 mover_bool
uint8 ad_control_enable 
uint8 gear_model
uint16 turn_value
uint16 drive_value
uint8 mower_height

float64 left_wheel_speed  # 无
float64 right_wheel_speed # 无
float64 speed_cmd   # int8 [-128~127]






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



(provide :util/VehicleControl "b4cc34e4fcf9d1ca27ccdd1d58a91a49")


