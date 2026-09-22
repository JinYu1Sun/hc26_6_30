;; Auto-generated. Do not edit!


(when (boundp 'util::ManageLocalizationMap)
  (if (not (find-package "UTIL"))
    (make-package "UTIL"))
  (shadow 'ManageLocalizationMap (find-package "UTIL")))
(unless (find-package "UTIL::MANAGELOCALIZATIONMAP")
  (make-package "UTIL::MANAGELOCALIZATIONMAP"))
(unless (find-package "UTIL::MANAGELOCALIZATIONMAPREQUEST")
  (make-package "UTIL::MANAGELOCALIZATIONMAPREQUEST"))
(unless (find-package "UTIL::MANAGELOCALIZATIONMAPRESPONSE")
  (make-package "UTIL::MANAGELOCALIZATIONMAPRESPONSE"))

(in-package "ROS")





(intern "*SAVE_CURRENT_AND_ACTIVATE*" (find-package "UTIL::MANAGELOCALIZATIONMAPREQUEST"))
(shadow '*SAVE_CURRENT_AND_ACTIVATE* (find-package "UTIL::MANAGELOCALIZATIONMAPREQUEST"))
(defconstant util::ManageLocalizationMapRequest::*SAVE_CURRENT_AND_ACTIVATE* 1)
(intern "*ACTIVATE_EXISTING*" (find-package "UTIL::MANAGELOCALIZATIONMAPREQUEST"))
(shadow '*ACTIVATE_EXISTING* (find-package "UTIL::MANAGELOCALIZATIONMAPREQUEST"))
(defconstant util::ManageLocalizationMapRequest::*ACTIVATE_EXISTING* 2)

(defun util::ManageLocalizationMapRequest-to-symbol (const)
  (cond
        ((= const 1) 'util::ManageLocalizationMapRequest::*SAVE_CURRENT_AND_ACTIVATE*)
        ((= const 2) 'util::ManageLocalizationMapRequest::*ACTIVATE_EXISTING*)
        (t nil)))

(defclass util::ManageLocalizationMapRequest
  :super ros::object
  :slots (_command _map_name _overwrite ))

(defmethod util::ManageLocalizationMapRequest
  (:init
   (&key
    ((:command __command) 0)
    ((:map_name __map_name) "")
    ((:overwrite __overwrite) nil)
    )
   (send-super :init)
   (setq _command (round __command))
   (setq _map_name (string __map_name))
   (setq _overwrite __overwrite)
   self)
  (:command
   (&optional __command)
   (if __command (setq _command __command)) _command)
  (:map_name
   (&optional __map_name)
   (if __map_name (setq _map_name __map_name)) _map_name)
  (:overwrite
   (&optional (__overwrite :null))
   (if (not (eq __overwrite :null)) (setq _overwrite __overwrite)) _overwrite)
  (:serialization-length
   ()
   (+
    ;; uint8 _command
    1
    ;; string _map_name
    4 (length _map_name)
    ;; bool _overwrite
    1
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; uint8 _command
       (write-byte _command s)
     ;; string _map_name
       (write-long (length _map_name) s) (princ _map_name s)
     ;; bool _overwrite
       (if _overwrite (write-byte -1 s) (write-byte 0 s))
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; uint8 _command
     (setq _command (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; string _map_name
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _map_name (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; bool _overwrite
     (setq _overwrite (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;;
   self)
  )

(defclass util::ManageLocalizationMapResponse
  :super ros::object
  :slots (_success _alignment_preserved _switch_pending _active_map _active_map_uuid _pending_map _pending_map_uuid _map_schema_version _coordinate_frame _origin_checksum _reason ))

(defmethod util::ManageLocalizationMapResponse
  (:init
   (&key
    ((:success __success) nil)
    ((:alignment_preserved __alignment_preserved) nil)
    ((:switch_pending __switch_pending) nil)
    ((:active_map __active_map) "")
    ((:active_map_uuid __active_map_uuid) "")
    ((:pending_map __pending_map) "")
    ((:pending_map_uuid __pending_map_uuid) "")
    ((:map_schema_version __map_schema_version) 0)
    ((:coordinate_frame __coordinate_frame) "")
    ((:origin_checksum __origin_checksum) "")
    ((:reason __reason) "")
    )
   (send-super :init)
   (setq _success __success)
   (setq _alignment_preserved __alignment_preserved)
   (setq _switch_pending __switch_pending)
   (setq _active_map (string __active_map))
   (setq _active_map_uuid (string __active_map_uuid))
   (setq _pending_map (string __pending_map))
   (setq _pending_map_uuid (string __pending_map_uuid))
   (setq _map_schema_version (round __map_schema_version))
   (setq _coordinate_frame (string __coordinate_frame))
   (setq _origin_checksum (string __origin_checksum))
   (setq _reason (string __reason))
   self)
  (:success
   (&optional (__success :null))
   (if (not (eq __success :null)) (setq _success __success)) _success)
  (:alignment_preserved
   (&optional (__alignment_preserved :null))
   (if (not (eq __alignment_preserved :null)) (setq _alignment_preserved __alignment_preserved)) _alignment_preserved)
  (:switch_pending
   (&optional (__switch_pending :null))
   (if (not (eq __switch_pending :null)) (setq _switch_pending __switch_pending)) _switch_pending)
  (:active_map
   (&optional __active_map)
   (if __active_map (setq _active_map __active_map)) _active_map)
  (:active_map_uuid
   (&optional __active_map_uuid)
   (if __active_map_uuid (setq _active_map_uuid __active_map_uuid)) _active_map_uuid)
  (:pending_map
   (&optional __pending_map)
   (if __pending_map (setq _pending_map __pending_map)) _pending_map)
  (:pending_map_uuid
   (&optional __pending_map_uuid)
   (if __pending_map_uuid (setq _pending_map_uuid __pending_map_uuid)) _pending_map_uuid)
  (:map_schema_version
   (&optional __map_schema_version)
   (if __map_schema_version (setq _map_schema_version __map_schema_version)) _map_schema_version)
  (:coordinate_frame
   (&optional __coordinate_frame)
   (if __coordinate_frame (setq _coordinate_frame __coordinate_frame)) _coordinate_frame)
  (:origin_checksum
   (&optional __origin_checksum)
   (if __origin_checksum (setq _origin_checksum __origin_checksum)) _origin_checksum)
  (:reason
   (&optional __reason)
   (if __reason (setq _reason __reason)) _reason)
  (:serialization-length
   ()
   (+
    ;; bool _success
    1
    ;; bool _alignment_preserved
    1
    ;; bool _switch_pending
    1
    ;; string _active_map
    4 (length _active_map)
    ;; string _active_map_uuid
    4 (length _active_map_uuid)
    ;; string _pending_map
    4 (length _pending_map)
    ;; string _pending_map_uuid
    4 (length _pending_map_uuid)
    ;; uint32 _map_schema_version
    4
    ;; string _coordinate_frame
    4 (length _coordinate_frame)
    ;; string _origin_checksum
    4 (length _origin_checksum)
    ;; string _reason
    4 (length _reason)
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; bool _success
       (if _success (write-byte -1 s) (write-byte 0 s))
     ;; bool _alignment_preserved
       (if _alignment_preserved (write-byte -1 s) (write-byte 0 s))
     ;; bool _switch_pending
       (if _switch_pending (write-byte -1 s) (write-byte 0 s))
     ;; string _active_map
       (write-long (length _active_map) s) (princ _active_map s)
     ;; string _active_map_uuid
       (write-long (length _active_map_uuid) s) (princ _active_map_uuid s)
     ;; string _pending_map
       (write-long (length _pending_map) s) (princ _pending_map s)
     ;; string _pending_map_uuid
       (write-long (length _pending_map_uuid) s) (princ _pending_map_uuid s)
     ;; uint32 _map_schema_version
       (write-long _map_schema_version s)
     ;; string _coordinate_frame
       (write-long (length _coordinate_frame) s) (princ _coordinate_frame s)
     ;; string _origin_checksum
       (write-long (length _origin_checksum) s) (princ _origin_checksum s)
     ;; string _reason
       (write-long (length _reason) s) (princ _reason s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; bool _success
     (setq _success (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; bool _alignment_preserved
     (setq _alignment_preserved (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; bool _switch_pending
     (setq _switch_pending (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; string _active_map
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _active_map (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; string _active_map_uuid
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _active_map_uuid (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; string _pending_map
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _pending_map (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; string _pending_map_uuid
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _pending_map_uuid (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; uint32 _map_schema_version
     (setq _map_schema_version (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; string _coordinate_frame
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _coordinate_frame (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; string _origin_checksum
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _origin_checksum (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;; string _reason
     (let (n) (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4) (setq _reason (subseq buf ptr- (+ ptr- n))) (incf ptr- n))
   ;;
   self)
  )

(defclass util::ManageLocalizationMap
  :super ros::object
  :slots ())

(setf (get util::ManageLocalizationMap :md5sum-) "f6eb786c2817a22f906c5e66a8ae611a")
(setf (get util::ManageLocalizationMap :datatype-) "util/ManageLocalizationMap")
(setf (get util::ManageLocalizationMap :request) util::ManageLocalizationMapRequest)
(setf (get util::ManageLocalizationMap :response) util::ManageLocalizationMapResponse)

(defmethod util::ManageLocalizationMapRequest
  (:response () (instance util::ManageLocalizationMapResponse :init)))

(setf (get util::ManageLocalizationMapRequest :md5sum-) "f6eb786c2817a22f906c5e66a8ae611a")
(setf (get util::ManageLocalizationMapRequest :datatype-) "util/ManageLocalizationMapRequest")
(setf (get util::ManageLocalizationMapRequest :definition-)
      "# Transactional map lifecycle API owned by fusion_4dof.
uint8 SAVE_CURRENT_AND_ACTIVATE=1
uint8 ACTIVATE_EXISTING=2

uint8 command
string map_name
bool overwrite
---
bool success
bool alignment_preserved
bool switch_pending
string active_map
string active_map_uuid
string pending_map
string pending_map_uuid
uint32 map_schema_version
string coordinate_frame
string origin_checksum
string reason

")

(setf (get util::ManageLocalizationMapResponse :md5sum-) "f6eb786c2817a22f906c5e66a8ae611a")
(setf (get util::ManageLocalizationMapResponse :datatype-) "util/ManageLocalizationMapResponse")
(setf (get util::ManageLocalizationMapResponse :definition-)
      "# Transactional map lifecycle API owned by fusion_4dof.
uint8 SAVE_CURRENT_AND_ACTIVATE=1
uint8 ACTIVATE_EXISTING=2

uint8 command
string map_name
bool overwrite
---
bool success
bool alignment_preserved
bool switch_pending
string active_map
string active_map_uuid
string pending_map
string pending_map_uuid
uint32 map_schema_version
string coordinate_frame
string origin_checksum
string reason

")



(provide :util/ManageLocalizationMap "f6eb786c2817a22f906c5e66a8ae611a")


