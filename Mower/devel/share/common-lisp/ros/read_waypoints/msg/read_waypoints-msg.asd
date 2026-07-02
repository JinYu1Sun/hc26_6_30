
(cl:in-package :asdf)

(defsystem "read_waypoints-msg"
  :depends-on (:roslisp-msg-protocol :roslisp-utils :std_msgs-msg
)
  :components ((:file "_package")
    (:file "LocalPath" :depends-on ("_package_LocalPath"))
    (:file "_package_LocalPath" :depends-on ("_package"))
  ))