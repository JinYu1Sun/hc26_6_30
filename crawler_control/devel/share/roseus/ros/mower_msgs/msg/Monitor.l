;; Auto-generated. Do not edit!


(when (boundp 'mower_msgs::Monitor)
  (if (not (find-package "MOWER_MSGS"))
    (make-package "MOWER_MSGS"))
  (shadow 'Monitor (find-package "MOWER_MSGS")))
(unless (find-package "MOWER_MSGS::MONITOR")
  (make-package "MOWER_MSGS::MONITOR"))

(in-package "ROS")
;;//! \htmlinclude Monitor.msg.html


(defclass mower_msgs::Monitor
  :super ros::object
  :slots (_node_normal ))

(defmethod mower_msgs::Monitor
  (:init
   (&key
    ((:node_normal __node_normal) (let (r) (dotimes (i 13) (push nil r)) r))
    )
   (send-super :init)
   (setq _node_normal __node_normal)
   self)
  (:node_normal
   (&optional __node_normal)
   (if __node_normal (setq _node_normal __node_normal)) _node_normal)
  (:serialization-length
   ()
   (+
    ;; bool[13] _node_normal
    (* 1    13)
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; bool[13] _node_normal
     (dotimes (i 13)
       (if (elt _node_normal i) (write-byte -1 s) (write-byte 0 s))
       )
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; bool[13] _node_normal
   (dotimes (i (length _node_normal))
     (setf (elt _node_normal i) (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
     )
   ;;
   self)
  )

(setf (get mower_msgs::Monitor :md5sum-) "8fa716a375e1d424b9eff8c8c0b57ef5")
(setf (get mower_msgs::Monitor :datatype-) "mower_msgs/Monitor")
(setf (get mower_msgs::Monitor :definition-)
      "bool[13] node_normal


")



(provide :mower_msgs/Monitor "8fa716a375e1d424b9eff8c8c0b57ef5")


