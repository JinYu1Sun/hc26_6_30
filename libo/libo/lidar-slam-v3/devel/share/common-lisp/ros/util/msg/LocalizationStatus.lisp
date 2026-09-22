; Auto-generated. Do not edit!


(cl:in-package util-msg)


;//! \htmlinclude LocalizationStatus.msg.html

(cl:defclass <LocalizationStatus> (roslisp-msg-protocol:ros-message)
  ((header
    :reader header
    :initarg :header
    :type std_msgs-msg:Header
    :initform (cl:make-instance 'std_msgs-msg:Header))
   (reporter
    :reader reporter
    :initarg :reporter
    :type cl:string
    :initform "")
   (requested_mode
    :reader requested_mode
    :initarg :requested_mode
    :type cl:fixnum
    :initform 0)
   (active_mode
    :reader active_mode
    :initarg :active_mode
    :type cl:fixnum
    :initform 0)
   (workflow_mode
    :reader workflow_mode
    :initarg :workflow_mode
    :type cl:fixnum
    :initform 0)
   (requested_runtime_mode
    :reader requested_runtime_mode
    :initarg :requested_runtime_mode
    :type cl:fixnum
    :initform 0)
   (active_runtime_mode
    :reader active_runtime_mode
    :initarg :active_runtime_mode
    :type cl:fixnum
    :initform 0)
   (phase
    :reader phase
    :initarg :phase
    :type cl:fixnum
    :initform 0)
   (position_source
    :reader position_source
    :initarg :position_source
    :type cl:fixnum
    :initform 0)
   (position_state
    :reader position_state
    :initarg :position_state
    :type cl:integer
    :initform 0)
   (position_valid
    :reader position_valid
    :initarg :position_valid
    :type cl:boolean
    :initform cl:nil)
   (ins_status_received
    :reader ins_status_received
    :initarg :ins_status_received
    :type cl:boolean
    :initform cl:nil)
   (ins_solution_good
    :reader ins_solution_good
    :initarg :ins_solution_good
    :type cl:boolean
    :initform cl:nil)
   (ins_initialized
    :reader ins_initialized
    :initarg :ins_initialized
    :type cl:boolean
    :initform cl:nil)
   (ins_status
    :reader ins_status
    :initarg :ins_status
    :type cl:fixnum
    :initform 0)
   (ins_vehicle_align
    :reader ins_vehicle_align
    :initarg :ins_vehicle_align
    :type cl:fixnum
    :initform 0)
   (ins_heading_flag
    :reader ins_heading_flag
    :initarg :ins_heading_flag
    :type cl:fixnum
    :initform 0)
   (ins_position_status
    :reader ins_position_status
    :initarg :ins_position_status
    :type cl:integer
    :initform 0)
   (fast_lio_initialized
    :reader fast_lio_initialized
    :initarg :fast_lio_initialized
    :type cl:boolean
    :initform cl:nil)
   (fusion_generation_valid
    :reader fusion_generation_valid
    :initarg :fusion_generation_valid
    :type cl:boolean
    :initform cl:nil)
   (fusion_generation_sec
    :reader fusion_generation_sec
    :initarg :fusion_generation_sec
    :type cl:integer
    :initform 0)
   (fusion_generation_nsec
    :reader fusion_generation_nsec
    :initarg :fusion_generation_nsec
    :type cl:integer
    :initform 0)
   (lio_generation_valid
    :reader lio_generation_valid
    :initarg :lio_generation_valid
    :type cl:boolean
    :initform cl:nil)
   (lio_generation_sec
    :reader lio_generation_sec
    :initarg :lio_generation_sec
    :type cl:integer
    :initform 0)
   (lio_generation_nsec
    :reader lio_generation_nsec
    :initarg :lio_generation_nsec
    :type cl:integer
    :initform 0)
   (map_origin_ready
    :reader map_origin_ready
    :initarg :map_origin_ready
    :type cl:boolean
    :initform cl:nil)
   (map_origin_committed
    :reader map_origin_committed
    :initarg :map_origin_committed
    :type cl:boolean
    :initform cl:nil)
   (alignment_ready
    :reader alignment_ready
    :initarg :alignment_ready
    :type cl:boolean
    :initform cl:nil)
   (alignment_locked
    :reader alignment_locked
    :initarg :alignment_locked
    :type cl:boolean
    :initform cl:nil)
   (imu_odometry_ready
    :reader imu_odometry_ready
    :initarg :imu_odometry_ready
    :type cl:boolean
    :initform cl:nil)
   (gps_good
    :reader gps_good
    :initarg :gps_good
    :type cl:boolean
    :initform cl:nil)
   (lio_good
    :reader lio_good
    :initarg :lio_good
    :type cl:boolean
    :initform cl:nil)
   (active_map
    :reader active_map
    :initarg :active_map
    :type cl:string
    :initform "")
   (pending_map
    :reader pending_map
    :initarg :pending_map
    :type cl:string
    :initform "")
   (last_created_map
    :reader last_created_map
    :initarg :last_created_map
    :type cl:string
    :initform "")
   (active_map_uuid
    :reader active_map_uuid
    :initarg :active_map_uuid
    :type cl:string
    :initform "")
   (pending_map_uuid
    :reader pending_map_uuid
    :initarg :pending_map_uuid
    :type cl:string
    :initform "")
   (map_schema_version
    :reader map_schema_version
    :initarg :map_schema_version
    :type cl:integer
    :initform 0)
   (coordinate_frame
    :reader coordinate_frame
    :initarg :coordinate_frame
    :type cl:string
    :initform "")
   (origin_checksum
    :reader origin_checksum
    :initarg :origin_checksum
    :type cl:string
    :initform "")
   (map_valid
    :reader map_valid
    :initarg :map_valid
    :type cl:boolean
    :initform cl:nil)
   (algorithm_status_valid
    :reader algorithm_status_valid
    :initarg :algorithm_status_valid
    :type cl:boolean
    :initform cl:nil)
   (transitioning
    :reader transitioning
    :initarg :transitioning
    :type cl:boolean
    :initform cl:nil)
   (restart_in_progress
    :reader restart_in_progress
    :initarg :restart_in_progress
    :type cl:boolean
    :initform cl:nil)
   (restart_scope
    :reader restart_scope
    :initarg :restart_scope
    :type cl:fixnum
    :initform 0)
   (watchdog_position_timeout
    :reader watchdog_position_timeout
    :initarg :watchdog_position_timeout
    :type cl:boolean
    :initform cl:nil)
   (watchdog_low_frequency
    :reader watchdog_low_frequency
    :initarg :watchdog_low_frequency
    :type cl:boolean
    :initform cl:nil)
   (automatic_restart_suspended
    :reader automatic_restart_suspended
    :initarg :automatic_restart_suspended
    :type cl:boolean
    :initform cl:nil)
   (automatic_restart_count
    :reader automatic_restart_count
    :initarg :automatic_restart_count
    :type cl:integer
    :initform 0)
   (stop_required
    :reader stop_required
    :initarg :stop_required
    :type cl:boolean
    :initform cl:nil)
   (fault_code
    :reader fault_code
    :initarg :fault_code
    :type cl:fixnum
    :initform 0)
   (reason
    :reader reason
    :initarg :reason
    :type cl:string
    :initform ""))
)

(cl:defclass LocalizationStatus (<LocalizationStatus>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <LocalizationStatus>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'LocalizationStatus)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name util-msg:<LocalizationStatus> is deprecated: use util-msg:LocalizationStatus instead.")))

(cl:ensure-generic-function 'header-val :lambda-list '(m))
(cl:defmethod header-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:header-val is deprecated.  Use util-msg:header instead.")
  (header m))

(cl:ensure-generic-function 'reporter-val :lambda-list '(m))
(cl:defmethod reporter-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:reporter-val is deprecated.  Use util-msg:reporter instead.")
  (reporter m))

(cl:ensure-generic-function 'requested_mode-val :lambda-list '(m))
(cl:defmethod requested_mode-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:requested_mode-val is deprecated.  Use util-msg:requested_mode instead.")
  (requested_mode m))

(cl:ensure-generic-function 'active_mode-val :lambda-list '(m))
(cl:defmethod active_mode-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:active_mode-val is deprecated.  Use util-msg:active_mode instead.")
  (active_mode m))

(cl:ensure-generic-function 'workflow_mode-val :lambda-list '(m))
(cl:defmethod workflow_mode-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:workflow_mode-val is deprecated.  Use util-msg:workflow_mode instead.")
  (workflow_mode m))

(cl:ensure-generic-function 'requested_runtime_mode-val :lambda-list '(m))
(cl:defmethod requested_runtime_mode-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:requested_runtime_mode-val is deprecated.  Use util-msg:requested_runtime_mode instead.")
  (requested_runtime_mode m))

(cl:ensure-generic-function 'active_runtime_mode-val :lambda-list '(m))
(cl:defmethod active_runtime_mode-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:active_runtime_mode-val is deprecated.  Use util-msg:active_runtime_mode instead.")
  (active_runtime_mode m))

(cl:ensure-generic-function 'phase-val :lambda-list '(m))
(cl:defmethod phase-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:phase-val is deprecated.  Use util-msg:phase instead.")
  (phase m))

(cl:ensure-generic-function 'position_source-val :lambda-list '(m))
(cl:defmethod position_source-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:position_source-val is deprecated.  Use util-msg:position_source instead.")
  (position_source m))

(cl:ensure-generic-function 'position_state-val :lambda-list '(m))
(cl:defmethod position_state-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:position_state-val is deprecated.  Use util-msg:position_state instead.")
  (position_state m))

(cl:ensure-generic-function 'position_valid-val :lambda-list '(m))
(cl:defmethod position_valid-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:position_valid-val is deprecated.  Use util-msg:position_valid instead.")
  (position_valid m))

(cl:ensure-generic-function 'ins_status_received-val :lambda-list '(m))
(cl:defmethod ins_status_received-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:ins_status_received-val is deprecated.  Use util-msg:ins_status_received instead.")
  (ins_status_received m))

(cl:ensure-generic-function 'ins_solution_good-val :lambda-list '(m))
(cl:defmethod ins_solution_good-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:ins_solution_good-val is deprecated.  Use util-msg:ins_solution_good instead.")
  (ins_solution_good m))

(cl:ensure-generic-function 'ins_initialized-val :lambda-list '(m))
(cl:defmethod ins_initialized-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:ins_initialized-val is deprecated.  Use util-msg:ins_initialized instead.")
  (ins_initialized m))

(cl:ensure-generic-function 'ins_status-val :lambda-list '(m))
(cl:defmethod ins_status-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:ins_status-val is deprecated.  Use util-msg:ins_status instead.")
  (ins_status m))

(cl:ensure-generic-function 'ins_vehicle_align-val :lambda-list '(m))
(cl:defmethod ins_vehicle_align-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:ins_vehicle_align-val is deprecated.  Use util-msg:ins_vehicle_align instead.")
  (ins_vehicle_align m))

(cl:ensure-generic-function 'ins_heading_flag-val :lambda-list '(m))
(cl:defmethod ins_heading_flag-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:ins_heading_flag-val is deprecated.  Use util-msg:ins_heading_flag instead.")
  (ins_heading_flag m))

(cl:ensure-generic-function 'ins_position_status-val :lambda-list '(m))
(cl:defmethod ins_position_status-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:ins_position_status-val is deprecated.  Use util-msg:ins_position_status instead.")
  (ins_position_status m))

(cl:ensure-generic-function 'fast_lio_initialized-val :lambda-list '(m))
(cl:defmethod fast_lio_initialized-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:fast_lio_initialized-val is deprecated.  Use util-msg:fast_lio_initialized instead.")
  (fast_lio_initialized m))

(cl:ensure-generic-function 'fusion_generation_valid-val :lambda-list '(m))
(cl:defmethod fusion_generation_valid-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:fusion_generation_valid-val is deprecated.  Use util-msg:fusion_generation_valid instead.")
  (fusion_generation_valid m))

(cl:ensure-generic-function 'fusion_generation_sec-val :lambda-list '(m))
(cl:defmethod fusion_generation_sec-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:fusion_generation_sec-val is deprecated.  Use util-msg:fusion_generation_sec instead.")
  (fusion_generation_sec m))

(cl:ensure-generic-function 'fusion_generation_nsec-val :lambda-list '(m))
(cl:defmethod fusion_generation_nsec-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:fusion_generation_nsec-val is deprecated.  Use util-msg:fusion_generation_nsec instead.")
  (fusion_generation_nsec m))

(cl:ensure-generic-function 'lio_generation_valid-val :lambda-list '(m))
(cl:defmethod lio_generation_valid-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:lio_generation_valid-val is deprecated.  Use util-msg:lio_generation_valid instead.")
  (lio_generation_valid m))

(cl:ensure-generic-function 'lio_generation_sec-val :lambda-list '(m))
(cl:defmethod lio_generation_sec-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:lio_generation_sec-val is deprecated.  Use util-msg:lio_generation_sec instead.")
  (lio_generation_sec m))

(cl:ensure-generic-function 'lio_generation_nsec-val :lambda-list '(m))
(cl:defmethod lio_generation_nsec-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:lio_generation_nsec-val is deprecated.  Use util-msg:lio_generation_nsec instead.")
  (lio_generation_nsec m))

(cl:ensure-generic-function 'map_origin_ready-val :lambda-list '(m))
(cl:defmethod map_origin_ready-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:map_origin_ready-val is deprecated.  Use util-msg:map_origin_ready instead.")
  (map_origin_ready m))

(cl:ensure-generic-function 'map_origin_committed-val :lambda-list '(m))
(cl:defmethod map_origin_committed-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:map_origin_committed-val is deprecated.  Use util-msg:map_origin_committed instead.")
  (map_origin_committed m))

(cl:ensure-generic-function 'alignment_ready-val :lambda-list '(m))
(cl:defmethod alignment_ready-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:alignment_ready-val is deprecated.  Use util-msg:alignment_ready instead.")
  (alignment_ready m))

(cl:ensure-generic-function 'alignment_locked-val :lambda-list '(m))
(cl:defmethod alignment_locked-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:alignment_locked-val is deprecated.  Use util-msg:alignment_locked instead.")
  (alignment_locked m))

(cl:ensure-generic-function 'imu_odometry_ready-val :lambda-list '(m))
(cl:defmethod imu_odometry_ready-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:imu_odometry_ready-val is deprecated.  Use util-msg:imu_odometry_ready instead.")
  (imu_odometry_ready m))

(cl:ensure-generic-function 'gps_good-val :lambda-list '(m))
(cl:defmethod gps_good-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:gps_good-val is deprecated.  Use util-msg:gps_good instead.")
  (gps_good m))

(cl:ensure-generic-function 'lio_good-val :lambda-list '(m))
(cl:defmethod lio_good-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:lio_good-val is deprecated.  Use util-msg:lio_good instead.")
  (lio_good m))

(cl:ensure-generic-function 'active_map-val :lambda-list '(m))
(cl:defmethod active_map-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:active_map-val is deprecated.  Use util-msg:active_map instead.")
  (active_map m))

(cl:ensure-generic-function 'pending_map-val :lambda-list '(m))
(cl:defmethod pending_map-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:pending_map-val is deprecated.  Use util-msg:pending_map instead.")
  (pending_map m))

(cl:ensure-generic-function 'last_created_map-val :lambda-list '(m))
(cl:defmethod last_created_map-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:last_created_map-val is deprecated.  Use util-msg:last_created_map instead.")
  (last_created_map m))

(cl:ensure-generic-function 'active_map_uuid-val :lambda-list '(m))
(cl:defmethod active_map_uuid-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:active_map_uuid-val is deprecated.  Use util-msg:active_map_uuid instead.")
  (active_map_uuid m))

(cl:ensure-generic-function 'pending_map_uuid-val :lambda-list '(m))
(cl:defmethod pending_map_uuid-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:pending_map_uuid-val is deprecated.  Use util-msg:pending_map_uuid instead.")
  (pending_map_uuid m))

(cl:ensure-generic-function 'map_schema_version-val :lambda-list '(m))
(cl:defmethod map_schema_version-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:map_schema_version-val is deprecated.  Use util-msg:map_schema_version instead.")
  (map_schema_version m))

(cl:ensure-generic-function 'coordinate_frame-val :lambda-list '(m))
(cl:defmethod coordinate_frame-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:coordinate_frame-val is deprecated.  Use util-msg:coordinate_frame instead.")
  (coordinate_frame m))

(cl:ensure-generic-function 'origin_checksum-val :lambda-list '(m))
(cl:defmethod origin_checksum-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:origin_checksum-val is deprecated.  Use util-msg:origin_checksum instead.")
  (origin_checksum m))

(cl:ensure-generic-function 'map_valid-val :lambda-list '(m))
(cl:defmethod map_valid-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:map_valid-val is deprecated.  Use util-msg:map_valid instead.")
  (map_valid m))

(cl:ensure-generic-function 'algorithm_status_valid-val :lambda-list '(m))
(cl:defmethod algorithm_status_valid-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:algorithm_status_valid-val is deprecated.  Use util-msg:algorithm_status_valid instead.")
  (algorithm_status_valid m))

(cl:ensure-generic-function 'transitioning-val :lambda-list '(m))
(cl:defmethod transitioning-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:transitioning-val is deprecated.  Use util-msg:transitioning instead.")
  (transitioning m))

(cl:ensure-generic-function 'restart_in_progress-val :lambda-list '(m))
(cl:defmethod restart_in_progress-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:restart_in_progress-val is deprecated.  Use util-msg:restart_in_progress instead.")
  (restart_in_progress m))

(cl:ensure-generic-function 'restart_scope-val :lambda-list '(m))
(cl:defmethod restart_scope-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:restart_scope-val is deprecated.  Use util-msg:restart_scope instead.")
  (restart_scope m))

(cl:ensure-generic-function 'watchdog_position_timeout-val :lambda-list '(m))
(cl:defmethod watchdog_position_timeout-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:watchdog_position_timeout-val is deprecated.  Use util-msg:watchdog_position_timeout instead.")
  (watchdog_position_timeout m))

(cl:ensure-generic-function 'watchdog_low_frequency-val :lambda-list '(m))
(cl:defmethod watchdog_low_frequency-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:watchdog_low_frequency-val is deprecated.  Use util-msg:watchdog_low_frequency instead.")
  (watchdog_low_frequency m))

(cl:ensure-generic-function 'automatic_restart_suspended-val :lambda-list '(m))
(cl:defmethod automatic_restart_suspended-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:automatic_restart_suspended-val is deprecated.  Use util-msg:automatic_restart_suspended instead.")
  (automatic_restart_suspended m))

(cl:ensure-generic-function 'automatic_restart_count-val :lambda-list '(m))
(cl:defmethod automatic_restart_count-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:automatic_restart_count-val is deprecated.  Use util-msg:automatic_restart_count instead.")
  (automatic_restart_count m))

(cl:ensure-generic-function 'stop_required-val :lambda-list '(m))
(cl:defmethod stop_required-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:stop_required-val is deprecated.  Use util-msg:stop_required instead.")
  (stop_required m))

(cl:ensure-generic-function 'fault_code-val :lambda-list '(m))
(cl:defmethod fault_code-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:fault_code-val is deprecated.  Use util-msg:fault_code instead.")
  (fault_code m))

(cl:ensure-generic-function 'reason-val :lambda-list '(m))
(cl:defmethod reason-val ((m <LocalizationStatus>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader util-msg:reason-val is deprecated.  Use util-msg:reason instead.")
  (reason m))
(cl:defmethod roslisp-msg-protocol:symbol-codes ((msg-type (cl:eql '<LocalizationStatus>)))
    "Constants for message type '<LocalizationStatus>"
  '((:MODE_UNKNOWN . 0)
    (:MODE_IDLE . 1)
    (:MODE_MAPPING . 2)
    (:MODE_LOCALIZATION . 3)
    (:WORKFLOW_UNKNOWN . 0)
    (:WORKFLOW_MAPPING . 1)
    (:WORKFLOW_LOCALIZATION . 2)
    (:RUNTIME_UNKNOWN . 0)
    (:RUNTIME_CREATE_ORIGIN . 1)
    (:RUNTIME_TRACK_SAVED_ORIGIN . 2)
    (:PHASE_UNKNOWN . 0)
    (:PHASE_WAIT_FAST_LIO . 1)
    (:PHASE_WAIT_INS . 2)
    (:PHASE_WAIT_MAP . 3)
    (:PHASE_WAIT_GPS . 4)
    (:PHASE_WAIT_ALIGNMENT . 5)
    (:PHASE_READY_RTK . 6)
    (:PHASE_READY_LIO_LOCKED . 7)
    (:PHASE_READY_LIO_SEEDED . 8)
    (:PHASE_READY_IMU_RESTRICTED . 9)
    (:PHASE_SWITCHING_MAP . 10)
    (:PHASE_RESTARTING_FUSION . 11)
    (:PHASE_RESTARTING_ALL . 12)
    (:PHASE_RECOVERING_LIO . 13)
    (:PHASE_FAULT . 14)
    (:PHASE_WAIT_IMU_ODOMETRY . 15)
    (:PHASE_RESTART_VERIFYING . 16)
    (:PHASE_WAIT_ALGORITHM . 17)
    (:PHASE_WAIT_TRUSTED_OUTPUT . 18)
    (:SOURCE_NONE . 0)
    (:SOURCE_RTK . 1)
    (:SOURCE_LIO_LOCKED . 2)
    (:SOURCE_LIO_SEEDED . 3)
    (:SOURCE_IMU_RESTRICTED . 4)
    (:RESTART_NONE . 0)
    (:RESTART_FUSION . 1)
    (:RESTART_LIO . 2)
    (:RESTART_ALL . 3)
    (:FAULT_NONE . 0)
    (:FAULT_MAP_INVALID . 1)
    (:FAULT_RESTART_FAILED . 2)
    (:FAULT_RESTART_SUPPRESSED . 3)
    (:FAULT_POSITION_TIMEOUT . 4)
    (:FAULT_LOW_FREQUENCY . 5)
    (:FAULT_ALGORITHM . 6)
    (:FAULT_RESTART_PARTIAL . 7))
)
(cl:defmethod roslisp-msg-protocol:symbol-codes ((msg-type (cl:eql 'LocalizationStatus)))
    "Constants for message type 'LocalizationStatus"
  '((:MODE_UNKNOWN . 0)
    (:MODE_IDLE . 1)
    (:MODE_MAPPING . 2)
    (:MODE_LOCALIZATION . 3)
    (:WORKFLOW_UNKNOWN . 0)
    (:WORKFLOW_MAPPING . 1)
    (:WORKFLOW_LOCALIZATION . 2)
    (:RUNTIME_UNKNOWN . 0)
    (:RUNTIME_CREATE_ORIGIN . 1)
    (:RUNTIME_TRACK_SAVED_ORIGIN . 2)
    (:PHASE_UNKNOWN . 0)
    (:PHASE_WAIT_FAST_LIO . 1)
    (:PHASE_WAIT_INS . 2)
    (:PHASE_WAIT_MAP . 3)
    (:PHASE_WAIT_GPS . 4)
    (:PHASE_WAIT_ALIGNMENT . 5)
    (:PHASE_READY_RTK . 6)
    (:PHASE_READY_LIO_LOCKED . 7)
    (:PHASE_READY_LIO_SEEDED . 8)
    (:PHASE_READY_IMU_RESTRICTED . 9)
    (:PHASE_SWITCHING_MAP . 10)
    (:PHASE_RESTARTING_FUSION . 11)
    (:PHASE_RESTARTING_ALL . 12)
    (:PHASE_RECOVERING_LIO . 13)
    (:PHASE_FAULT . 14)
    (:PHASE_WAIT_IMU_ODOMETRY . 15)
    (:PHASE_RESTART_VERIFYING . 16)
    (:PHASE_WAIT_ALGORITHM . 17)
    (:PHASE_WAIT_TRUSTED_OUTPUT . 18)
    (:SOURCE_NONE . 0)
    (:SOURCE_RTK . 1)
    (:SOURCE_LIO_LOCKED . 2)
    (:SOURCE_LIO_SEEDED . 3)
    (:SOURCE_IMU_RESTRICTED . 4)
    (:RESTART_NONE . 0)
    (:RESTART_FUSION . 1)
    (:RESTART_LIO . 2)
    (:RESTART_ALL . 3)
    (:FAULT_NONE . 0)
    (:FAULT_MAP_INVALID . 1)
    (:FAULT_RESTART_FAILED . 2)
    (:FAULT_RESTART_SUPPRESSED . 3)
    (:FAULT_POSITION_TIMEOUT . 4)
    (:FAULT_LOW_FREQUENCY . 5)
    (:FAULT_ALGORITHM . 6)
    (:FAULT_RESTART_PARTIAL . 7))
)
(cl:defmethod roslisp-msg-protocol:serialize ((msg <LocalizationStatus>) ostream)
  "Serializes a message object of type '<LocalizationStatus>"
  (roslisp-msg-protocol:serialize (cl:slot-value msg 'header) ostream)
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'reporter))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'reporter))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'requested_mode)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'active_mode)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'workflow_mode)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'requested_runtime_mode)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'active_runtime_mode)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'phase)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'position_source)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'position_state)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'position_state)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'position_state)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'position_state)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'position_valid) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'ins_status_received) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'ins_solution_good) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'ins_initialized) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'ins_status)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'ins_vehicle_align)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'ins_heading_flag)) ostream)
  (cl:let* ((signed (cl:slot-value msg 'ins_position_status)) (unsigned (cl:if (cl:< signed 0) (cl:+ signed 4294967296) signed)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) unsigned) ostream)
    )
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'fast_lio_initialized) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'fusion_generation_valid) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'fusion_generation_sec)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'fusion_generation_sec)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'fusion_generation_sec)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'fusion_generation_sec)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'fusion_generation_nsec)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'fusion_generation_nsec)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'fusion_generation_nsec)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'fusion_generation_nsec)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'lio_generation_valid) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'lio_generation_sec)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'lio_generation_sec)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'lio_generation_sec)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'lio_generation_sec)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'lio_generation_nsec)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'lio_generation_nsec)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'lio_generation_nsec)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'lio_generation_nsec)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'map_origin_ready) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'map_origin_committed) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'alignment_ready) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'alignment_locked) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'imu_odometry_ready) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'gps_good) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'lio_good) 1 0)) ostream)
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'active_map))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'active_map))
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'pending_map))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'pending_map))
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'last_created_map))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'last_created_map))
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'active_map_uuid))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'active_map_uuid))
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'pending_map_uuid))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'pending_map_uuid))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'map_schema_version)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'map_schema_version)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'map_schema_version)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'map_schema_version)) ostream)
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'coordinate_frame))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'coordinate_frame))
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'origin_checksum))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'origin_checksum))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'map_valid) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'algorithm_status_valid) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'transitioning) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'restart_in_progress) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'restart_scope)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'watchdog_position_timeout) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'watchdog_low_frequency) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'automatic_restart_suspended) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'automatic_restart_count)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'automatic_restart_count)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'automatic_restart_count)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'automatic_restart_count)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'stop_required) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'fault_code)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'fault_code)) ostream)
  (cl:let ((__ros_str_len (cl:length (cl:slot-value msg 'reason))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_str_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_str_len) ostream))
  (cl:map cl:nil #'(cl:lambda (c) (cl:write-byte (cl:char-code c) ostream)) (cl:slot-value msg 'reason))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <LocalizationStatus>) istream)
  "Deserializes a message object of type '<LocalizationStatus>"
  (roslisp-msg-protocol:deserialize (cl:slot-value msg 'header) istream)
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'reporter) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'reporter) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'requested_mode)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'active_mode)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'workflow_mode)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'requested_runtime_mode)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'active_runtime_mode)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'phase)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'position_source)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'position_state)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'position_state)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'position_state)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'position_state)) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'position_valid) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:slot-value msg 'ins_status_received) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:slot-value msg 'ins_solution_good) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:slot-value msg 'ins_initialized) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'ins_status)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'ins_vehicle_align)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'ins_heading_flag)) (cl:read-byte istream))
    (cl:let ((unsigned 0))
      (cl:setf (cl:ldb (cl:byte 8 0) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) unsigned) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'ins_position_status) (cl:if (cl:< unsigned 2147483648) unsigned (cl:- unsigned 4294967296))))
    (cl:setf (cl:slot-value msg 'fast_lio_initialized) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:slot-value msg 'fusion_generation_valid) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'fusion_generation_sec)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'fusion_generation_sec)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'fusion_generation_sec)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'fusion_generation_sec)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'fusion_generation_nsec)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'fusion_generation_nsec)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'fusion_generation_nsec)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'fusion_generation_nsec)) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'lio_generation_valid) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'lio_generation_sec)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'lio_generation_sec)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'lio_generation_sec)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'lio_generation_sec)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'lio_generation_nsec)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'lio_generation_nsec)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'lio_generation_nsec)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'lio_generation_nsec)) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'map_origin_ready) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:slot-value msg 'map_origin_committed) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:slot-value msg 'alignment_ready) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:slot-value msg 'alignment_locked) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:slot-value msg 'imu_odometry_ready) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:slot-value msg 'gps_good) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:slot-value msg 'lio_good) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'active_map) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'active_map) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'pending_map) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'pending_map) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'last_created_map) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'last_created_map) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'active_map_uuid) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'active_map_uuid) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'pending_map_uuid) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'pending_map_uuid) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'map_schema_version)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'map_schema_version)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'map_schema_version)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'map_schema_version)) (cl:read-byte istream))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'coordinate_frame) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'coordinate_frame) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'origin_checksum) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'origin_checksum) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
    (cl:setf (cl:slot-value msg 'map_valid) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:slot-value msg 'algorithm_status_valid) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:slot-value msg 'transitioning) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:slot-value msg 'restart_in_progress) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'restart_scope)) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'watchdog_position_timeout) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:slot-value msg 'watchdog_low_frequency) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:slot-value msg 'automatic_restart_suspended) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'automatic_restart_count)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'automatic_restart_count)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'automatic_restart_count)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'automatic_restart_count)) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'stop_required) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'fault_code)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'fault_code)) (cl:read-byte istream))
    (cl:let ((__ros_str_len 0))
      (cl:setf (cl:ldb (cl:byte 8 0) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) __ros_str_len) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'reason) (cl:make-string __ros_str_len))
      (cl:dotimes (__ros_str_idx __ros_str_len msg)
        (cl:setf (cl:char (cl:slot-value msg 'reason) __ros_str_idx) (cl:code-char (cl:read-byte istream)))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<LocalizationStatus>)))
  "Returns string type for a message object of type '<LocalizationStatus>"
  "util/LocalizationStatus")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'LocalizationStatus)))
  "Returns string type for a message object of type 'LocalizationStatus"
  "util/LocalizationStatus")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<LocalizationStatus>)))
  "Returns md5sum for a message object of type '<LocalizationStatus>"
  "66d83c808f19f476c7a0fb306057009c")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'LocalizationStatus)))
  "Returns md5sum for a message object of type 'LocalizationStatus"
  "66d83c808f19f476c7a0fb306057009c")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<LocalizationStatus>)))
  "Returns full string definition for message of type '<LocalizationStatus>"
  (cl:format cl:nil "# Unified localization lifecycle status.  fusion_4dof publishes the algorithm~%# view on /Mower/localization_algorithm_status; android_manager_4dof publishes~%# the authoritative aggregate view on /Mower/localization_status.  Both topics~%# are latched.~%Header header~%~%uint8 MODE_UNKNOWN=0~%uint8 MODE_IDLE=1~%uint8 MODE_MAPPING=2~%uint8 MODE_LOCALIZATION=3~%~%# The user-visible workflow and the algorithm process runtime are deliberately~%# separate. During a boundary-recording workflow, a recovery may restart fusion~%# in TRACK_SAVED_ORIGIN without ending the MAPPING workflow.~%uint8 WORKFLOW_UNKNOWN=0~%uint8 WORKFLOW_MAPPING=1~%uint8 WORKFLOW_LOCALIZATION=2~%~%uint8 RUNTIME_UNKNOWN=0~%uint8 RUNTIME_CREATE_ORIGIN=1~%uint8 RUNTIME_TRACK_SAVED_ORIGIN=2~%~%uint8 PHASE_UNKNOWN=0~%uint8 PHASE_WAIT_FAST_LIO=1~%uint8 PHASE_WAIT_INS=2~%uint8 PHASE_WAIT_MAP=3~%uint8 PHASE_WAIT_GPS=4~%uint8 PHASE_WAIT_ALIGNMENT=5~%uint8 PHASE_READY_RTK=6~%uint8 PHASE_READY_LIO_LOCKED=7~%uint8 PHASE_READY_LIO_SEEDED=8~%uint8 PHASE_READY_IMU_RESTRICTED=9~%uint8 PHASE_SWITCHING_MAP=10~%uint8 PHASE_RESTARTING_FUSION=11~%uint8 PHASE_RESTARTING_ALL=12~%uint8 PHASE_RECOVERING_LIO=13~%uint8 PHASE_FAULT=14~%uint8 PHASE_WAIT_IMU_ODOMETRY=15~%uint8 PHASE_RESTART_VERIFYING=16~%uint8 PHASE_WAIT_ALGORITHM=17~%uint8 PHASE_WAIT_TRUSTED_OUTPUT=18~%~%uint8 SOURCE_NONE=0~%uint8 SOURCE_RTK=1~%uint8 SOURCE_LIO_LOCKED=2~%uint8 SOURCE_LIO_SEEDED=3~%uint8 SOURCE_IMU_RESTRICTED=4~%~%uint8 RESTART_NONE=0~%uint8 RESTART_FUSION=1~%uint8 RESTART_LIO=2~%uint8 RESTART_ALL=3~%~%uint16 FAULT_NONE=0~%uint16 FAULT_MAP_INVALID=1~%uint16 FAULT_RESTART_FAILED=2~%uint16 FAULT_RESTART_SUPPRESSED=3~%uint16 FAULT_POSITION_TIMEOUT=4~%uint16 FAULT_LOW_FREQUENCY=5~%uint16 FAULT_ALGORITHM=6~%uint16 FAULT_RESTART_PARTIAL=7~%~%# Publisher identity and legacy process-mode fields. requested_mode/active_mode~%# are retained for compatibility; use workflow_mode plus active_runtime_mode for~%# unambiguous new integrations.~%string reporter~%uint8 requested_mode~%uint8 active_mode~%uint8 workflow_mode~%uint8 requested_runtime_mode~%uint8 active_runtime_mode~%uint8 phase~%~%# Current production position semantics.~%uint8 position_source~%uint32 position_state~%bool position_valid~%~%# INS readiness. Both ins_solution_good and the compatibility field~%# ins_initialized mean that a fresh receiver status has INS_Status==3.~%# ins_position_status remains the 1ant publisher's motion-derived heading~%# consistency diagnostic; it is not a localization startup gate.~%bool ins_status_received~%bool ins_solution_good~%bool ins_initialized~%uint8 ins_status~%uint8 ins_vehicle_align~%uint8 ins_heading_flag~%int32 ins_position_status~%~%# FAST-LIO and alignment truth owned by fusion_4dof.~%bool fast_lio_initialized~%# Process-generation identity is wall-clock based and used only for equality,~%# never for message-age calculations. It lets the manager distinguish a new~%# fusion process from a verification failure in the restart wrapper.~%bool fusion_generation_valid~%uint32 fusion_generation_sec~%uint32 fusion_generation_nsec~%bool lio_generation_valid~%# Process identity may become valid before static calibration. Consumers must~%# use fast_lio_initialized for pose readiness.~%uint32 lio_generation_sec~%uint32 lio_generation_nsec~%bool map_origin_ready~%bool map_origin_committed~%bool alignment_ready~%bool alignment_locked~%bool imu_odometry_ready~%bool gps_good~%bool lio_good~%~%# fusion_4dof owns runtime map identity; android_manager_4dof adds requested~%# lifecycle state, last-created identity, restart and watchdog ownership.~%string active_map~%string pending_map~%string last_created_map~%string active_map_uuid~%string pending_map_uuid~%uint32 map_schema_version~%string coordinate_frame~%string origin_checksum~%bool map_valid~%bool algorithm_status_valid~%bool transitioning~%bool restart_in_progress~%uint8 restart_scope~%bool watchdog_position_timeout~%bool watchdog_low_frequency~%bool automatic_restart_suspended~%uint32 automatic_restart_count~%bool stop_required~%uint16 fault_code~%string reason~%~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'LocalizationStatus)))
  "Returns full string definition for message of type 'LocalizationStatus"
  (cl:format cl:nil "# Unified localization lifecycle status.  fusion_4dof publishes the algorithm~%# view on /Mower/localization_algorithm_status; android_manager_4dof publishes~%# the authoritative aggregate view on /Mower/localization_status.  Both topics~%# are latched.~%Header header~%~%uint8 MODE_UNKNOWN=0~%uint8 MODE_IDLE=1~%uint8 MODE_MAPPING=2~%uint8 MODE_LOCALIZATION=3~%~%# The user-visible workflow and the algorithm process runtime are deliberately~%# separate. During a boundary-recording workflow, a recovery may restart fusion~%# in TRACK_SAVED_ORIGIN without ending the MAPPING workflow.~%uint8 WORKFLOW_UNKNOWN=0~%uint8 WORKFLOW_MAPPING=1~%uint8 WORKFLOW_LOCALIZATION=2~%~%uint8 RUNTIME_UNKNOWN=0~%uint8 RUNTIME_CREATE_ORIGIN=1~%uint8 RUNTIME_TRACK_SAVED_ORIGIN=2~%~%uint8 PHASE_UNKNOWN=0~%uint8 PHASE_WAIT_FAST_LIO=1~%uint8 PHASE_WAIT_INS=2~%uint8 PHASE_WAIT_MAP=3~%uint8 PHASE_WAIT_GPS=4~%uint8 PHASE_WAIT_ALIGNMENT=5~%uint8 PHASE_READY_RTK=6~%uint8 PHASE_READY_LIO_LOCKED=7~%uint8 PHASE_READY_LIO_SEEDED=8~%uint8 PHASE_READY_IMU_RESTRICTED=9~%uint8 PHASE_SWITCHING_MAP=10~%uint8 PHASE_RESTARTING_FUSION=11~%uint8 PHASE_RESTARTING_ALL=12~%uint8 PHASE_RECOVERING_LIO=13~%uint8 PHASE_FAULT=14~%uint8 PHASE_WAIT_IMU_ODOMETRY=15~%uint8 PHASE_RESTART_VERIFYING=16~%uint8 PHASE_WAIT_ALGORITHM=17~%uint8 PHASE_WAIT_TRUSTED_OUTPUT=18~%~%uint8 SOURCE_NONE=0~%uint8 SOURCE_RTK=1~%uint8 SOURCE_LIO_LOCKED=2~%uint8 SOURCE_LIO_SEEDED=3~%uint8 SOURCE_IMU_RESTRICTED=4~%~%uint8 RESTART_NONE=0~%uint8 RESTART_FUSION=1~%uint8 RESTART_LIO=2~%uint8 RESTART_ALL=3~%~%uint16 FAULT_NONE=0~%uint16 FAULT_MAP_INVALID=1~%uint16 FAULT_RESTART_FAILED=2~%uint16 FAULT_RESTART_SUPPRESSED=3~%uint16 FAULT_POSITION_TIMEOUT=4~%uint16 FAULT_LOW_FREQUENCY=5~%uint16 FAULT_ALGORITHM=6~%uint16 FAULT_RESTART_PARTIAL=7~%~%# Publisher identity and legacy process-mode fields. requested_mode/active_mode~%# are retained for compatibility; use workflow_mode plus active_runtime_mode for~%# unambiguous new integrations.~%string reporter~%uint8 requested_mode~%uint8 active_mode~%uint8 workflow_mode~%uint8 requested_runtime_mode~%uint8 active_runtime_mode~%uint8 phase~%~%# Current production position semantics.~%uint8 position_source~%uint32 position_state~%bool position_valid~%~%# INS readiness. Both ins_solution_good and the compatibility field~%# ins_initialized mean that a fresh receiver status has INS_Status==3.~%# ins_position_status remains the 1ant publisher's motion-derived heading~%# consistency diagnostic; it is not a localization startup gate.~%bool ins_status_received~%bool ins_solution_good~%bool ins_initialized~%uint8 ins_status~%uint8 ins_vehicle_align~%uint8 ins_heading_flag~%int32 ins_position_status~%~%# FAST-LIO and alignment truth owned by fusion_4dof.~%bool fast_lio_initialized~%# Process-generation identity is wall-clock based and used only for equality,~%# never for message-age calculations. It lets the manager distinguish a new~%# fusion process from a verification failure in the restart wrapper.~%bool fusion_generation_valid~%uint32 fusion_generation_sec~%uint32 fusion_generation_nsec~%bool lio_generation_valid~%# Process identity may become valid before static calibration. Consumers must~%# use fast_lio_initialized for pose readiness.~%uint32 lio_generation_sec~%uint32 lio_generation_nsec~%bool map_origin_ready~%bool map_origin_committed~%bool alignment_ready~%bool alignment_locked~%bool imu_odometry_ready~%bool gps_good~%bool lio_good~%~%# fusion_4dof owns runtime map identity; android_manager_4dof adds requested~%# lifecycle state, last-created identity, restart and watchdog ownership.~%string active_map~%string pending_map~%string last_created_map~%string active_map_uuid~%string pending_map_uuid~%uint32 map_schema_version~%string coordinate_frame~%string origin_checksum~%bool map_valid~%bool algorithm_status_valid~%bool transitioning~%bool restart_in_progress~%uint8 restart_scope~%bool watchdog_position_timeout~%bool watchdog_low_frequency~%bool automatic_restart_suspended~%uint32 automatic_restart_count~%bool stop_required~%uint16 fault_code~%string reason~%~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%string frame_id~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <LocalizationStatus>))
  (cl:+ 0
     (roslisp-msg-protocol:serialization-length (cl:slot-value msg 'header))
     4 (cl:length (cl:slot-value msg 'reporter))
     1
     1
     1
     1
     1
     1
     1
     4
     1
     1
     1
     1
     1
     1
     1
     4
     1
     1
     4
     4
     1
     4
     4
     1
     1
     1
     1
     1
     1
     1
     4 (cl:length (cl:slot-value msg 'active_map))
     4 (cl:length (cl:slot-value msg 'pending_map))
     4 (cl:length (cl:slot-value msg 'last_created_map))
     4 (cl:length (cl:slot-value msg 'active_map_uuid))
     4 (cl:length (cl:slot-value msg 'pending_map_uuid))
     4
     4 (cl:length (cl:slot-value msg 'coordinate_frame))
     4 (cl:length (cl:slot-value msg 'origin_checksum))
     1
     1
     1
     1
     1
     1
     1
     1
     4
     1
     2
     4 (cl:length (cl:slot-value msg 'reason))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <LocalizationStatus>))
  "Converts a ROS message object to a list"
  (cl:list 'LocalizationStatus
    (cl:cons ':header (header msg))
    (cl:cons ':reporter (reporter msg))
    (cl:cons ':requested_mode (requested_mode msg))
    (cl:cons ':active_mode (active_mode msg))
    (cl:cons ':workflow_mode (workflow_mode msg))
    (cl:cons ':requested_runtime_mode (requested_runtime_mode msg))
    (cl:cons ':active_runtime_mode (active_runtime_mode msg))
    (cl:cons ':phase (phase msg))
    (cl:cons ':position_source (position_source msg))
    (cl:cons ':position_state (position_state msg))
    (cl:cons ':position_valid (position_valid msg))
    (cl:cons ':ins_status_received (ins_status_received msg))
    (cl:cons ':ins_solution_good (ins_solution_good msg))
    (cl:cons ':ins_initialized (ins_initialized msg))
    (cl:cons ':ins_status (ins_status msg))
    (cl:cons ':ins_vehicle_align (ins_vehicle_align msg))
    (cl:cons ':ins_heading_flag (ins_heading_flag msg))
    (cl:cons ':ins_position_status (ins_position_status msg))
    (cl:cons ':fast_lio_initialized (fast_lio_initialized msg))
    (cl:cons ':fusion_generation_valid (fusion_generation_valid msg))
    (cl:cons ':fusion_generation_sec (fusion_generation_sec msg))
    (cl:cons ':fusion_generation_nsec (fusion_generation_nsec msg))
    (cl:cons ':lio_generation_valid (lio_generation_valid msg))
    (cl:cons ':lio_generation_sec (lio_generation_sec msg))
    (cl:cons ':lio_generation_nsec (lio_generation_nsec msg))
    (cl:cons ':map_origin_ready (map_origin_ready msg))
    (cl:cons ':map_origin_committed (map_origin_committed msg))
    (cl:cons ':alignment_ready (alignment_ready msg))
    (cl:cons ':alignment_locked (alignment_locked msg))
    (cl:cons ':imu_odometry_ready (imu_odometry_ready msg))
    (cl:cons ':gps_good (gps_good msg))
    (cl:cons ':lio_good (lio_good msg))
    (cl:cons ':active_map (active_map msg))
    (cl:cons ':pending_map (pending_map msg))
    (cl:cons ':last_created_map (last_created_map msg))
    (cl:cons ':active_map_uuid (active_map_uuid msg))
    (cl:cons ':pending_map_uuid (pending_map_uuid msg))
    (cl:cons ':map_schema_version (map_schema_version msg))
    (cl:cons ':coordinate_frame (coordinate_frame msg))
    (cl:cons ':origin_checksum (origin_checksum msg))
    (cl:cons ':map_valid (map_valid msg))
    (cl:cons ':algorithm_status_valid (algorithm_status_valid msg))
    (cl:cons ':transitioning (transitioning msg))
    (cl:cons ':restart_in_progress (restart_in_progress msg))
    (cl:cons ':restart_scope (restart_scope msg))
    (cl:cons ':watchdog_position_timeout (watchdog_position_timeout msg))
    (cl:cons ':watchdog_low_frequency (watchdog_low_frequency msg))
    (cl:cons ':automatic_restart_suspended (automatic_restart_suspended msg))
    (cl:cons ':automatic_restart_count (automatic_restart_count msg))
    (cl:cons ':stop_required (stop_required msg))
    (cl:cons ':fault_code (fault_code msg))
    (cl:cons ':reason (reason msg))
))
