;; Auto-generated. Do not edit!


(when (boundp 'pure_pursuit::TurnCompleted)
  (if (not (find-package "PURE_PURSUIT"))
    (make-package "PURE_PURSUIT"))
  (shadow 'TurnCompleted (find-package "PURE_PURSUIT")))
(unless (find-package "PURE_PURSUIT::TURNCOMPLETED")
  (make-package "PURE_PURSUIT::TURNCOMPLETED"))
(unless (find-package "PURE_PURSUIT::TURNCOMPLETEDREQUEST")
  (make-package "PURE_PURSUIT::TURNCOMPLETEDREQUEST"))
(unless (find-package "PURE_PURSUIT::TURNCOMPLETEDRESPONSE")
  (make-package "PURE_PURSUIT::TURNCOMPLETEDRESPONSE"))

(in-package "ROS")





(defclass pure_pursuit::TurnCompletedRequest
  :super ros::object
  :slots (_x _y ))

(defmethod pure_pursuit::TurnCompletedRequest
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

(defclass pure_pursuit::TurnCompletedResponse
  :super ros::object
  :slots (_turn_finish ))

(defmethod pure_pursuit::TurnCompletedResponse
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

(defclass pure_pursuit::TurnCompleted
  :super ros::object
  :slots ())

(setf (get pure_pursuit::TurnCompleted :md5sum-) "5677d53f5f233f7e8f08a8788d1eb1c6")
(setf (get pure_pursuit::TurnCompleted :datatype-) "pure_pursuit/TurnCompleted")
(setf (get pure_pursuit::TurnCompleted :request) pure_pursuit::TurnCompletedRequest)
(setf (get pure_pursuit::TurnCompleted :response) pure_pursuit::TurnCompletedResponse)

(defmethod pure_pursuit::TurnCompletedRequest
  (:response () (instance pure_pursuit::TurnCompletedResponse :init)))

(setf (get pure_pursuit::TurnCompletedRequest :md5sum-) "5677d53f5f233f7e8f08a8788d1eb1c6")
(setf (get pure_pursuit::TurnCompletedRequest :datatype-) "pure_pursuit/TurnCompletedRequest")
(setf (get pure_pursuit::TurnCompletedRequest :definition-)
      "float64 x
float64 y
---
bool turn_finish

")

(setf (get pure_pursuit::TurnCompletedResponse :md5sum-) "5677d53f5f233f7e8f08a8788d1eb1c6")
(setf (get pure_pursuit::TurnCompletedResponse :datatype-) "pure_pursuit/TurnCompletedResponse")
(setf (get pure_pursuit::TurnCompletedResponse :definition-)
      "float64 x
float64 y
---
bool turn_finish

")



(provide :pure_pursuit/TurnCompleted "5677d53f5f233f7e8f08a8788d1eb1c6")


