;; Auto-generated. Do not edit!


(when (boundp 'mower_msgs::CameraState)
  (if (not (find-package "MOWER_MSGS"))
    (make-package "MOWER_MSGS"))
  (shadow 'CameraState (find-package "MOWER_MSGS")))
(unless (find-package "MOWER_MSGS::CAMERASTATE")
  (make-package "MOWER_MSGS::CAMERASTATE"))

(in-package "ROS")
;;//! \htmlinclude CameraState.msg.html


(defclass mower_msgs::CameraState
  :super ros::object
  :slots (_is_camera_ok ))

(defmethod mower_msgs::CameraState
  (:init
   (&key
    ((:is_camera_ok __is_camera_ok) nil)
    )
   (send-super :init)
   (setq _is_camera_ok __is_camera_ok)
   self)
  (:is_camera_ok
   (&optional (__is_camera_ok :null))
   (if (not (eq __is_camera_ok :null)) (setq _is_camera_ok __is_camera_ok)) _is_camera_ok)
  (:serialization-length
   ()
   (+
    ;; bool _is_camera_ok
    1
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; bool _is_camera_ok
       (if _is_camera_ok (write-byte -1 s) (write-byte 0 s))
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; bool _is_camera_ok
     (setq _is_camera_ok (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;;
   self)
  )

(setf (get mower_msgs::CameraState :md5sum-) "56198635b7732d260a91b05411870558")
(setf (get mower_msgs::CameraState :datatype-) "mower_msgs/CameraState")
(setf (get mower_msgs::CameraState :definition-)
      "bool is_camera_ok

")



(provide :mower_msgs/CameraState "56198635b7732d260a91b05411870558")


