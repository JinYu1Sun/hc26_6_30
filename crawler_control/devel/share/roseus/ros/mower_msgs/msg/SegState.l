;; Auto-generated. Do not edit!


(when (boundp 'mower_msgs::SegState)
  (if (not (find-package "MOWER_MSGS"))
    (make-package "MOWER_MSGS"))
  (shadow 'SegState (find-package "MOWER_MSGS")))
(unless (find-package "MOWER_MSGS::SEGSTATE")
  (make-package "MOWER_MSGS::SEGSTATE"))

(in-package "ROS")
;;//! \htmlinclude SegState.msg.html


(defclass mower_msgs::SegState
  :super ros::object
  :slots (_is_seg_ok ))

(defmethod mower_msgs::SegState
  (:init
   (&key
    ((:is_seg_ok __is_seg_ok) nil)
    )
   (send-super :init)
   (setq _is_seg_ok __is_seg_ok)
   self)
  (:is_seg_ok
   (&optional (__is_seg_ok :null))
   (if (not (eq __is_seg_ok :null)) (setq _is_seg_ok __is_seg_ok)) _is_seg_ok)
  (:serialization-length
   ()
   (+
    ;; bool _is_seg_ok
    1
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; bool _is_seg_ok
       (if _is_seg_ok (write-byte -1 s) (write-byte 0 s))
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; bool _is_seg_ok
     (setq _is_seg_ok (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;;
   self)
  )

(setf (get mower_msgs::SegState :md5sum-) "2a12252519f1037241c6783756d61d0f")
(setf (get mower_msgs::SegState :datatype-) "mower_msgs/SegState")
(setf (get mower_msgs::SegState :definition-)
      "bool is_seg_ok
")



(provide :mower_msgs/SegState "2a12252519f1037241c6783756d61d0f")


