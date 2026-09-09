;; Auto-generated. Do not edit!


(when (boundp 'mower_msgs::PerceptionSelfDetect)
  (if (not (find-package "MOWER_MSGS"))
    (make-package "MOWER_MSGS"))
  (shadow 'PerceptionSelfDetect (find-package "MOWER_MSGS")))
(unless (find-package "MOWER_MSGS::PERCEPTIONSELFDETECT")
  (make-package "MOWER_MSGS::PERCEPTIONSELFDETECT"))

(in-package "ROS")
;;//! \htmlinclude PerceptionSelfDetect.msg.html


(defclass mower_msgs::PerceptionSelfDetect
  :super ros::object
  :slots (_is_perception_ok ))

(defmethod mower_msgs::PerceptionSelfDetect
  (:init
   (&key
    ((:is_perception_ok __is_perception_ok) nil)
    )
   (send-super :init)
   (setq _is_perception_ok __is_perception_ok)
   self)
  (:is_perception_ok
   (&optional (__is_perception_ok :null))
   (if (not (eq __is_perception_ok :null)) (setq _is_perception_ok __is_perception_ok)) _is_perception_ok)
  (:serialization-length
   ()
   (+
    ;; bool _is_perception_ok
    1
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; bool _is_perception_ok
       (if _is_perception_ok (write-byte -1 s) (write-byte 0 s))
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; bool _is_perception_ok
     (setq _is_perception_ok (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;;
   self)
  )

(setf (get mower_msgs::PerceptionSelfDetect :md5sum-) "1e4f85ff5f6ce4dd7e761543975830a4")
(setf (get mower_msgs::PerceptionSelfDetect :datatype-) "mower_msgs/PerceptionSelfDetect")
(setf (get mower_msgs::PerceptionSelfDetect :definition-)
      "bool is_perception_ok
")



(provide :mower_msgs/PerceptionSelfDetect "1e4f85ff5f6ce4dd7e761543975830a4")


