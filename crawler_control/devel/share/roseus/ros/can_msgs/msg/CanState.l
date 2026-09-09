;; Auto-generated. Do not edit!


(when (boundp 'can_msgs::CanState)
  (if (not (find-package "CAN_MSGS"))
    (make-package "CAN_MSGS"))
  (shadow 'CanState (find-package "CAN_MSGS")))
(unless (find-package "CAN_MSGS::CANSTATE")
  (make-package "CAN_MSGS::CANSTATE"))

(in-package "ROS")
;;//! \htmlinclude CanState.msg.html


(intern "*CLOSED*" (find-package "CAN_MSGS::CANSTATE"))
(shadow '*CLOSED* (find-package "CAN_MSGS::CANSTATE"))
(defconstant can_msgs::CanState::*CLOSED* 0)
(intern "*OPEN*" (find-package "CAN_MSGS::CANSTATE"))
(shadow '*OPEN* (find-package "CAN_MSGS::CANSTATE"))
(defconstant can_msgs::CanState::*OPEN* 1)
(intern "*READY*" (find-package "CAN_MSGS::CANSTATE"))
(shadow '*READY* (find-package "CAN_MSGS::CANSTATE"))
(defconstant can_msgs::CanState::*READY* 2)

(defun can_msgs::CanState-to-symbol (const)
  (cond
        ((= const 0) 'can_msgs::CanState::*CLOSED*)
        ((= const 1) 'can_msgs::CanState::*OPEN*)
        ((= const 2) 'can_msgs::CanState::*READY*)
        (t nil)))

(defclass can_msgs::CanState
  :super ros::object
  :slots (_driver_state ))

(defmethod can_msgs::CanState
  (:init
   (&key
    ((:driver_state __driver_state) 0)
    )
   (send-super :init)
   (setq _driver_state (round __driver_state))
   self)
  (:driver_state
   (&optional __driver_state)
   (if __driver_state (setq _driver_state __driver_state)) _driver_state)
  (:serialization-length
   ()
   (+
    ;; uint8 _driver_state
    1
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; uint8 _driver_state
       (write-byte _driver_state s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; uint8 _driver_state
     (setq _driver_state (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;;
   self)
  )

(setf (get can_msgs::CanState :md5sum-) "ae2e3357d3e85e9ae470ea447e6ef3b6")
(setf (get can_msgs::CanState :datatype-) "can_msgs/CanState")
(setf (get can_msgs::CanState :definition-)
      "# CanState.msg
#
# Message denoting the state of the CAN Bus
#
# @author Joe Adkisson
# @version 0.1

uint8  driver_state

# enumeration values for status:
uint8 CLOSED=0
uint8 OPEN=1
uint8 READY=2
")



(provide :can_msgs/CanState "ae2e3357d3e85e9ae470ea447e6ef3b6")


