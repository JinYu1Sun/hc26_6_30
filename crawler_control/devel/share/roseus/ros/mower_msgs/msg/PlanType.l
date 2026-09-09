;; Auto-generated. Do not edit!


(when (boundp 'mower_msgs::PlanType)
  (if (not (find-package "MOWER_MSGS"))
    (make-package "MOWER_MSGS"))
  (shadow 'PlanType (find-package "MOWER_MSGS")))
(unless (find-package "MOWER_MSGS::PLANTYPE")
  (make-package "MOWER_MSGS::PLANTYPE"))

(in-package "ROS")
;;//! \htmlinclude PlanType.msg.html


(defclass mower_msgs::PlanType
  :super ros::object
  :slots (_plan_type_state ))

(defmethod mower_msgs::PlanType
  (:init
   (&key
    ((:plan_type_state __plan_type_state) 0)
    )
   (send-super :init)
   (setq _plan_type_state (round __plan_type_state))
   self)
  (:plan_type_state
   (&optional __plan_type_state)
   (if __plan_type_state (setq _plan_type_state __plan_type_state)) _plan_type_state)
  (:serialization-length
   ()
   (+
    ;; uint8 _plan_type_state
    1
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; uint8 _plan_type_state
       (write-byte _plan_type_state s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; uint8 _plan_type_state
     (setq _plan_type_state (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;;
   self)
  )

(setf (get mower_msgs::PlanType :md5sum-) "23d6dde9eb5a6d1df00bc1332e5a8aeb")
(setf (get mower_msgs::PlanType :datatype-) "mower_msgs/PlanType")
(setf (get mower_msgs::PlanType :definition-)
      "uint8 plan_type_state
")



(provide :mower_msgs/PlanType "23d6dde9eb5a6d1df00bc1332e5a8aeb")


