;; Auto-generated. Do not edit!


(when (boundp 'read_waypoints::TurnCompleted)
  (if (not (find-package "READ_WAYPOINTS"))
    (make-package "READ_WAYPOINTS"))
  (shadow 'TurnCompleted (find-package "READ_WAYPOINTS")))
(unless (find-package "READ_WAYPOINTS::TURNCOMPLETED")
  (make-package "READ_WAYPOINTS::TURNCOMPLETED"))
(unless (find-package "READ_WAYPOINTS::TURNCOMPLETEDREQUEST")
  (make-package "READ_WAYPOINTS::TURNCOMPLETEDREQUEST"))
(unless (find-package "READ_WAYPOINTS::TURNCOMPLETEDRESPONSE")
  (make-package "READ_WAYPOINTS::TURNCOMPLETEDRESPONSE"))

(in-package "ROS")





(defclass read_waypoints::TurnCompletedRequest
  :super ros::object
  :slots (_x _y ))

(defmethod read_waypoints::TurnCompletedRequest
  (:init
   (&key
    ((:x __x) 0.0)
    ((:y __y) 0.0)
    )
   (send-super :init)
   (setq _x (float __x))
   (setq _y (float __y))
   self)
  (:x
   (&optional __x)
   (if __x (setq _x __x)) _x)
  (:y
   (&optional __y)
   (if __y (setq _y __y)) _y)
  (:serialization-length
   ()
   (+
    ;; float64 _x
    8
    ;; float64 _y
    8
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; float64 _x
       (sys::poke _x (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; float64 _y
       (sys::poke _y (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; float64 _x
     (setq _x (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; float64 _y
     (setq _y (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;;
   self)
  )

(defclass read_waypoints::TurnCompletedResponse
  :super ros::object
  :slots (_turn_finish ))

(defmethod read_waypoints::TurnCompletedResponse
  (:init
   (&key
    ((:turn_finish __turn_finish) nil)
    )
   (send-super :init)
   (setq _turn_finish __turn_finish)
   self)
  (:turn_finish
   (&optional (__turn_finish :null))
   (if (not (eq __turn_finish :null)) (setq _turn_finish __turn_finish)) _turn_finish)
  (:serialization-length
   ()
   (+
    ;; bool _turn_finish
    1
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; bool _turn_finish
       (if _turn_finish (write-byte -1 s) (write-byte 0 s))
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; bool _turn_finish
     (setq _turn_finish (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;;
   self)
  )

(defclass read_waypoints::TurnCompleted
  :super ros::object
  :slots ())

(setf (get read_waypoints::TurnCompleted :md5sum-) "5677d53f5f233f7e8f08a8788d1eb1c6")
(setf (get read_waypoints::TurnCompleted :datatype-) "read_waypoints/TurnCompleted")
(setf (get read_waypoints::TurnCompleted :request) read_waypoints::TurnCompletedRequest)
(setf (get read_waypoints::TurnCompleted :response) read_waypoints::TurnCompletedResponse)

(defmethod read_waypoints::TurnCompletedRequest
  (:response () (instance read_waypoints::TurnCompletedResponse :init)))

(setf (get read_waypoints::TurnCompletedRequest :md5sum-) "5677d53f5f233f7e8f08a8788d1eb1c6")
(setf (get read_waypoints::TurnCompletedRequest :datatype-) "read_waypoints/TurnCompletedRequest")
(setf (get read_waypoints::TurnCompletedRequest :definition-)
      "float64 x
float64 y
---
bool turn_finish
")

(setf (get read_waypoints::TurnCompletedResponse :md5sum-) "5677d53f5f233f7e8f08a8788d1eb1c6")
(setf (get read_waypoints::TurnCompletedResponse :datatype-) "read_waypoints/TurnCompletedResponse")
(setf (get read_waypoints::TurnCompletedResponse :definition-)
      "float64 x
float64 y
---
bool turn_finish
")



(provide :read_waypoints/TurnCompleted "5677d53f5f233f7e8f08a8788d1eb1c6")


