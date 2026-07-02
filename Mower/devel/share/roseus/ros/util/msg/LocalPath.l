;; Auto-generated. Do not edit!


(when (boundp 'util::LocalPath)
  (if (not (find-package "UTIL"))
    (make-package "UTIL"))
  (shadow 'LocalPath (find-package "UTIL")))
(unless (find-package "UTIL::LOCALPATH")
  (make-package "UTIL::LOCALPATH"))

(in-package "ROS")
;;//! \htmlinclude LocalPath.msg.html
(if (not (find-package "STD_MSGS"))
  (ros::roseus-add-msgs "std_msgs"))


(defclass util::LocalPath
  :super ros::object
  :slots (_header _point_num _pathtype _x _y _speed _heading _gear ))

(defmethod util::LocalPath
  (:init
   (&key
    ((:header __header) (instance std_msgs::Header :init))
    ((:point_num __point_num) 0)
    ((:pathtype __pathtype) 0)
    ((:x __x) (make-array 0 :initial-element 0.0 :element-type :float))
    ((:y __y) (make-array 0 :initial-element 0.0 :element-type :float))
    ((:speed __speed) (make-array 0 :initial-element 0.0 :element-type :float))
    ((:heading __heading) (make-array 0 :initial-element 0.0 :element-type :float))
    ((:gear __gear) (make-array 0 :initial-element 0 :element-type :integer))
    )
   (send-super :init)
   (setq _header __header)
   (setq _point_num (round __point_num))
   (setq _pathtype (round __pathtype))
   (setq _x __x)
   (setq _y __y)
   (setq _speed __speed)
   (setq _heading __heading)
   (setq _gear __gear)
   self)
  (:header
   (&rest __header)
   (if (keywordp (car __header))
       (send* _header __header)
     (progn
       (if __header (setq _header (car __header)))
       _header)))
  (:point_num
   (&optional __point_num)
   (if __point_num (setq _point_num __point_num)) _point_num)
  (:pathtype
   (&optional __pathtype)
   (if __pathtype (setq _pathtype __pathtype)) _pathtype)
  (:x
   (&optional __x)
   (if __x (setq _x __x)) _x)
  (:y
   (&optional __y)
   (if __y (setq _y __y)) _y)
  (:speed
   (&optional __speed)
   (if __speed (setq _speed __speed)) _speed)
  (:heading
   (&optional __heading)
   (if __heading (setq _heading __heading)) _heading)
  (:gear
   (&optional __gear)
   (if __gear (setq _gear __gear)) _gear)
  (:serialization-length
   ()
   (+
    ;; std_msgs/Header _header
    (send _header :serialization-length)
    ;; uint8 _point_num
    1
    ;; uint8 _pathtype
    1
    ;; float32[] _x
    (* 4    (length _x)) 4
    ;; float32[] _y
    (* 4    (length _y)) 4
    ;; float32[] _speed
    (* 4    (length _speed)) 4
    ;; float32[] _heading
    (* 4    (length _heading)) 4
    ;; int8[] _gear
    (* 1    (length _gear)) 4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; std_msgs/Header _header
       (send _header :serialize s)
     ;; uint8 _point_num
       (write-byte _point_num s)
     ;; uint8 _pathtype
       (write-byte _pathtype s)
     ;; float32[] _x
     (write-long (length _x) s)
     (dotimes (i (length _x))
       (sys::poke (elt _x i) (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
       )
     ;; float32[] _y
     (write-long (length _y) s)
     (dotimes (i (length _y))
       (sys::poke (elt _y i) (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
       )
     ;; float32[] _speed
     (write-long (length _speed) s)
     (dotimes (i (length _speed))
       (sys::poke (elt _speed i) (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
       )
     ;; float32[] _heading
     (write-long (length _heading) s)
     (dotimes (i (length _heading))
       (sys::poke (elt _heading i) (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
       )
     ;; int8[] _gear
     (write-long (length _gear) s)
     (dotimes (i (length _gear))
       (write-byte (elt _gear i) s)
       )
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; std_msgs/Header _header
     (send _header :deserialize buf ptr-) (incf ptr- (send _header :serialization-length))
   ;; uint8 _point_num
     (setq _point_num (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _pathtype
     (setq _pathtype (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; float32[] _x
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _x (instantiate float-vector n))
     (dotimes (i n)
     (setf (elt _x i) (sys::peek buf ptr- :float)) (incf ptr- 4)
     ))
   ;; float32[] _y
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _y (instantiate float-vector n))
     (dotimes (i n)
     (setf (elt _y i) (sys::peek buf ptr- :float)) (incf ptr- 4)
     ))
   ;; float32[] _speed
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _speed (instantiate float-vector n))
     (dotimes (i n)
     (setf (elt _speed i) (sys::peek buf ptr- :float)) (incf ptr- 4)
     ))
   ;; float32[] _heading
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _heading (instantiate float-vector n))
     (dotimes (i n)
     (setf (elt _heading i) (sys::peek buf ptr- :float)) (incf ptr- 4)
     ))
   ;; int8[] _gear
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _gear (instantiate integer-vector n))
     (dotimes (i n)
     (setf (elt _gear i) (sys::peek buf ptr- :char)) (incf ptr- 1)
     (if (> (elt _gear i) 127) (setf (elt _gear i) (- (elt _gear i) 256)))
     ))
   ;;
   self)
  )

(setf (get util::LocalPath :md5sum-) "badab2507bf5944eff88120326817f6c")
(setf (get util::LocalPath :datatype-) "util/LocalPath")
(setf (get util::LocalPath :definition-)
      "Header header

uint8 point_num

uint8 pathtype
float32[] x
# m

float32[] y
# m
float32[] speed
# km/h

float32[] heading
# deg, OX=0, OY=90, 0~360

int8[] gear
#0-P档，1-d档，2-R档


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



(provide :util/LocalPath "badab2507bf5944eff88120326817f6c")


