; Auto-generated. Do not edit!


(cl:in-package util-msg)


;//! \htmlinclude MapPath.msg.html

(cl:defclass <MapPath> (roslisp-msg-protocol:ros-message)
  ((path_points
    :reader path_points
    :initarg :path_points
    :type (cl:vector geometry_msgs-msg:Point32)
   :initform (cl:make-array 0 :element-type 'geometry_msgs-msg:Point32 :initial-element (cl:make-instance 'geometry_msgs-msg:Point32))))
)

(cl:defclass MapPath (<MapPath>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <MapPath>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'MapPath)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name util-msg:<MapPath> is deprecated: use util-msg:MapPath instead.")))

(cl:ensure-generic-function 'path_points-val :lambda-list '(m))
(cl:defmethod path_points-val ((m <MapPath>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:path_points-val is deprecated.  Use util-msg:path_points instead.")
  (path_points m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <MapPath>) ostream)
  "Serializes a message object of type '<MapPath>"
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'path_points))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (roslisp-msg-protocol:serialize ele ostream))
   (cl:slot-value msg 'path_points))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <MapPath>) istream)
  "Deserializes a message object of type '<MapPath>"
  (cl:let ((__ros_arr_len 0))
    (cl:setf (cl:ldb (cl:byte 8 0) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) __ros_arr_len) (cl:read-byte istream))
  (cl:setf (cl:slot-value msg 'path_points) (cl:make-array __ros_arr_len))
  (cl:let ((vals (cl:slot-value msg 'path_points)))
    (cl:dotimes (i __ros_arr_len)
    (cl:setf (cl:aref vals i) (cl:make-instance 'geometry_msgs-msg:Point32))
  (roslisp-msg-protocol:deserialize (cl:aref vals i) istream))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<MapPath>)))
  "Returns string type for a message object of type '<MapPath>"
  "util/MapPath")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'MapPath)))
  "Returns string type for a message object of type 'MapPath"
  "util/MapPath")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<MapPath>)))
  "Returns md5sum for a message object of type '<MapPath>"
  "a2e1cc73e7e7fe23e7e24d49c45f4f1a")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'MapPath)))
  "Returns md5sum for a message object of type 'MapPath"
  "a2e1cc73e7e7fe23e7e24d49c45f4f1a")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<MapPath>)))
  "Returns full string definition for message of type '<MapPath>"
  (cl:format cl:nil "#Paths between different maps~%geometry_msgs/Point32[] path_points~%~%================================================================================~%MSG: geometry_msgs/Point32~%# This contains the position of a point in free space(with 32 bits of precision).~%# It is recommeded to use Point wherever possible instead of Point32.  ~%# ~%# This recommendation is to promote interoperability.  ~%#~%# This message is designed to take up less space when sending~%# lots of points at once, as in the case of a PointCloud.  ~%~%float32 x~%float32 y~%float32 z~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'MapPath)))
  "Returns full string definition for message of type 'MapPath"
  (cl:format cl:nil "#Paths between different maps~%geometry_msgs/Point32[] path_points~%~%================================================================================~%MSG: geometry_msgs/Point32~%# This contains the position of a point in free space(with 32 bits of precision).~%# It is recommeded to use Point wherever possible instead of Point32.  ~%# ~%# This recommendation is to promote interoperability.  ~%#~%# This message is designed to take up less space when sending~%# lots of points at once, as in the case of a PointCloud.  ~%~%float32 x~%float32 y~%float32 z~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <MapPath>))
  (cl:+ 0
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'path_points) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ (roslisp-msg-protocol:serialization-length ele))))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <MapPath>))
  "Converts a ROS message object to a list"
  (cl:list 'MapPath
    (cl:cons ':path_points (path_points msg))
))
