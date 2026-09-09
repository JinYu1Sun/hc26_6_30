;; Auto-generated. Do not edit!


(when (boundp 'mower_msgs::Intervene)
  (if (not (find-package "MOWER_MSGS"))
    (make-package "MOWER_MSGS"))
  (shadow 'Intervene (find-package "MOWER_MSGS")))
(unless (find-package "MOWER_MSGS::INTERVENE")
  (make-package "MOWER_MSGS::INTERVENE"))
(unless (find-package "MOWER_MSGS::INTERVENEREQUEST")
  (make-package "MOWER_MSGS::INTERVENEREQUEST"))
(unless (find-package "MOWER_MSGS::INTERVENERESPONSE")
  (make-package "MOWER_MSGS::INTERVENERESPONSE"))

(in-package "ROS")





(defclass mower_msgs::InterveneRequest
  :super ros::object
  :slots (_node_id _node_status _msg ))

(defmethod mower_msgs::InterveneRequest
  (:init
   (&key
    ((:node_id __node_id) 0)
    ((:node_status __node_status) 0)
    ((:msg __msg) "")
    )
   (send-super :init)
   (setq _node_id (round __node_id))
   (setq _node_status (round __node_status))
   (setq _msg (string __msg))
   self)
  (:node_id
   (&optional __node_id)
   (if __node_id (setq _node_id __node_id)) _node_id)
  (:node_status
   (&optional __node_status)
   (if __node_status (setq _node_status __node_status)) _node_status)
  (:msg
   (&optional __msg)
   (if __msg (setq _msg __msg)) _msg)
  (:serialization-length
   ()
   (+
    ;; int32 _node_id
    4
    ;; int32 _node_status
    4
    ;; string _msg
    4 (length _msg)
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; int32 _node_id
       (write-long _node_id s)
     ;; int32 _node_status
       (write-long _node_status s)
     ;; string _msg
       (write-long (length _msg) s) (princ _msg s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; int32 _node_id
     (setq _node_id (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; int32 _node_status
     (setq _node_status (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; string _msg
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _msg (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;;
   self)
  )

(defclass mower_msgs::InterveneResponse
  :super ros::object
  :slots (_node_event ))

(defmethod mower_msgs::InterveneResponse
  (:init
   (&key
    ((:node_event __node_event) 0)
    )
   (send-super :init)
   (setq _node_event (round __node_event))
   self)
  (:node_event
   (&optional __node_event)
   (if __node_event (setq _node_event __node_event)) _node_event)
  (:serialization-length
   ()
   (+
    ;; int32 _node_event
    4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; int32 _node_event
       (write-long _node_event s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; int32 _node_event
     (setq _node_event (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;;
   self)
  )

(defclass mower_msgs::Intervene
  :super ros::object
  :slots ())

(setf (get mower_msgs::Intervene :md5sum-) "15f1f9695525704904993d3c41cc8ed1")
(setf (get mower_msgs::Intervene :datatype-) "mower_msgs/Intervene")
(setf (get mower_msgs::Intervene :request) mower_msgs::InterveneRequest)
(setf (get mower_msgs::Intervene :response) mower_msgs::InterveneResponse)

(defmethod mower_msgs::InterveneRequest
  (:response () (instance mower_msgs::InterveneResponse :init)))

(setf (get mower_msgs::InterveneRequest :md5sum-) "15f1f9695525704904993d3c41cc8ed1")
(setf (get mower_msgs::InterveneRequest :datatype-) "mower_msgs/InterveneRequest")
(setf (get mower_msgs::InterveneRequest :definition-)
      "int32  node_id
int32  node_status
string msg
---
int32  node_event

")

(setf (get mower_msgs::InterveneResponse :md5sum-) "15f1f9695525704904993d3c41cc8ed1")
(setf (get mower_msgs::InterveneResponse :datatype-) "mower_msgs/InterveneResponse")
(setf (get mower_msgs::InterveneResponse :definition-)
      "int32  node_id
int32  node_status
string msg
---
int32  node_event

")



(provide :mower_msgs/Intervene "15f1f9695525704904993d3c41cc8ed1")


