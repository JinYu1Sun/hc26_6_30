;; Auto-generated. Do not edit!


(when (boundp 'mower_msgs::Manual_Driving_Cmd)
  (if (not (find-package "MOWER_MSGS"))
    (make-package "MOWER_MSGS"))
  (shadow 'Manual_Driving_Cmd (find-package "MOWER_MSGS")))
(unless (find-package "MOWER_MSGS::MANUAL_DRIVING_CMD")
  (make-package "MOWER_MSGS::MANUAL_DRIVING_CMD"))

(in-package "ROS")
;;//! \htmlinclude Manual_Driving_Cmd.msg.html


(defclass mower_msgs::Manual_Driving_Cmd
  :super ros::object
  :slots (_ad_control_enable _turn_value _drive_value _gear_model _mover_bool _mow_height ))

(defmethod mower_msgs::Manual_Driving_Cmd
  (:init
   (&key
    ((:ad_control_enable __ad_control_enable) 0)
    ((:turn_value __turn_value) 0)
    ((:drive_value __drive_value) 0)
    ((:gear_model __gear_model) 0)
    ((:mover_bool __mover_bool) 0)
    ((:mow_height __mow_height) 0)
    )
   (send-super :init)
   (setq _ad_control_enable (round __ad_control_enable))
   (setq _turn_value (round __turn_value))
   (setq _drive_value (round __drive_value))
   (setq _gear_model (round __gear_model))
   (setq _mover_bool (round __mover_bool))
   (setq _mow_height (round __mow_height))
   self)
  (:ad_control_enable
   (&optional __ad_control_enable)
   (if __ad_control_enable (setq _ad_control_enable __ad_control_enable)) _ad_control_enable)
  (:turn_value
   (&optional __turn_value)
   (if __turn_value (setq _turn_value __turn_value)) _turn_value)
  (:drive_value
   (&optional __drive_value)
   (if __drive_value (setq _drive_value __drive_value)) _drive_value)
  (:gear_model
   (&optional __gear_model)
   (if __gear_model (setq _gear_model __gear_model)) _gear_model)
  (:mover_bool
   (&optional __mover_bool)
   (if __mover_bool (setq _mover_bool __mover_bool)) _mover_bool)
  (:mow_height
   (&optional __mow_height)
   (if __mow_height (setq _mow_height __mow_height)) _mow_height)
  (:serialization-length
   ()
   (+
    ;; uint8 _ad_control_enable
    1
    ;; uint16 _turn_value
    2
    ;; uint16 _drive_value
    2
    ;; uint8 _gear_model
    1
    ;; uint8 _mover_bool
    1
    ;; uint8 _mow_height
    1
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; uint8 _ad_control_enable
       (write-byte _ad_control_enable s)
     ;; uint16 _turn_value
       (write-word _turn_value s)
     ;; uint16 _drive_value
       (write-word _drive_value s)
     ;; uint8 _gear_model
       (write-byte _gear_model s)
     ;; uint8 _mover_bool
       (write-byte _mover_bool s)
     ;; uint8 _mow_height
       (write-byte _mow_height s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; uint8 _ad_control_enable
     (setq _ad_control_enable (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint16 _turn_value
     (setq _turn_value (sys::peek buf ptr- :short)) (incf ptr- 2)
   ;; uint16 _drive_value
     (setq _drive_value (sys::peek buf ptr- :short)) (incf ptr- 2)
   ;; uint8 _gear_model
     (setq _gear_model (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _mover_bool
     (setq _mover_bool (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _mow_height
     (setq _mow_height (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;;
   self)
  )

(setf (get mower_msgs::Manual_Driving_Cmd :md5sum-) "2904fb773f7150886bc5690b6fc3e977")
(setf (get mower_msgs::Manual_Driving_Cmd :datatype-) "mower_msgs/Manual_Driving_Cmd")
(setf (get mower_msgs::Manual_Driving_Cmd :definition-)
      "#Manual_Driving_Cmd
uint8 ad_control_enable
uint16 turn_value
uint16 drive_value
uint8 gear_model
uint8 mover_bool
uint8 mow_height





")



(provide :mower_msgs/Manual_Driving_Cmd "2904fb773f7150886bc5690b6fc3e977")


