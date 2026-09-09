;; Auto-generated. Do not edit!


(when (boundp 'mower_msgs::VslamState)
  (if (not (find-package "MOWER_MSGS"))
    (make-package "MOWER_MSGS"))
  (shadow 'VslamState (find-package "MOWER_MSGS")))
(unless (find-package "MOWER_MSGS::VSLAMSTATE")
  (make-package "MOWER_MSGS::VSLAMSTATE"))

(in-package "ROS")
;;//! \htmlinclude VslamState.msg.html


(defclass mower_msgs::VslamState
  :super ros::object
  :slots (_is_vslam_ok ))

(defmethod mower_msgs::VslamState
  (:init
   (&key
    ((:is_vslam_ok __is_vslam_ok) nil)
    )
   (send-super :init)
   (setq _is_vslam_ok __is_vslam_ok)
   self)
  (:is_vslam_ok
   (&optional (__is_vslam_ok :null))
   (if (not (eq __is_vslam_ok :null)) (setq _is_vslam_ok __is_vslam_ok)) _is_vslam_ok)
  (:serialization-length
   ()
   (+
    ;; bool _is_vslam_ok
    1
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; bool _is_vslam_ok
       (if _is_vslam_ok (write-byte -1 s) (write-byte 0 s))
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; bool _is_vslam_ok
     (setq _is_vslam_ok (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;;
   self)
  )

(setf (get mower_msgs::VslamState :md5sum-) "d58db3b3d9624dbcc385f025b40a612c")
(setf (get mower_msgs::VslamState :datatype-) "mower_msgs/VslamState")
(setf (get mower_msgs::VslamState :definition-)
      "bool is_vslam_ok
")



(provide :mower_msgs/VslamState "d58db3b3d9624dbcc385f025b40a612c")


