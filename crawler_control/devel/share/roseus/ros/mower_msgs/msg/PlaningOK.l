;; Auto-generated. Do not edit!


(when (boundp 'mower_msgs::PlaningOK)
  (if (not (find-package "MOWER_MSGS"))
    (make-package "MOWER_MSGS"))
  (shadow 'PlaningOK (find-package "MOWER_MSGS")))
(unless (find-package "MOWER_MSGS::PLANINGOK")
  (make-package "MOWER_MSGS::PLANINGOK"))

(in-package "ROS")
;;//! \htmlinclude PlaningOK.msg.html


(defclass mower_msgs::PlaningOK
  :super ros::object
  :slots (_is_planing_ok ))

(defmethod mower_msgs::PlaningOK
  (:init
   (&key
    ((:is_planing_ok __is_planing_ok) nil)
    )
   (send-super :init)
   (setq _is_planing_ok __is_planing_ok)
   self)
  (:is_planing_ok
   (&optional (__is_planing_ok :null))
   (if (not (eq __is_planing_ok :null)) (setq _is_planing_ok __is_planing_ok)) _is_planing_ok)
  (:serialization-length
   ()
   (+
    ;; bool _is_planing_ok
    1
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; bool _is_planing_ok
       (if _is_planing_ok (write-byte -1 s) (write-byte 0 s))
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; bool _is_planing_ok
     (setq _is_planing_ok (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;;
   self)
  )

(setf (get mower_msgs::PlaningOK :md5sum-) "60643f33fa88814b437453da7ce2bc4b")
(setf (get mower_msgs::PlaningOK :datatype-) "mower_msgs/PlaningOK")
(setf (get mower_msgs::PlaningOK :definition-)
      "bool is_planing_ok

")



(provide :mower_msgs/PlaningOK "60643f33fa88814b437453da7ce2bc4b")


