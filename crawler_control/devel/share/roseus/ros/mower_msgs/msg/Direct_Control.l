;; Auto-generated. Do not edit!


(when (boundp 'mower_msgs::Direct_Control)
  (if (not (find-package "MOWER_MSGS"))
    (make-package "MOWER_MSGS"))
  (shadow 'Direct_Control (find-package "MOWER_MSGS")))
(unless (find-package "MOWER_MSGS::DIRECT_CONTROL")
  (make-package "MOWER_MSGS::DIRECT_CONTROL"))

(in-package "ROS")
;;//! \htmlinclude Direct_Control.msg.html


(defclass mower_msgs::Direct_Control
  :super ros::object
  :slots (_drive_mode _speed_max _mow_height ))

(defmethod mower_msgs::Direct_Control
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
    ;; float64 _speed_max
    8
    ;; uint8 _mow_height
    1
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; uint8 _drive_mode
       (write-byte _drive_mode s)
     ;; float64 _speed_max
       (sys::poke _speed_max (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; uint8 _mow_height
       (write-byte _mow_height s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; uint8 _drive_mode
     (setq _drive_mode (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; float64 _speed_max
     (setq _speed_max (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; uint8 _mow_height
     (setq _mow_height (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;;
   self)
  )

(setf (get mower_msgs::Direct_Control :md5sum-) "10579a0a02d1c1d2b089c0bf3df04393")
(setf (get mower_msgs::Direct_Control :datatype-) "mower_msgs/Direct_Control")
(setf (get mower_msgs::Direct_Control :definition-)
      "uint8 drive_mode
float64 speed_max
uint8 mow_height
")



(provide :mower_msgs/Direct_Control "10579a0a02d1c1d2b089c0bf3df04393")


