;; Auto-generated. Do not edit!


(when (boundp 'util::VehicleReport)
  (if (not (find-package "UTIL"))
    (make-package "UTIL"))
  (shadow 'VehicleReport (find-package "UTIL")))
(unless (find-package "UTIL::VEHICLEREPORT")
  (make-package "UTIL::VEHICLEREPORT"))

(in-package "ROS")
;;//! \htmlinclude VehicleReport.msg.html
(if (not (find-package "STD_MSGS"))
  (ros::roseus-add-msgs "std_msgs"))


(defclass util::VehicleReport
  :super ros::object
  :slots (_header _vehicle_gear _clamping_brake_status _drive_mode_state _vcu_speed_req _vehicle_soc _vehicle_status_msgcntr _vehicle_speed _vehicle_brake_pressure _vehicle_steering_angle _vehicle_status_2_msgcntr ))

(defmethod util::VehicleReport
  (:init
   (&key
    ((:header __header) (instance std_msgs::Header :init))
    ((:vehicle_gear __vehicle_gear) 0)
    ((:clamping_brake_status __clamping_brake_status) 0)
    ((:drive_mode_state __drive_mode_state) 0)
    ((:vcu_speed_req __vcu_speed_req) 0.0)
    ((:vehicle_soc __vehicle_soc) 0)
    ((:vehicle_status_msgcntr __vehicle_status_msgcntr) 0)
    ((:vehicle_speed __vehicle_speed) 0.0)
    ((:vehicle_brake_pressure __vehicle_brake_pressure) 0.0)
    ((:vehicle_steering_angle __vehicle_steering_angle) 0.0)
    ((:vehicle_status_2_msgcntr __vehicle_status_2_msgcntr) 0.0)
    )
   (send-super :init)
   (setq _header __header)
   (setq _vehicle_gear (round __vehicle_gear))
   (setq _clamping_brake_status (round __clamping_brake_status))
   (setq _drive_mode_state (round __drive_mode_state))
   (setq _vcu_speed_req (float __vcu_speed_req))
   (setq _vehicle_soc (round __vehicle_soc))
   (setq _vehicle_status_msgcntr (round __vehicle_status_msgcntr))
   (setq _vehicle_speed (float __vehicle_speed))
   (setq _vehicle_brake_pressure (float __vehicle_brake_pressure))
   (setq _vehicle_steering_angle (float __vehicle_steering_angle))
   (setq _vehicle_status_2_msgcntr (float __vehicle_status_2_msgcntr))
   self)
  (:header
   (&rest __header)
   (if (keywordp (car __header))
       (send* _header __header)
     (progn
       (if __header (setq _header (car __header)))
       _header)))
  (:vehicle_gear
   (&optional __vehicle_gear)
   (if __vehicle_gear (setq _vehicle_gear __vehicle_gear)) _vehicle_gear)
  (:clamping_brake_status
   (&optional __clamping_brake_status)
   (if __clamping_brake_status (setq _clamping_brake_status __clamping_brake_status)) _clamping_brake_status)
  (:drive_mode_state
   (&optional __drive_mode_state)
   (if __drive_mode_state (setq _drive_mode_state __drive_mode_state)) _drive_mode_state)
  (:vcu_speed_req
   (&optional __vcu_speed_req)
   (if __vcu_speed_req (setq _vcu_speed_req __vcu_speed_req)) _vcu_speed_req)
  (:vehicle_soc
   (&optional __vehicle_soc)
   (if __vehicle_soc (setq _vehicle_soc __vehicle_soc)) _vehicle_soc)
  (:vehicle_status_msgcntr
   (&optional __vehicle_status_msgcntr)
   (if __vehicle_status_msgcntr (setq _vehicle_status_msgcntr __vehicle_status_msgcntr)) _vehicle_status_msgcntr)
  (:vehicle_speed
   (&optional __vehicle_speed)
   (if __vehicle_speed (setq _vehicle_speed __vehicle_speed)) _vehicle_speed)
  (:vehicle_brake_pressure
   (&optional __vehicle_brake_pressure)
   (if __vehicle_brake_pressure (setq _vehicle_brake_pressure __vehicle_brake_pressure)) _vehicle_brake_pressure)
  (:vehicle_steering_angle
   (&optional __vehicle_steering_angle)
   (if __vehicle_steering_angle (setq _vehicle_steering_angle __vehicle_steering_angle)) _vehicle_steering_angle)
  (:vehicle_status_2_msgcntr
   (&optional __vehicle_status_2_msgcntr)
   (if __vehicle_status_2_msgcntr (setq _vehicle_status_2_msgcntr __vehicle_status_2_msgcntr)) _vehicle_status_2_msgcntr)
  (:serialization-length
   ()
   (+
    ;; std_msgs/Header _header
    (send _header :serialization-length)
    ;; uint8 _vehicle_gear
    1
    ;; uint8 _clamping_brake_status
    1
    ;; uint8 _drive_mode_state
    1
    ;; float32 _vcu_speed_req
    4
    ;; uint8 _vehicle_soc
    1
    ;; uint8 _vehicle_status_msgcntr
    1
    ;; float32 _vehicle_speed
    4
    ;; float32 _vehicle_brake_pressure
    4
    ;; float32 _vehicle_steering_angle
    4
    ;; float32 _vehicle_status_2_msgcntr
    4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; std_msgs/Header _header
       (send _header :serialize s)
     ;; uint8 _vehicle_gear
       (write-byte _vehicle_gear s)
     ;; uint8 _clamping_brake_status
       (write-byte _clamping_brake_status s)
     ;; uint8 _drive_mode_state
       (write-byte _drive_mode_state s)
     ;; float32 _vcu_speed_req
       (sys::poke _vcu_speed_req (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; uint8 _vehicle_soc
       (write-byte _vehicle_soc s)
     ;; uint8 _vehicle_status_msgcntr
       (write-byte _vehicle_status_msgcntr s)
     ;; float32 _vehicle_speed
       (sys::poke _vehicle_speed (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _vehicle_brake_pressure
       (sys::poke _vehicle_brake_pressure (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _vehicle_steering_angle
       (sys::poke _vehicle_steering_angle (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _vehicle_status_2_msgcntr
       (sys::poke _vehicle_status_2_msgcntr (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; std_msgs/Header _header
     (send _header :deserialize buf ptr-) (incf ptr- (send _header :serialization-length))
   ;; uint8 _vehicle_gear
     (setq _vehicle_gear (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _clamping_brake_status
     (setq _clamping_brake_status (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _drive_mode_state
     (setq _drive_mode_state (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; float32 _vcu_speed_req
     (setq _vcu_speed_req (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; uint8 _vehicle_soc
     (setq _vehicle_soc (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _vehicle_status_msgcntr
     (setq _vehicle_status_msgcntr (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; float32 _vehicle_speed
     (setq _vehicle_speed (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _vehicle_brake_pressure
     (setq _vehicle_brake_pressure (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _vehicle_steering_angle
     (setq _vehicle_steering_angle (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _vehicle_status_2_msgcntr
     (setq _vehicle_status_2_msgcntr (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;;
   self)
  )

(setf (get util::VehicleReport :md5sum-) "8141fbcf1c26eb79107a5ea125eaf0a9")
(setf (get util::VehicleReport :datatype-) "util/VehicleReport")
(setf (get util::VehicleReport :definition-)
      "Header header

# 0x303
uint8 vehicle_gear
uint8 clamping_brake_status
uint8 drive_mode_state
float32 vcu_speed_req
uint8 vehicle_soc
uint8 vehicle_status_msgcntr

# 0x304
float32 vehicle_speed
float32 vehicle_brake_pressure
float32 vehicle_steering_angle
float32 vehicle_status_2_msgcntr

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



(provide :util/VehicleReport "8141fbcf1c26eb79107a5ea125eaf0a9")


