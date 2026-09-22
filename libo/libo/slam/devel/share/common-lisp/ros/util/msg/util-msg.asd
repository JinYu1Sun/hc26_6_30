
(cl:in-package :asdf)

(defsystem "util-msg"
  :depends-on (:roslisp-msg-protocol :roslisp-utils :std_msgs-msg
)
  :components ((:file "_package")
    (:file "GpsPosition" :depends-on ("_package_GpsPosition"))
    (:file "_package_GpsPosition" :depends-on ("_package"))
    (:file "Vslam" :depends-on ("_package_Vslam"))
    (:file "_package_Vslam" :depends-on ("_package"))
  ))