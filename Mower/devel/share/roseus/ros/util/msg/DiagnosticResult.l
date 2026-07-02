;; Auto-generated. Do not edit!


(when (boundp 'util::DiagnosticResult)
  (if (not (find-package "UTIL"))
    (make-package "UTIL"))
  (shadow 'DiagnosticResult (find-package "UTIL")))
(unless (find-package "UTIL::DIAGNOSTICRESULT")
  (make-package "UTIL::DIAGNOSTICRESULT"))

(in-package "ROS")
;;//! \htmlinclude DiagnosticResult.msg.html
(if (not (find-package "STD_MSGS"))
  (ros::roseus-add-msgs "std_msgs"))


(defclass util::DiagnosticResult
  :super ros::object
  :slots (_header _fault_code ))

(defmethod util::DiagnosticResult
  (:init
   (&key
    ((:header __header) (instance std_msgs::Header :init))
    ((:fault_code __fault_code) 0)
    )
   (send-super :init)
   (setq _header __header)
   (setq _fault_code (round __fault_code))
   self)
  (:header
   (&rest __header)
   (if (keywordp (car __header))
       (send* _header __header)
     (progn
       (if __header (setq _header (car __header)))
       _header)))
  (:fault_code
   (&optional __fault_code)
   (if __fault_code (setq _fault_code __fault_code)) _fault_code)
  (:serialization-length
   ()
   (+
    ;; std_msgs/Header _header
    (send _header :serialization-length)
    ;; uint8 _fault_code
    1
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; std_msgs/Header _header
       (send _header :serialize s)
     ;; uint8 _fault_code
       (write-byte _fault_code s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; std_msgs/Header _header
     (send _header :deserialize buf ptr-) (incf ptr- (send _header :serialization-length))
   ;; uint8 _fault_code
     (setq _fault_code (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;;
   self)
  )

(setf (get util::DiagnosticResult :md5sum-) "d380b94db25b373da53086abf486f7d3")
(setf (get util::DiagnosticResult :datatype-) "util/DiagnosticResult")
(setf (get util::DiagnosticResult :definition-)
      "Header header

# Diagnostic status code
# 0: Normal operation
# 1: No position data (/Mower/position)
# 2: Stop car command received (/mower/stop_car=true)
# 3: Emergency stop for obstacle avoidance (/lawn_mower/avoid_state==3)
# 4: No global path (/lawn_mower/global_path)
# 5: PID controller node not running (pid_controller node)
# 6: CAN communication buffer full (fallback check when others are normal)
uint8 fault_code 
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



(provide :util/DiagnosticResult "d380b94db25b373da53086abf486f7d3")


