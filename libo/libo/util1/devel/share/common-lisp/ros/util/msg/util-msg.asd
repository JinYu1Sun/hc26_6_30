
(cl:in-package :asdf)

(defsystem "util-msg"
  :depends-on (:roslisp-msg-protocol :roslisp-utils :std_msgs-msg
)
  :components ((:file "_package")
    (:file "GpsPosition" :depends-on ("_package_GpsPosition"))
    (:file "_package_GpsPosition" :depends-on ("_package"))
    (:file "LocalPose" :depends-on ("_package_LocalPose"))
    (:file "_package_LocalPose" :depends-on ("_package"))
    (:file "VehicleReport" :depends-on ("_package_VehicleReport"))
    (:file "_package_VehicleReport" :depends-on ("_package"))
    (:file "Vslam" :depends-on ("_package_Vslam"))
    (:file "_package_Vslam" :depends-on ("_package"))
  ))