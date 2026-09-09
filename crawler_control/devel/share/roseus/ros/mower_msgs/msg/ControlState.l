;; Auto-generated. Do not edit!


(when (boundp 'mower_msgs::ControlState)
  (if (not (find-package "MOWER_MSGS"))
    (make-package "MOWER_MSGS"))
  (shadow 'ControlState (find-package "MOWER_MSGS")))
(unless (find-package "MOWER_MSGS::CONTROLSTATE")
  (make-package "MOWER_MSGS::CONTROLSTATE"))

(in-package "ROS")
;;//! \htmlinclude ControlState.msg.html
(if (not (find-package "STD_MSGS"))
  (ros::roseus-add-msgs "std_msgs"))


(defclass mower_msgs::ControlState
  :super ros::object
  :slots (_header _control_state ))

(defmethod mower_msgs::ControlState
  (:init
   (&key
    ((:header __header) (instance std_msgs::Header :init))
    ((:control_state __control_state) 0)
    )
   (send-super :init)
   (setq _header __header)
   (setq _control_state (round __control_state))
   self)
  (:header
   (&rest __header)
   (if (keywordp (car __header))
       (send* _header __header)
     (progn
       (if __header (setq _header (car __header)))
       _header)))
  (:control_state
   (&optional __control_state)
   (if __control_state (setq _control_state __control_state)) _control_state)
  (:serialization-length
   ()
   (+
    ;; std_msgs/Header _header
    (send _header :serialization-length)
    ;; uint8 _control_state
    1
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; std_msgs/Header _header
       (send _header :serialize s)
     ;; uint8 _control_state
       (write-byte _control_state s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; std_msgs/Header _header
     (send _header :deserialize buf ptr-) (incf ptr- (send _header :serialization-length))
   ;; uint8 _control_state
     (setq _control_state (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;;
   self)
  )

(setf (get mower_msgs::ControlState :md5sum-) "15251f5c1c2c764e6732e712efff289a")
(setf (get mower_msgs::ControlState :datatype-) "mower_msgs/ControlState")
(setf (get mower_msgs::ControlState :definition-)
      "Header header

uint8 control_state

================================================================================
MSG: std_msgs/Header
# Standard metadata for higher-level stamped data types.
# This is generally used to communicate timestamped data 
# in a particular coordinate frame.
# 
# sequence ID: consecutively increasing ID 
uint32 seq
#Two-integer timestamp that is expressed as:
# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')
# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')
# time-handling sugar is provided by the client library
time stamp
#Frame this data is associated with
string frame_id

")



(provide :mower_msgs/ControlState "15251f5c1c2c764e6732e712efff289a")


