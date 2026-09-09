
(cl:in-package :asdf)

(defsystem "pure_pursuit-srv"
  :depends-on (:roslisp-msg-protocol :roslisp-utils )
  :components ((:file "_package")
    (:file "TurnCompleted" :depends-on ("_package_TurnCompleted"))
    (:file "_package_TurnCompleted" :depends-on ("_package"))
  ))