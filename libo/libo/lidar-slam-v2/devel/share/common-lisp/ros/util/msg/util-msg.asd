
(cl:in-package :asdf)

(defsystem "util-msg"
  :depends-on (:roslisp-msg-protocol :roslisp-utils :std_msgs-msg
)
  :components ((:file "_package")
    (:file "GPS" :depends-on ("_package_GPS"))
    (:file "_package_GPS" :depends-on ("_package"))
    (:file "GpsPosition" :depends-on ("_package_GpsPosition"))
    (:file "_package_GpsPosition" :depends-on ("_package"))
    (:file "LIOPose" :depends-on ("_package_LIOPose"))
    (:file "_package_LIOPose" :depends-on ("_package"))
    (:file "LocalPose" :depends-on ("_package_LocalPose"))
    (:file "_package_LocalPose" :depends-on ("_package"))
    (:file "Pose6D" :depends-on ("_package_Pose6D"))
    (:file "_package_Pose6D" :depends-on ("_package"))
    (:file "Position" :depends-on ("_package_Position"))
    (:file "_package_Position" :depends-on ("_package"))
    (:file "Vslam" :depends-on ("_package_Vslam"))
    (:file "_package_Vslam" :depends-on ("_package"))
    (:file "err" :depends-on ("_package_err"))
    (:file "_package_err" :depends-on ("_package"))
  ))