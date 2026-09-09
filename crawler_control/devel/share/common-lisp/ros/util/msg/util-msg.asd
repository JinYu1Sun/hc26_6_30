
(cl:in-package :asdf)

(defsystem "util-msg"
  :depends-on (:roslisp-msg-protocol :roslisp-utils :geometry_msgs-msg
               :std_msgs-msg
)
  :components ((:file "_package")
    (:file "DiagnosticResult" :depends-on ("_package_DiagnosticResult"))
    (:file "_package_DiagnosticResult" :depends-on ("_package"))
    (:file "GpsPosition" :depends-on ("_package_GpsPosition"))
    (:file "_package_GpsPosition" :depends-on ("_package"))
    (:file "LocalPath" :depends-on ("_package_LocalPath"))
    (:file "_package_LocalPath" :depends-on ("_package"))
    (:file "LocalPose" :depends-on ("_package_LocalPose"))
    (:file "_package_LocalPose" :depends-on ("_package"))
    (:file "MapPath" :depends-on ("_package_MapPath"))
    (:file "_package_MapPath" :depends-on ("_package"))
    (:file "PolygonWithHoles" :depends-on ("_package_PolygonWithHoles"))
    (:file "_package_PolygonWithHoles" :depends-on ("_package"))
    (:file "PolygonWithHolesStamped" :depends-on ("_package_PolygonWithHolesStamped"))
    (:file "_package_PolygonWithHolesStamped" :depends-on ("_package"))
    (:file "Position" :depends-on ("_package_Position"))
    (:file "_package_Position" :depends-on ("_package"))
  ))