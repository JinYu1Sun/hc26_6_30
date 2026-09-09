;; Auto-generated. Do not edit!


(when (boundp 'mower_msgs::VehicleStatus)
  (if (not (find-package "MOWER_MSGS"))
    (make-package "MOWER_MSGS"))
  (shadow 'VehicleStatus (find-package "MOWER_MSGS")))
(unless (find-package "MOWER_MSGS::VEHICLESTATUS")
  (make-package "MOWER_MSGS::VEHICLESTATUS"))

(in-package "ROS")
;;//! \htmlinclude VehicleStatus.msg.html
(if (not (find-package "STD_MSGS"))
  (ros::roseus-add-msgs "std_msgs"))


(defclass mower_msgs::VehicleStatus
  :super ros::object
  :slots (_header _battery_soc _warning_state_one _warning_state_two ))

(defmethod mower_msgs::VehicleStatus
  (:init
   (&key
    ((:header __header) (instance std_msgs::Header :init))
    ((:battery_soc __battery_soc) 0)
    ((:warning_state_one __warning_state_one) 0)
    ((:warning_state_two __warning_state_two) 0)
    )
   (send-super :init)
   (setq _header __header)
   (setq _battery_soc (round __battery_soc))
   (setq _warning_state_one (round __warning_state_one))
   (setq _warning_state_two (round __warning_state_two))
   self)
  (:header
   (&rest __header)
   (if (keywordp (car __header))
       (send* _header __header)
     (progn
       (if __header (setq _header (car __header)))
       _header)))
  (:battery_soc
   (&optional __battery_soc)
   (if __battery_soc (setq _battery_soc __battery_soc)) _battery_soc)
  (:warning_state_one
   (&optional __warning_state_one)
   (if __warning_state_one (setq _warning_state_one __warning_state_one)) _warning_state_one)
  (:warning_state_two
   (&optional __warning_state_two)
   (if __warning_state_two (setq _warning_state_two __warning_state_two)) _warning_state_two)
  (:serialization-length
   ()
   (+
    ;; std_msgs/Header _header
    (send _header :serialization-length)
    ;; uint8 _battery_soc
    1
    ;; uint8 _warning_state_one
    1
    ;; uint8 _warning_state_two
    1
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; std_msgs/Header _header
       (send _header :serialize s)
     ;; uint8 _battery_soc
       (write-byte _battery_soc s)
     ;; uint8 _warning_state_one
       (write-byte _warning_state_one s)
     ;; uint8 _warning_state_two
       (write-byte _warning_state_two s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; std_msgs/Header _header
     (send _header :deserialize buf ptr-) (incf ptr- (send _header :serialization-length))
   ;; uint8 _battery_soc
     (setq _battery_soc (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _warning_state_one
     (setq _warning_state_one (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _warning_state_two
     (setq _warning_state_two (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;;
   self)
  )

(setf (get mower_msgs::VehicleStatus :md5sum-) "7d07e9e512dc3175789403cb7342112b")
(setf (get mower_msgs::VehicleStatus :datatype-) "mower_msgs/VehicleStatus")
(setf (get mower_msgs::VehicleStatus :definition-)
      "Header header
uint8 battery_soc
uint8 warning_state_one
uint8 warning_state_two
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



(provide :mower_msgs/VehicleStatus "7d07e9e512dc3175789403cb7342112b")


