;; Auto-generated. Do not edit!


(when (boundp 'mower_msgs::Fault_Code)
  (if (not (find-package "MOWER_MSGS"))
    (make-package "MOWER_MSGS"))
  (shadow 'Fault_Code (find-package "MOWER_MSGS")))
(unless (find-package "MOWER_MSGS::FAULT_CODE")
  (make-package "MOWER_MSGS::FAULT_CODE"))

(in-package "ROS")
;;//! \htmlinclude Fault_Code.msg.html


(defclass mower_msgs::Fault_Code
  :super ros::object
  :slots (_fault_code ))

(defmethod mower_msgs::Fault_Code
  (:init
   (&key
    ((:fault_code __fault_code) (let (r) (dotimes (i 13) (push "" r)) r))
    )
   (send-super :init)
   (setq _fault_code __fault_code)
   self)
  (:fault_code
   (&optional __fault_code)
   (if __fault_code (setq _fault_code __fault_code)) _fault_code)
  (:serialization-length
   ()
   (+
    ;; string[13] _fault_code
    (apply #'+ 4 (length _fault_code)    13)
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; string[13] _fault_code
     (write-long (length _fault_code) s)
     (dolist (elem _fault_code)
       (write-long (length elem) s) (princ elem s)
       )
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; string[13] _fault_code
   (dotimes (i (length _fault_code))
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setf (elt _fault_code i) (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
     )
   ;;
   self)
  )

(setf (get mower_msgs::Fault_Code :md5sum-) "42556c67ec9fdf90479d909abb5c32a1")
(setf (get mower_msgs::Fault_Code :datatype-) "mower_msgs/Fault_Code")
(setf (get mower_msgs::Fault_Code :definition-)
      "string[13] fault_code


")



(provide :mower_msgs/Fault_Code "42556c67ec9fdf90479d909abb5c32a1")


