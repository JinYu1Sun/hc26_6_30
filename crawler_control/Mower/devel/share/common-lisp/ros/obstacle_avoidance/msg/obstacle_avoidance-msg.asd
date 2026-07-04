
(cl:in-package :asdf)

(defsystem "obstacle_avoidance-msg"
  :depends-on (:roslisp-msg-protocol :roslisp-utils :std_msgs-msg
)
  :components ((:file "_package")
    (:file "LocalPath" :depends-on ("_package_LocalPath"))
    (:file "_package_LocalPath" :depends-on ("_package"))
    (:file "PathPoint" :depends-on ("_package_PathPoint"))
    (:file "_package_PathPoint" :depends-on ("_package"))
    (:file "Point2D" :depends-on ("_package_Point2D"))
    (:file "_package_Point2D" :depends-on ("_package"))
    (:file "Point2DArray" :depends-on ("_package_Point2DArray"))
    (:file "_package_Point2DArray" :depends-on ("_package"))
  ))