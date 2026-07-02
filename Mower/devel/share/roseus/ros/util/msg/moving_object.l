;; Auto-generated. Do not edit!


(when (boundp 'util::moving_object)
  (if (not (find-package "UTIL"))
    (make-package "UTIL"))
  (shadow 'moving_object (find-package "UTIL")))
(unless (find-package "UTIL::MOVING_OBJECT")
  (make-package "UTIL::MOVING_OBJECT"))

(in-package "ROS")
;;//! \htmlinclude moving_object.msg.html


(defclass util::moving_object
  :super ros::object
  :slots (_x1 _y1 _x2 _y2 _x3 _y3 _x4 _y4 _zmin _zmax _speed _speed_direction _id _label _global_x _global_y _local_x _local_y ))

(defmethod util::moving_object
  (:init
   (&key
    ((:x1 __x1) 0.0)
    ((:y1 __y1) 0.0)
    ((:x2 __x2) 0.0)
    ((:y2 __y2) 0.0)
    ((:x3 __x3) 0.0)
    ((:y3 __y3) 0.0)
    ((:x4 __x4) 0.0)
    ((:y4 __y4) 0.0)
    ((:zmin __zmin) 0.0)
    ((:zmax __zmax) 0.0)
    ((:speed __speed) 0.0)
    ((:speed_direction __speed_direction) 0.0)
    ((:id __id) 0)
    ((:label __label) "")
    ((:global_x __global_x) 0.0)
    ((:global_y __global_y) 0.0)
    ((:local_x __local_x) 0.0)
    ((:local_y __local_y) 0.0)
    )
   (send-super :init)
   (setq _x1 (float __x1))
   (setq _y1 (float __y1))
   (setq _x2 (float __x2))
   (setq _y2 (float __y2))
   (setq _x3 (float __x3))
   (setq _y3 (float __y3))
   (setq _x4 (float __x4))
   (setq _y4 (float __y4))
   (setq _zmin (float __zmin))
   (setq _zmax (float __zmax))
   (setq _speed (float __speed))
   (setq _speed_direction (float __speed_direction))
   (setq _id (round __id))
   (setq _label (string __label))
   (setq _global_x (float __global_x))
   (setq _global_y (float __global_y))
   (setq _local_x (float __local_x))
   (setq _local_y (float __local_y))
   self)
  (:x1
   (&optional __x1)
   (if __x1 (setq _x1 __x1)) _x1)
  (:y1
   (&optional __y1)
   (if __y1 (setq _y1 __y1)) _y1)
  (:x2
   (&optional __x2)
   (if __x2 (setq _x2 __x2)) _x2)
  (:y2
   (&optional __y2)
   (if __y2 (setq _y2 __y2)) _y2)
  (:x3
   (&optional __x3)
   (if __x3 (setq _x3 __x3)) _x3)
  (:y3
   (&optional __y3)
   (if __y3 (setq _y3 __y3)) _y3)
  (:x4
   (&optional __x4)
   (if __x4 (setq _x4 __x4)) _x4)
  (:y4
   (&optional __y4)
   (if __y4 (setq _y4 __y4)) _y4)
  (:zmin
   (&optional __zmin)
   (if __zmin (setq _zmin __zmin)) _zmin)
  (:zmax
   (&optional __zmax)
   (if __zmax (setq _zmax __zmax)) _zmax)
  (:speed
   (&optional __speed)
   (if __speed (setq _speed __speed)) _speed)
  (:speed_direction
   (&optional __speed_direction)
   (if __speed_direction (setq _speed_direction __speed_direction)) _speed_direction)
  (:id
   (&optional __id)
   (if __id (setq _id __id)) _id)
  (:label
   (&optional __label)
   (if __label (setq _label __label)) _label)
  (:global_x
   (&optional __global_x)
   (if __global_x (setq _global_x __global_x)) _global_x)
  (:global_y
   (&optional __global_y)
   (if __global_y (setq _global_y __global_y)) _global_y)
  (:local_x
   (&optional __local_x)
   (if __local_x (setq _local_x __local_x)) _local_x)
  (:local_y
   (&optional __local_y)
   (if __local_y (setq _local_y __local_y)) _local_y)
  (:serialization-length
   ()
   (+
    ;; float32 _x1
    4
    ;; float32 _y1
    4
    ;; float32 _x2
    4
    ;; float32 _y2
    4
    ;; float32 _x3
    4
    ;; float32 _y3
    4
    ;; float32 _x4
    4
    ;; float32 _y4
    4
    ;; float32 _zmin
    4
    ;; float32 _zmax
    4
    ;; float32 _speed
    4
    ;; float32 _speed_direction
    4
    ;; uint32 _id
    4
    ;; string _label
    4 (length _label)
    ;; float32 _global_x
    4
    ;; float32 _global_y
    4
    ;; float32 _local_x
    4
    ;; float32 _local_y
    4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; float32 _x1
       (sys::poke _x1 (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _y1
       (sys::poke _y1 (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _x2
       (sys::poke _x2 (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _y2
       (sys::poke _y2 (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _x3
       (sys::poke _x3 (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _y3
       (sys::poke _y3 (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _x4
       (sys::poke _x4 (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _y4
       (sys::poke _y4 (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _zmin
       (sys::poke _zmin (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _zmax
       (sys::poke _zmax (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _speed
       (sys::poke _speed (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _speed_direction
       (sys::poke _speed_direction (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; uint32 _id
       (write-long _id s)
     ;; string _label
       (write-long (length _label) s) (princ _label s)
     ;; float32 _global_x
       (sys::poke _global_x (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _global_y
       (sys::poke _global_y (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _local_x
       (sys::poke _local_x (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _local_y
       (sys::poke _local_y (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; float32 _x1
     (setq _x1 (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _y1
     (setq _y1 (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _x2
     (setq _x2 (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _y2
     (setq _y2 (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _x3
     (setq _x3 (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _y3
     (setq _y3 (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _x4
     (setq _x4 (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _y4
     (setq _y4 (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _zmin
     (setq _zmin (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _zmax
     (setq _zmax (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _speed
     (setq _speed (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _speed_direction
     (setq _speed_direction (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; uint32 _id
     (setq _id (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; string _label
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _label (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; float32 _global_x
     (setq _global_x (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _global_y
     (setq _global_y (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _local_x
     (setq _local_x (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _local_y
     (setq _local_y (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;;
   self)
  )

(setf (get util::moving_object :md5sum-) "7cd6ca358df051582ae0af990bcae84b")
(setf (get util::moving_object :datatype-) "util/moving_object")
(setf (get util::moving_object :definition-)
      "#relative position, m
float32 x1 # x_min
float32 y1 # y_min
float32 x2 # x_min
float32 y2 # y_max
float32 x3 # x_max
float32 y3 # y_max
float32 x4 # x_max
float32 y4 # y_min

# height, cm
float32 zmin

float32 zmax

# absolute speed, m/s
float32 speed

# deg
float32 speed_direction

uint32 id
string label

# center point coordinate in localpose frame, m
float32 global_x
float32 global_y

float32 local_x
float32 local_y
")



(provide :util/moving_object "7cd6ca358df051582ae0af990bcae84b")


