;; Auto-generated. Do not edit!


(when (boundp 'mower_msgs::CheckResult)
  (if (not (find-package "MOWER_MSGS"))
    (make-package "MOWER_MSGS"))
  (shadow 'CheckResult (find-package "MOWER_MSGS")))
(unless (find-package "MOWER_MSGS::CHECKRESULT")
  (make-package "MOWER_MSGS::CHECKRESULT"))

(in-package "ROS")
;;//! \htmlinclude CheckResult.msg.html


(defclass mower_msgs::CheckResult
  :super ros::object
  :slots (_is_checkresult_ok ))

(defmethod mower_msgs::CheckResult
  (:init
   (&key
    ((:is_checkresult_ok __is_checkresult_ok) nil)
    )
   (send-super :init)
   (setq _is_checkresult_ok __is_checkresult_ok)
   self)
  (:is_checkresult_ok
   (&optional (__is_checkresult_ok :null))
   (if (not (eq __is_checkresult_ok :null)) (setq _is_checkresult_ok __is_checkresult_ok)) _is_checkresult_ok)
  (:serialization-length
   ()
   (+
    ;; bool _is_checkresult_ok
    1
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; bool _is_checkresult_ok
       (if _is_checkresult_ok (write-byte -1 s) (write-byte 0 s))
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; bool _is_checkresult_ok
     (setq _is_checkresult_ok (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;;
   self)
  )

(setf (get mower_msgs::CheckResult :md5sum-) "d65d3e1f019288ca453b865d8cb24468")
(setf (get mower_msgs::CheckResult :datatype-) "mower_msgs/CheckResult")
(setf (get mower_msgs::CheckResult :definition-)
      "bool is_checkresult_ok


")



(provide :mower_msgs/CheckResult "d65d3e1f019288ca453b865d8cb24468")


