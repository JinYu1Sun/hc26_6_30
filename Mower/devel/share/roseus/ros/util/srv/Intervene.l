;; Auto-generated. Do not edit!


(when (boundp 'util::Intervene)
  (if (not (find-package "UTIL"))
    (make-package "UTIL"))
  (shadow 'Intervene (find-package "UTIL")))
(unless (find-package "UTIL::INTERVENE")
  (make-package "UTIL::INTERVENE"))
(unless (find-package "UTIL::INTERVENEREQUEST")
  (make-package "UTIL::INTERVENEREQUEST"))
(unless (find-package "UTIL::INTERVENERESPONSE")
  (make-package "UTIL::INTERVENERESPONSE"))

(in-package "ROS")





(defclass util::InterveneRequest
  :super ros::object
  :slots (_node_id _node_status _msg ))

(defmethod util::InterveneRequest
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

(defclass util::InterveneResponse
  :super ros::object
  :slots (_node_event ))

(defmethod util::InterveneResponse
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

(defclass util::Intervene
  :super ros::object
  :slots ())

(setf (get util::Intervene :md5sum-) "15f1f9695525704904993d3c41cc8ed1")
(setf (get util::Intervene :datatype-) "util/Intervene")
(setf (get util::Intervene :request) util::InterveneRequest)
(setf (get util::Intervene :response) util::InterveneResponse)

(defmethod util::InterveneRequest
  (:response () (instance util::InterveneResponse :init)))

(setf (get util::InterveneRequest :md5sum-) "15f1f9695525704904993d3c41cc8ed1")
(setf (get util::InterveneRequest :datatype-) "util/InterveneRequest")
(setf (get util::InterveneRequest :definition-)
      "int32  node_id
int32  node_status
string msg
---
int32  node_event

")

(setf (get util::InterveneResponse :md5sum-) "15f1f9695525704904993d3c41cc8ed1")
(setf (get util::InterveneResponse :datatype-) "util/InterveneResponse")
(setf (get util::InterveneResponse :definition-)
      "int32  node_id
int32  node_status
string msg
---
int32  node_event

")



(provide :util/Intervene "15f1f9695525704904993d3c41cc8ed1")


