;; Auto-generated. Do not edit!


(when (boundp 'util::VehicleBrakeCmd)
  (if (not (find-package "UTIL"))
    (make-package "UTIL"))
  (shadow 'VehicleBrakeCmd (find-package "UTIL")))
(unless (find-package "UTIL::VEHICLEBRAKECMD")
  (make-package "UTIL::VEHICLEBRAKECMD"))

(in-package "ROS")
;;//! \htmlinclude VehicleBrakeCmd.msg.html
(if (not (find-package "STD_MSGS"))
  (ros::roseus-add-msgs "std_msgs"))


(defclass util::VehicleBrakeCmd
  :super ros::object
  :slots (_header _BrakeCmd ))

(defmethod util::VehicleBrakeCmd
  (:init
   (&key
    ((:header __header) (instance std_msgs::Header :init))
    ((:BrakeCmd __BrakeCmd) nil)
    )
   (send-super :init)
   (setq _header __header)
   (setq _BrakeCmd __BrakeCmd)
   self)
  (:header
   (&rest __header)
   (if (keywordp (car __header))
       (send* _header __header)
     (progn
       (if __header (setq _header (car __header)))
       _header)))
  (:BrakeCmd
   (&optional (__BrakeCmd :null))
   (if (not (eq __BrakeCmd :null)) (setq _BrakeCmd __BrakeCmd)) _BrakeCmd)
  (:serialization-length
   ()
   (+
    ;; std_msgs/Header _header
    (send _header :serialization-length)
    ;; bool _BrakeCmd
    1
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; std_msgs/Header _header
       (send _header :serialize s)
     ;; bool _BrakeCmd
       (if _BrakeCmd (write-byte -1 s) (write-byte 0 s))
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; std_msgs/Header _header
     (send _header :deserialize buf ptr-) (incf ptr- (send _header :serialization-length))
   ;; bool _BrakeCmd
     (setq _BrakeCmd (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;;
   self)
  )

(setf (get util::VehicleBrakeCmd :md5sum-) "0effc161a91329ac582a70b1515cb00a")
(setf (get util::VehicleBrakeCmd :datatype-) "util/VehicleBrakeCmd")
(setf (get util::VehicleBrakeCmd :definition-)
      "Header header

bool BrakeCmd #刹车信号
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



(provide :util/VehicleBrakeCmd "0effc161a91329ac582a70b1515cb00a")


