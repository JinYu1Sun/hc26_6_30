# Unified localization lifecycle status.  fusion_4dof publishes the algorithm
# view on /Mower/localization_algorithm_status; android_manager_4dof publishes
# the authoritative aggregate view on /Mower/localization_status.  Both topics
# are latched.
Header header

uint8 MODE_UNKNOWN=0
uint8 MODE_IDLE=1
uint8 MODE_MAPPING=2
uint8 MODE_LOCALIZATION=3

# The user-visible workflow and the algorithm process runtime are deliberately
# separate. During a boundary-recording workflow, a recovery may restart fusion
# in TRACK_SAVED_ORIGIN without ending the MAPPING workflow.
uint8 WORKFLOW_UNKNOWN=0
uint8 WORKFLOW_MAPPING=1
uint8 WORKFLOW_LOCALIZATION=2

uint8 RUNTIME_UNKNOWN=0
uint8 RUNTIME_CREATE_ORIGIN=1
uint8 RUNTIME_TRACK_SAVED_ORIGIN=2

uint8 PHASE_UNKNOWN=0
uint8 PHASE_WAIT_FAST_LIO=1
uint8 PHASE_WAIT_INS=2
uint8 PHASE_WAIT_MAP=3
uint8 PHASE_WAIT_GPS=4
uint8 PHASE_WAIT_ALIGNMENT=5
uint8 PHASE_READY_RTK=6
uint8 PHASE_READY_LIO_LOCKED=7
uint8 PHASE_READY_LIO_SEEDED=8
uint8 PHASE_READY_IMU_RESTRICTED=9
uint8 PHASE_SWITCHING_MAP=10
uint8 PHASE_RESTARTING_FUSION=11
uint8 PHASE_RESTARTING_ALL=12
uint8 PHASE_RECOVERING_LIO=13
uint8 PHASE_FAULT=14
uint8 PHASE_WAIT_IMU_ODOMETRY=15
uint8 PHASE_RESTART_VERIFYING=16
uint8 PHASE_WAIT_ALGORITHM=17
uint8 PHASE_WAIT_TRUSTED_OUTPUT=18

uint8 SOURCE_NONE=0
uint8 SOURCE_RTK=1
uint8 SOURCE_LIO_LOCKED=2
uint8 SOURCE_LIO_SEEDED=3
uint8 SOURCE_IMU_RESTRICTED=4

uint8 RESTART_NONE=0
uint8 RESTART_FUSION=1
uint8 RESTART_LIO=2
uint8 RESTART_ALL=3

uint16 FAULT_NONE=0
uint16 FAULT_MAP_INVALID=1
uint16 FAULT_RESTART_FAILED=2
uint16 FAULT_RESTART_SUPPRESSED=3
uint16 FAULT_POSITION_TIMEOUT=4
uint16 FAULT_LOW_FREQUENCY=5
uint16 FAULT_ALGORITHM=6
uint16 FAULT_RESTART_PARTIAL=7

# Publisher identity and legacy process-mode fields. requested_mode/active_mode
# are retained for compatibility; use workflow_mode plus active_runtime_mode for
# unambiguous new integrations.
string reporter
uint8 requested_mode
uint8 active_mode
uint8 workflow_mode
uint8 requested_runtime_mode
uint8 active_runtime_mode
uint8 phase

# Current production position semantics.
uint8 position_source
uint32 position_state
bool position_valid

# INS readiness. Both ins_solution_good and the compatibility field
# ins_initialized mean that a fresh receiver status has INS_Status==3.
# ins_position_status remains the 1ant publisher's motion-derived heading
# consistency diagnostic; it is not a localization startup gate.
bool ins_status_received
bool ins_solution_good
bool ins_initialized
uint8 ins_status
uint8 ins_vehicle_align
uint8 ins_heading_flag
int32 ins_position_status

# FAST-LIO and alignment truth owned by fusion_4dof.
bool fast_lio_initialized
# Process-generation identity is wall-clock based and used only for equality,
# never for message-age calculations. It lets the manager distinguish a new
# fusion process from a verification failure in the restart wrapper.
bool fusion_generation_valid
uint32 fusion_generation_sec
uint32 fusion_generation_nsec
bool lio_generation_valid
# Process identity may become valid before static calibration. Consumers must
# use fast_lio_initialized for pose readiness.
uint32 lio_generation_sec
uint32 lio_generation_nsec
bool map_origin_ready
bool map_origin_committed
bool alignment_ready
bool alignment_locked
bool imu_odometry_ready
bool gps_good
bool lio_good

# fusion_4dof owns runtime map identity; android_manager_4dof adds requested
# lifecycle state, last-created identity, restart and watchdog ownership.
string active_map
string pending_map
string last_created_map
string active_map_uuid
string pending_map_uuid
uint32 map_schema_version
string coordinate_frame
string origin_checksum
bool map_valid
bool algorithm_status_valid
bool transitioning
bool restart_in_progress
uint8 restart_scope
bool watchdog_position_timeout
bool watchdog_low_frequency
bool automatic_restart_suspended
uint32 automatic_restart_count
bool stop_required
uint16 fault_code
string reason
