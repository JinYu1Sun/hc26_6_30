;; Auto-generated. Do not edit!


(when (boundp 'mower_msgs::TimeAreaLeft)
  (if (not (find-package "MOWER_MSGS"))
    (make-package "MOWER_MSGS"))
  (shadow 'TimeAreaLeft (find-package "MOWER_MSGS")))
(unless (find-package "MOWER_MSGS::TIMEAREALEFT")
  (make-package "MOWER_MSGS::TIMEAREALEFT"))

(in-package "ROS")
;;//! \htmlinclude TimeAreaLeft.msg.html


(defclass mower_msgs::TimeAreaLeft
  :super ros::object
  :slots (_time_left _area_left ))

(defmethod mower_msgs::TimeAreaLeft
  (:init
   (&key
    ((:time_left __time_left) 0.0)
    ((:area_left __area_left) 0.0)
    )
   (send-super :init)
   (setq _time_left (float __time_left))
   (setq _area_left (float __area_left))
   self)
  (:time_left
   (&optional __time_left)
   (if __time_left (setq _time_left __time_left)) _time_left)
  (:area_left
   (&optional __area_left)
   (if __area_left (setq _area_left __area_left)) _area_left)
  (:serialization-length
   ()
   (+
    ;; float64 _time_left
    8
    ;; float64 _area_left
    8
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; float64 _time_left
       (sys::poke _time_left (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _area_left
       (sys::poke _area_left (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; float64 _time_left
     (setq _time_left (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _area_left
     (setq _area_left (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;;
   self)
  )

(setf (get mower_msgs::TimeAreaLeft :md5sum-) "3450601933b56d4bc50d61223a1754f6")
(setf (get mower_msgs::TimeAreaLeft :datatype-) "mower_msgs/TimeAreaLeft")
(setf (get mower_msgs::TimeAreaLeft :definition-)
      "float64 time_left
float64 area_left

")



(provide :mower_msgs/TimeAreaLeft "3450601933b56d4bc50d61223a1754f6")


