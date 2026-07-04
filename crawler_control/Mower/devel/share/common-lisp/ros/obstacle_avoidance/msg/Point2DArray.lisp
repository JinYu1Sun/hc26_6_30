; Auto-generated. Do not edit!


(cl:in-package obstacle_avoidance-msg)


;//! \htmlinclude Point2DArray.msg.html

(cl:defclass <Point2DArray> (roslisp-msg-protocol:ros-message)
  ((points
    :reader points
    :initarg :points
    :type (cl:vector obstacle_avoidance-msg:Point2D)
   :initform (cl:make-array 0 :element-type 'obstacle_avoidance-msg:Point2D :initial-element (cl:make-instance 'obstacle_avoidance-msg:Point2D))))
)

(cl:defclass Point2DArray (<Point2DArray>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <Point2DArray>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'Point2DArray)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name obstacle_avoidance-msg:<Point2DArray> is deprecated: use obstacle_avoidance-msg:Point2DArray instead.")))

(cl:ensure-generic-function 'points-val :lambda-list '(m))
(cl:defmethod points-val ((m <Point2DArray>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader obstacle_avoidance-msg:points-val is deprecated.  Use obstacle_avoidance-msg:points instead.")
  (points m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <Point2DArray>) ostream)
  "Serializes a message object of type '<Point2DArray>"
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'points))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (roslisp-msg-protocol:serialize ele ostream))
   (cl:slot-value msg 'points))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <Point2DArray>) istream)
  "Deserializes a message object of type '<Point2DArray>"
  (cl:let ((__ros_arr_len 0))
    (cl:setf (cl:ldb (cl:byte 8 0) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) __ros_arr_len) (cl:read-byte istream))
  (cl:setf (cl:slot-value msg 'points) (cl:make-array __ros_arr_len))
  (cl:let ((vals (cl:slot-value msg 'points)))
    (cl:dotimes (i __ros_arr_len)
    (cl:setf (cl:aref vals i) (cl:make-instance 'obstacle_avoidance-msg:Point2D))
  (roslisp-msg-protocol:deserialize (cl:aref vals i) istream))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<Point2DArray>)))
  "Returns string type for a message object of type '<Point2DArray>"
  "obstacle_avoidance/Point2DArray")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'Point2DArray)))
  "Returns string type for a message object of type 'Point2DArray"
  "obstacle_avoidance/Point2DArray")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<Point2DArray>)))
  "Returns md5sum for a message object of type '<Point2DArray>"
  "0083ddac30f807eeef29c66ffedb79c7")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'Point2DArray)))
  "Returns md5sum for a message object of type 'Point2DArray"
  "0083ddac30f807eeef29c66ffedb79c7")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<Point2DArray>)))
  "Returns full string definition for message of type '<Point2DArray>"
  (cl:format cl:nil "Point2D[] points  # Array of Point2D~%~%================================================================================~%MSG: obstacle_avoidance/Point2D~%# Point2D.msg~%float32 x~%float32 y~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'Point2DArray)))
  "Returns full string definition for message of type 'Point2DArray"
  (cl:format cl:nil "Point2D[] points  # Array of Point2D~%~%================================================================================~%MSG: obstacle_avoidance/Point2D~%# Point2D.msg~%float32 x~%float32 y~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <Point2DArray>))
  (cl:+ 0
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'points) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ (roslisp-msg-protocol:serialization-length ele))))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <Point2DArray>))
  "Converts a ROS message object to a list"
  (cl:list 'Point2DArray
    (cl:cons ':points (points msg))
))
