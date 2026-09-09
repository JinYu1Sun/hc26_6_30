;; Auto-generated. Do not edit!


(when (boundp 'mower_msgs::ControlOk)
  (if (not (find-package "MOWER_MSGS"))
    (make-package "MOWER_MSGS"))
  (shadow 'ControlOk (find-package "MOWER_MSGS")))
(unless (find-package "MOWER_MSGS::CONTROLOK")
  (make-package "MOWER_MSGS::CONTROLOK"))

(in-package "ROS")
;;//! \htmlinclude ControlOk.msg.html
(if (not (find-package "STD_MSGS"))
  (ros::roseus-add-msgs "std_msgs"))


(defclass mower_msgs::ControlOk
  :super ros::object
  :slots (_header _is_control_ok _finished ))

(defmethod mower_msgs::ControlOk
  (:init
   (&key
    ((:header __header) (instance std_msgs::Header :init))
    ((:is_control_ok __is_control_ok) nil)
    ((:finished __finished) nil)
    )
   (send-super :init)
   (setq _header __header)
   (setq _is_control_ok __is_control_ok)
   (setq _finished __finished)
   self)
  (:header
   (&rest __header)
   (if (keywordp (car __header))
       (send* _header __header)
     (progn
       (if __header (setq _header (car __header)))
       _header)))
  (:is_control_ok
   (&optional (__is_control_ok :null))
   (if (not (eq __is_control_ok :null)) (setq _is_control_ok __is_control_ok)) _is_control_ok)
  (:finished
   (&optional (__finished :null))
   (if (not (eq __finished :null)) (setq _finished __finished)) _finished)
  (:serialization-length
   ()
   (+
    ;; std_msgs/Header _header
    (send _header :serialization-length)
    ;; bool _is_control_ok
    1
    ;; bool _finished
    1
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; std_msgs/Header _header
       (send _header :serialize s)
     ;; bool _is_control_ok
       (if _is_control_ok (write-byte -1 s) (write-byte 0 s))
     ;; bool _finished
       (if _finished (write-byte -1 s) (write-byte 0 s))
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; std_msgs/Header _header
     (send _header :deserialize buf ptr-) (incf ptr- (send _header :serialization-length))
   ;; bool _is_control_ok
     (setq _is_control_ok (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; bool _finished
     (setq _finished (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;;
   self)
  )

(setf (get mower_msgs::ControlOk :md5sum-) "b1b519c21944b4cf1e0569f6c2cb3a72")
(setf (get mower_msgs::ControlOk :datatype-) "mower_msgs/ControlOk")
(setf (get mower_msgs::ControlOk :definition-)
      "Header header


bool is_control_ok
bool finished

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



(provide :mower_msgs/ControlOk "b1b519c21944b4cf1e0569f6c2cb3a72")


