
(cl:in-package :asdf)

(defsystem "read_waypoints-srv"
  :depends-on (:roslisp-msg-protocol :roslisp-utils )
  :components ((:file "_package")
    (:file "TurnCompleted" :depends-on ("_package_TurnCompleted"))
    (:file "_package_TurnCompleted" :depends-on ("_package"))
  ))