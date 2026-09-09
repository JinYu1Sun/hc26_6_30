;; Auto-generated. Do not edit!


(when (boundp 'mower_msgs::Manual_Set)
  (if (not (find-package "MOWER_MSGS"))
    (make-package "MOWER_MSGS"))
  (shadow 'Manual_Set (find-package "MOWER_MSGS")))
(unless (find-package "MOWER_MSGS::MANUAL_SET")
  (make-package "MOWER_MSGS::MANUAL_SET"))

(in-package "ROS")
;;//! \htmlinclude Manual_Set.msg.html


(defclass mower_msgs::Manual_Set
  :super ros::object
  :slots (_drive_mode _speed_max _mow_height ))

(defmethod mower_msgs::Manual_Set
  (:init
   (&key
    ((:drive_mode __drive_mode) 0)
    ((:speed_max __speed_max) 0.0)
    ((:mow_height __mow_height) 0)
    )
   (send-super :init)
   (setq _drive_mode (round __drive_mode))
   (setq _speed_max (float __speed_max))
   (setq _mow_height (round __mow_height))
   self)
  (:drive_mode
   (&optional __drive_mode)
   (if __drive_mode (setq _drive_mode __drive_mode)) _drive_mode)
  (:speed_max
   (&optional __speed_max)
   (if __speed_max (setq _speed_max __speed_max)) _speed_max)
  (:mow_height
   (&optional __mow_height)
   (if __mow_height (setq _mow_height __mow_height)) _mow_height)
  (:serialization-length
   ()
   (+
    ;; uint8 _drive_mode
    1
    ;; float32 _speed_max
    4
    ;; uint8 _mow_height
    1
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; uint8 _drive_mode
       (write-byte _drive_mode s)
     ;; float32 _speed_max
       (sys::poke _speed_max (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; uint8 _mow_height
       (write-byte _mow_height s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; uint8 _drive_mode
     (setq _drive_mode (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; float32 _speed_max
     (setq _speed_max (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; uint8 _mow_height
     (setq _mow_height (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;;
   self)
  )

(setf (get mower_msgs::Manual_Set :md5sum-) "232d61a03b6fa3c9aa8838703e52b5f4")
(setf (get mower_msgs::Manual_Set :datatype-) "mower_msgs/Manual_Set")
(setf (get mower_msgs::Manual_Set :definition-)
      "uint8 drive_mode
float32 speed_max
uint8 mow_height

")



(provide :mower_msgs/Manual_Set "232d61a03b6fa3c9aa8838703e52b5f4")


