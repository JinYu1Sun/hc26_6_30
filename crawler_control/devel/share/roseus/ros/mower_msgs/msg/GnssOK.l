;; Auto-generated. Do not edit!


(when (boundp 'mower_msgs::GnssOK)
  (if (not (find-package "MOWER_MSGS"))
    (make-package "MOWER_MSGS"))
  (shadow 'GnssOK (find-package "MOWER_MSGS")))
(unless (find-package "MOWER_MSGS::GNSSOK")
  (make-package "MOWER_MSGS::GNSSOK"))

(in-package "ROS")
;;//! \htmlinclude GnssOK.msg.html


(defclass mower_msgs::GnssOK
  :super ros::object
  :slots (_is_gnss_ok ))

(defmethod mower_msgs::GnssOK
  (:init
   (&key
    ((:is_gnss_ok __is_gnss_ok) nil)
    )
   (send-super :init)
   (setq _is_gnss_ok __is_gnss_ok)
   self)
  (:is_gnss_ok
   (&optional (__is_gnss_ok :null))
   (if (not (eq __is_gnss_ok :null)) (setq _is_gnss_ok __is_gnss_ok)) _is_gnss_ok)
  (:serialization-length
   ()
   (+
    ;; bool _is_gnss_ok
    1
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; bool _is_gnss_ok
       (if _is_gnss_ok (write-byte -1 s) (write-byte 0 s))
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; bool _is_gnss_ok
     (setq _is_gnss_ok (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;;
   self)
  )

(setf (get mower_msgs::GnssOK :md5sum-) "928ba1b0bbd91610cb8761c3d298eeac")
(setf (get mower_msgs::GnssOK :datatype-) "mower_msgs/GnssOK")
(setf (get mower_msgs::GnssOK :definition-)
      "bool is_gnss_ok 


")



(provide :mower_msgs/GnssOK "928ba1b0bbd91610cb8761c3d298eeac")


