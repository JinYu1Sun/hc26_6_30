;; Auto-generated. Do not edit!


(when (boundp 'mower_msgs::MultiMapSelfDetect)
  (if (not (find-package "MOWER_MSGS"))
    (make-package "MOWER_MSGS"))
  (shadow 'MultiMapSelfDetect (find-package "MOWER_MSGS")))
(unless (find-package "MOWER_MSGS::MULTIMAPSELFDETECT")
  (make-package "MOWER_MSGS::MULTIMAPSELFDETECT"))

(in-package "ROS")
;;//! \htmlinclude MultiMapSelfDetect.msg.html


(defclass mower_msgs::MultiMapSelfDetect
  :super ros::object
  :slots (_is_multi_map_ok ))

(defmethod mower_msgs::MultiMapSelfDetect
  (:init
   (&key
    ((:is_multi_map_ok __is_multi_map_ok) nil)
    )
   (send-super :init)
   (setq _is_multi_map_ok __is_multi_map_ok)
   self)
  (:is_multi_map_ok
   (&optional (__is_multi_map_ok :null))
   (if (not (eq __is_multi_map_ok :null)) (setq _is_multi_map_ok __is_multi_map_ok)) _is_multi_map_ok)
  (:serialization-length
   ()
   (+
    ;; bool _is_multi_map_ok
    1
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; bool _is_multi_map_ok
       (if _is_multi_map_ok (write-byte -1 s) (write-byte 0 s))
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; bool _is_multi_map_ok
     (setq _is_multi_map_ok (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;;
   self)
  )

(setf (get mower_msgs::MultiMapSelfDetect :md5sum-) "31d7c6c31bbf8db62e247ca761103d55")
(setf (get mower_msgs::MultiMapSelfDetect :datatype-) "mower_msgs/MultiMapSelfDetect")
(setf (get mower_msgs::MultiMapSelfDetect :definition-)
      "bool is_multi_map_ok
")



(provide :mower_msgs/MultiMapSelfDetect "31d7c6c31bbf8db62e247ca761103d55")


