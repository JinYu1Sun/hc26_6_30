;; Auto-generated. Do not edit!


(when (boundp 'mower_msgs::TaskStatus)
  (if (not (find-package "MOWER_MSGS"))
    (make-package "MOWER_MSGS"))
  (shadow 'TaskStatus (find-package "MOWER_MSGS")))
(unless (find-package "MOWER_MSGS::TASKSTATUS")
  (make-package "MOWER_MSGS::TASKSTATUS"))

(in-package "ROS")
;;//! \htmlinclude TaskStatus.msg.html


(defclass mower_msgs::TaskStatus
  :super ros::object
  :slots (_task_status ))

(defmethod mower_msgs::TaskStatus
  (:init
   (&key
    ((:task_status __task_status) "")
    )
   (send-super :init)
   (setq _task_status (string __task_status))
   self)
  (:task_status
   (&optional __task_status)
   (if __task_status (setq _task_status __task_status)) _task_status)
  (:serialization-length
   ()
   (+
    ;; string _task_status
    4 (length _task_status)
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; string _task_status
       (write-long (length _task_status) s) (princ _task_status s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; string _task_status
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _task_status (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;;
   self)
  )

(setf (get mower_msgs::TaskStatus :md5sum-) "53109e0b2984975887b08ba65c603762")
(setf (get mower_msgs::TaskStatus :datatype-) "mower_msgs/TaskStatus")
(setf (get mower_msgs::TaskStatus :definition-)
      "string task_status


")



(provide :mower_msgs/TaskStatus "53109e0b2984975887b08ba65c603762")


