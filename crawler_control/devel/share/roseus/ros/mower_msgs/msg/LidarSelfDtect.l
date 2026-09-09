;; Auto-generated. Do not edit!


(when (boundp 'mower_msgs::LidarSelfDtect)
  (if (not (find-package "MOWER_MSGS"))
    (make-package "MOWER_MSGS"))
  (shadow 'LidarSelfDtect (find-package "MOWER_MSGS")))
(unless (find-package "MOWER_MSGS::LIDARSELFDTECT")
  (make-package "MOWER_MSGS::LIDARSELFDTECT"))

(in-package "ROS")
;;//! \htmlinclude LidarSelfDtect.msg.html


(defclass mower_msgs::LidarSelfDtect
  :super ros::object
  :slots (_is_lidar_ok ))

(defmethod mower_msgs::LidarSelfDtect
  (:init
   (&key
    ((:is_lidar_ok __is_lidar_ok) nil)
    )
   (send-super :init)
   (setq _is_lidar_ok __is_lidar_ok)
   self)
  (:is_lidar_ok
   (&optional (__is_lidar_ok :null))
   (if (not (eq __is_lidar_ok :null)) (setq _is_lidar_ok __is_lidar_ok)) _is_lidar_ok)
  (:serialization-length
   ()
   (+
    ;; bool _is_lidar_ok
    1
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; bool _is_lidar_ok
       (if _is_lidar_ok (write-byte -1 s) (write-byte 0 s))
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; bool _is_lidar_ok
     (setq _is_lidar_ok (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;;
   self)
  )

(setf (get mower_msgs::LidarSelfDtect :md5sum-) "61b1182e9e63e0b66c126403f70148e4")
(setf (get mower_msgs::LidarSelfDtect :datatype-) "mower_msgs/LidarSelfDtect")
(setf (get mower_msgs::LidarSelfDtect :definition-)
      "bool is_lidar_ok

")



(provide :mower_msgs/LidarSelfDtect "61b1182e9e63e0b66c126403f70148e4")


