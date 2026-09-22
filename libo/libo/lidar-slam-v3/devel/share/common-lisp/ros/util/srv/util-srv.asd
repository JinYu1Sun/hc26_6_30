
(cl:in-package :asdf)

(defsystem "util-srv"
  :depends-on (:roslisp-msg-protocol :roslisp-utils )
  :components ((:file "_package")
    (:file "Intervene" :depends-on ("_package_Intervene"))
    (:file "_package_Intervene" :depends-on ("_package"))
    (:file "ManageLocalizationMap" :depends-on ("_package_ManageLocalizationMap"))
    (:file "_package_ManageLocalizationMap" :depends-on ("_package"))
  ))