;; Auto-generated. Do not edit!


(when (boundp 'mower_msgs::CamerargbState)
  (if (not (find-package "MOWER_MSGS"))
    (make-package "MOWER_MSGS"))
  (shadow 'CamerargbState (find-package "MOWER_MSGS")))
(unless (find-package "MOWER_MSGS::CAMERARGBSTATE")
  (make-package "MOWER_MSGS::CAMERARGBSTATE"))

(in-package "ROS")
;;//! \htmlinclude CamerargbState.msg.html


(defclass mower_msgs::CamerargbState
  :super ros::object
  :slots (_is_camerargb_ok ))

(defmethod mower_msgs::CamerargbState
  (:init
   (&key
    ((:is_camerargb_ok __is_camerargb_ok) nil)
    )
   (send-super :init)
   (setq _is_camerargb_ok __is_camerargb_ok)
   self)
  (:is_camerargb_ok
   (&optional (__is_camerargb_ok :null))
   (if (not (eq __is_camerargb_ok :null)) (setq _is_camerargb_ok __is_camerargb_ok)) _is_camerargb_ok)
  (:serialization-length
   ()
   (+
    ;; bool _is_camerargb_ok
    1
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; bool _is_camerargb_ok
       (if _is_camerargb_ok (write-byte -1 s) (write-byte 0 s))
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; bool _is_camerargb_ok
     (setq _is_camerargb_ok (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;;
   self)
  )

(setf (get mower_msgs::CamerargbState :md5sum-) "8721ee57cc9045b375b087d6e5ab5399")
(setf (get mower_msgs::CamerargbState :datatype-) "mower_msgs/CamerargbState")
(setf (get mower_msgs::CamerargbState :definition-)
      "bool is_camerargb_ok
")



(provide :mower_msgs/CamerargbState "8721ee57cc9045b375b087d6e5ab5399")


