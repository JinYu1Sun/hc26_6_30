
(cl:in-package :asdf)

(defsystem "can_msgs-msg"
  :depends-on (:roslisp-msg-protocol :roslisp-utils :std_msgs-msg
)
  :components ((:file "_package")
    (:file "CanState" :depends-on ("_package_CanState"))
    (:file "_package_CanState" :depends-on ("_package"))
    (:file "Frame" :depends-on ("_package_Frame"))
    (:file "_package_Frame" :depends-on ("_package"))
  ))