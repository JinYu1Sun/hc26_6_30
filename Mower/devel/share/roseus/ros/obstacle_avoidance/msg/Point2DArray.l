;; Auto-generated. Do not edit!


(when (boundp 'obstacle_avoidance::Point2DArray)
  (if (not (find-package "OBSTACLE_AVOIDANCE"))
    (make-package "OBSTACLE_AVOIDANCE"))
  (shadow 'Point2DArray (find-package "OBSTACLE_AVOIDANCE")))
(unless (find-package "OBSTACLE_AVOIDANCE::POINT2DARRAY")
  (make-package "OBSTACLE_AVOIDANCE::POINT2DARRAY"))

(in-package "ROS")
;;//! \htmlinclude Point2DArray.msg.html


(defclass obstacle_avoidance::Point2DArray
  :super ros::object
  :slots (_points ))

(defmethod obstacle_avoidance::Point2DArray
  (:init
   (&key
    ((:points __points) ())
    )
   (send-super :init)
   (setq _points __points)
   self)
  (:points
   (&rest __points)
   (if (keywordp (car __points))
       (send* _points __points)
     (progn
       (if __points (setq _points (car __points)))
       _points)))
  (:serialization-length
   ()
   (+
    ;; obstacle_avoidance/Point2D[] _points
    (apply #'+ (send-all _points :serialization-length)) 4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; obstacle_avoidance/Point2D[] _points
     (write-long (length _points) s)
     (dolist (elem _points)
       (send elem :serialize s)
       )
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; obstacle_avoidance/Point2D[] _points
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _points (let (r) (dotimes (i n) (push (instance obstacle_avoidance::Point2D :init) r)) r))
     (dolist (elem- _points)
     (send elem- :deserialize buf ptr-) (incf ptr- (send elem- :serialization-length))
     ))
   ;;
   self)
  )

(setf (get obstacle_avoidance::Point2DArray :md5sum-) "0083ddac30f807eeef29c66ffedb79c7")
(setf (get obstacle_avoidance::Point2DArray :datatype-) "obstacle_avoidance/Point2DArray")
(setf (get obstacle_avoidance::Point2DArray :definition-)
      "Point2D[] points  # Array of Point2D

================================================================================
MSG: obstacle_avoidance/Point2D
# Point2D.msg
float32 x
float32 y

")



(provide :obstacle_avoidance/Point2DArray "0083ddac30f807eeef29c66ffedb79c7")


