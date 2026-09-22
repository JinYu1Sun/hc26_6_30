// Auto-generated. Do not edit!

// (in-package util.msg)


"use strict";

const _serializer = _ros_msg_utils.Serialize;
const _arraySerializer = _serializer.Array;
const _deserializer = _ros_msg_utils.Deserialize;
const _arrayDeserializer = _deserializer.Array;
const _finder = _ros_msg_utils.Find;
const _getByteLength = _ros_msg_utils.getByteLength;
let std_msgs = _finder('std_msgs');

//-----------------------------------------------------------

class LocalizationStatus {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.header = null;
      this.reporter = null;
      this.requested_mode = null;
      this.active_mode = null;
      this.workflow_mode = null;
      this.requested_runtime_mode = null;
      this.active_runtime_mode = null;
      this.phase = null;
      this.position_source = null;
      this.position_state = null;
      this.position_valid = null;
      this.ins_status_received = null;
      this.ins_solution_good = null;
      this.ins_initialized = null;
      this.ins_status = null;
      this.ins_vehicle_align = null;
      this.ins_heading_flag = null;
      this.ins_position_status = null;
      this.fast_lio_initialized = null;
      this.fusion_generation_valid = null;
      this.fusion_generation_sec = null;
      this.fusion_generation_nsec = null;
      this.lio_generation_valid = null;
      this.lio_generation_sec = null;
      this.lio_generation_nsec = null;
      this.map_origin_ready = null;
      this.map_origin_committed = null;
      this.alignment_ready = null;
      this.alignment_locked = null;
      this.imu_odometry_ready = null;
      this.gps_good = null;
      this.lio_good = null;
      this.active_map = null;
      this.pending_map = null;
      this.last_created_map = null;
      this.active_map_uuid = null;
      this.pending_map_uuid = null;
      this.map_schema_version = null;
      this.coordinate_frame = null;
      this.origin_checksum = null;
      this.map_valid = null;
      this.algorithm_status_valid = null;
      this.transitioning = null;
      this.restart_in_progress = null;
      this.restart_scope = null;
      this.watchdog_position_timeout = null;
      this.watchdog_low_frequency = null;
      this.automatic_restart_suspended = null;
      this.automatic_restart_count = null;
      this.stop_required = null;
      this.fault_code = null;
      this.reason = null;
    }
    else {
      if (initObj.hasOwnProperty('header')) {
        this.header = initObj.header
      }
      else {
        this.header = new std_msgs.msg.Header();
      }
      if (initObj.hasOwnProperty('reporter')) {
        this.reporter = initObj.reporter
      }
      else {
        this.reporter = '';
      }
      if (initObj.hasOwnProperty('requested_mode')) {
        this.requested_mode = initObj.requested_mode
      }
      else {
        this.requested_mode = 0;
      }
      if (initObj.hasOwnProperty('active_mode')) {
        this.active_mode = initObj.active_mode
      }
      else {
        this.active_mode = 0;
      }
      if (initObj.hasOwnProperty('workflow_mode')) {
        this.workflow_mode = initObj.workflow_mode
      }
      else {
        this.workflow_mode = 0;
      }
      if (initObj.hasOwnProperty('requested_runtime_mode')) {
        this.requested_runtime_mode = initObj.requested_runtime_mode
      }
      else {
        this.requested_runtime_mode = 0;
      }
      if (initObj.hasOwnProperty('active_runtime_mode')) {
        this.active_runtime_mode = initObj.active_runtime_mode
      }
      else {
        this.active_runtime_mode = 0;
      }
      if (initObj.hasOwnProperty('phase')) {
        this.phase = initObj.phase
      }
      else {
        this.phase = 0;
      }
      if (initObj.hasOwnProperty('position_source')) {
        this.position_source = initObj.position_source
      }
      else {
        this.position_source = 0;
      }
      if (initObj.hasOwnProperty('position_state')) {
        this.position_state = initObj.position_state
      }
      else {
        this.position_state = 0;
      }
      if (initObj.hasOwnProperty('position_valid')) {
        this.position_valid = initObj.position_valid
      }
      else {
        this.position_valid = false;
      }
      if (initObj.hasOwnProperty('ins_status_received')) {
        this.ins_status_received = initObj.ins_status_received
      }
      else {
        this.ins_status_received = false;
      }
      if (initObj.hasOwnProperty('ins_solution_good')) {
        this.ins_solution_good = initObj.ins_solution_good
      }
      else {
        this.ins_solution_good = false;
      }
      if (initObj.hasOwnProperty('ins_initialized')) {
        this.ins_initialized = initObj.ins_initialized
      }
      else {
        this.ins_initialized = false;
      }
      if (initObj.hasOwnProperty('ins_status')) {
        this.ins_status = initObj.ins_status
      }
      else {
        this.ins_status = 0;
      }
      if (initObj.hasOwnProperty('ins_vehicle_align')) {
        this.ins_vehicle_align = initObj.ins_vehicle_align
      }
      else {
        this.ins_vehicle_align = 0;
      }
      if (initObj.hasOwnProperty('ins_heading_flag')) {
        this.ins_heading_flag = initObj.ins_heading_flag
      }
      else {
        this.ins_heading_flag = 0;
      }
      if (initObj.hasOwnProperty('ins_position_status')) {
        this.ins_position_status = initObj.ins_position_status
      }
      else {
        this.ins_position_status = 0;
      }
      if (initObj.hasOwnProperty('fast_lio_initialized')) {
        this.fast_lio_initialized = initObj.fast_lio_initialized
      }
      else {
        this.fast_lio_initialized = false;
      }
      if (initObj.hasOwnProperty('fusion_generation_valid')) {
        this.fusion_generation_valid = initObj.fusion_generation_valid
      }
      else {
        this.fusion_generation_valid = false;
      }
      if (initObj.hasOwnProperty('fusion_generation_sec')) {
        this.fusion_generation_sec = initObj.fusion_generation_sec
      }
      else {
        this.fusion_generation_sec = 0;
      }
      if (initObj.hasOwnProperty('fusion_generation_nsec')) {
        this.fusion_generation_nsec = initObj.fusion_generation_nsec
      }
      else {
        this.fusion_generation_nsec = 0;
      }
      if (initObj.hasOwnProperty('lio_generation_valid')) {
        this.lio_generation_valid = initObj.lio_generation_valid
      }
      else {
        this.lio_generation_valid = false;
      }
      if (initObj.hasOwnProperty('lio_generation_sec')) {
        this.lio_generation_sec = initObj.lio_generation_sec
      }
      else {
        this.lio_generation_sec = 0;
      }
      if (initObj.hasOwnProperty('lio_generation_nsec')) {
        this.lio_generation_nsec = initObj.lio_generation_nsec
      }
      else {
        this.lio_generation_nsec = 0;
      }
      if (initObj.hasOwnProperty('map_origin_ready')) {
        this.map_origin_ready = initObj.map_origin_ready
      }
      else {
        this.map_origin_ready = false;
      }
      if (initObj.hasOwnProperty('map_origin_committed')) {
        this.map_origin_committed = initObj.map_origin_committed
      }
      else {
        this.map_origin_committed = false;
      }
      if (initObj.hasOwnProperty('alignment_ready')) {
        this.alignment_ready = initObj.alignment_ready
      }
      else {
        this.alignment_ready = false;
      }
      if (initObj.hasOwnProperty('alignment_locked')) {
        this.alignment_locked = initObj.alignment_locked
      }
      else {
        this.alignment_locked = false;
      }
      if (initObj.hasOwnProperty('imu_odometry_ready')) {
        this.imu_odometry_ready = initObj.imu_odometry_ready
      }
      else {
        this.imu_odometry_ready = false;
      }
      if (initObj.hasOwnProperty('gps_good')) {
        this.gps_good = initObj.gps_good
      }
      else {
        this.gps_good = false;
      }
      if (initObj.hasOwnProperty('lio_good')) {
        this.lio_good = initObj.lio_good
      }
      else {
        this.lio_good = false;
      }
      if (initObj.hasOwnProperty('active_map')) {
        this.active_map = initObj.active_map
      }
      else {
        this.active_map = '';
      }
      if (initObj.hasOwnProperty('pending_map')) {
        this.pending_map = initObj.pending_map
      }
      else {
        this.pending_map = '';
      }
      if (initObj.hasOwnProperty('last_created_map')) {
        this.last_created_map = initObj.last_created_map
      }
      else {
        this.last_created_map = '';
      }
      if (initObj.hasOwnProperty('active_map_uuid')) {
        this.active_map_uuid = initObj.active_map_uuid
      }
      else {
        this.active_map_uuid = '';
      }
      if (initObj.hasOwnProperty('pending_map_uuid')) {
        this.pending_map_uuid = initObj.pending_map_uuid
      }
      else {
        this.pending_map_uuid = '';
      }
      if (initObj.hasOwnProperty('map_schema_version')) {
        this.map_schema_version = initObj.map_schema_version
      }
      else {
        this.map_schema_version = 0;
      }
      if (initObj.hasOwnProperty('coordinate_frame')) {
        this.coordinate_frame = initObj.coordinate_frame
      }
      else {
        this.coordinate_frame = '';
      }
      if (initObj.hasOwnProperty('origin_checksum')) {
        this.origin_checksum = initObj.origin_checksum
      }
      else {
        this.origin_checksum = '';
      }
      if (initObj.hasOwnProperty('map_valid')) {
        this.map_valid = initObj.map_valid
      }
      else {
        this.map_valid = false;
      }
      if (initObj.hasOwnProperty('algorithm_status_valid')) {
        this.algorithm_status_valid = initObj.algorithm_status_valid
      }
      else {
        this.algorithm_status_valid = false;
      }
      if (initObj.hasOwnProperty('transitioning')) {
        this.transitioning = initObj.transitioning
      }
      else {
        this.transitioning = false;
      }
      if (initObj.hasOwnProperty('restart_in_progress')) {
        this.restart_in_progress = initObj.restart_in_progress
      }
      else {
        this.restart_in_progress = false;
      }
      if (initObj.hasOwnProperty('restart_scope')) {
        this.restart_scope = initObj.restart_scope
      }
      else {
        this.restart_scope = 0;
      }
      if (initObj.hasOwnProperty('watchdog_position_timeout')) {
        this.watchdog_position_timeout = initObj.watchdog_position_timeout
      }
      else {
        this.watchdog_position_timeout = false;
      }
      if (initObj.hasOwnProperty('watchdog_low_frequency')) {
        this.watchdog_low_frequency = initObj.watchdog_low_frequency
      }
      else {
        this.watchdog_low_frequency = false;
      }
      if (initObj.hasOwnProperty('automatic_restart_suspended')) {
        this.automatic_restart_suspended = initObj.automatic_restart_suspended
      }
      else {
        this.automatic_restart_suspended = false;
      }
      if (initObj.hasOwnProperty('automatic_restart_count')) {
        this.automatic_restart_count = initObj.automatic_restart_count
      }
      else {
        this.automatic_restart_count = 0;
      }
      if (initObj.hasOwnProperty('stop_required')) {
        this.stop_required = initObj.stop_required
      }
      else {
        this.stop_required = false;
      }
      if (initObj.hasOwnProperty('fault_code')) {
        this.fault_code = initObj.fault_code
      }
      else {
        this.fault_code = 0;
      }
      if (initObj.hasOwnProperty('reason')) {
        this.reason = initObj.reason
      }
      else {
        this.reason = '';
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type LocalizationStatus
    // Serialize message field [header]
    bufferOffset = std_msgs.msg.Header.serialize(obj.header, buffer, bufferOffset);
    // Serialize message field [reporter]
    bufferOffset = _serializer.string(obj.reporter, buffer, bufferOffset);
    // Serialize message field [requested_mode]
    bufferOffset = _serializer.uint8(obj.requested_mode, buffer, bufferOffset);
    // Serialize message field [active_mode]
    bufferOffset = _serializer.uint8(obj.active_mode, buffer, bufferOffset);
    // Serialize message field [workflow_mode]
    bufferOffset = _serializer.uint8(obj.workflow_mode, buffer, bufferOffset);
    // Serialize message field [requested_runtime_mode]
    bufferOffset = _serializer.uint8(obj.requested_runtime_mode, buffer, bufferOffset);
    // Serialize message field [active_runtime_mode]
    bufferOffset = _serializer.uint8(obj.active_runtime_mode, buffer, bufferOffset);
    // Serialize message field [phase]
    bufferOffset = _serializer.uint8(obj.phase, buffer, bufferOffset);
    // Serialize message field [position_source]
    bufferOffset = _serializer.uint8(obj.position_source, buffer, bufferOffset);
    // Serialize message field [position_state]
    bufferOffset = _serializer.uint32(obj.position_state, buffer, bufferOffset);
    // Serialize message field [position_valid]
    bufferOffset = _serializer.bool(obj.position_valid, buffer, bufferOffset);
    // Serialize message field [ins_status_received]
    bufferOffset = _serializer.bool(obj.ins_status_received, buffer, bufferOffset);
    // Serialize message field [ins_solution_good]
    bufferOffset = _serializer.bool(obj.ins_solution_good, buffer, bufferOffset);
    // Serialize message field [ins_initialized]
    bufferOffset = _serializer.bool(obj.ins_initialized, buffer, bufferOffset);
    // Serialize message field [ins_status]
    bufferOffset = _serializer.uint8(obj.ins_status, buffer, bufferOffset);
    // Serialize message field [ins_vehicle_align]
    bufferOffset = _serializer.uint8(obj.ins_vehicle_align, buffer, bufferOffset);
    // Serialize message field [ins_heading_flag]
    bufferOffset = _serializer.uint8(obj.ins_heading_flag, buffer, bufferOffset);
    // Serialize message field [ins_position_status]
    bufferOffset = _serializer.int32(obj.ins_position_status, buffer, bufferOffset);
    // Serialize message field [fast_lio_initialized]
    bufferOffset = _serializer.bool(obj.fast_lio_initialized, buffer, bufferOffset);
    // Serialize message field [fusion_generation_valid]
    bufferOffset = _serializer.bool(obj.fusion_generation_valid, buffer, bufferOffset);
    // Serialize message field [fusion_generation_sec]
    bufferOffset = _serializer.uint32(obj.fusion_generation_sec, buffer, bufferOffset);
    // Serialize message field [fusion_generation_nsec]
    bufferOffset = _serializer.uint32(obj.fusion_generation_nsec, buffer, bufferOffset);
    // Serialize message field [lio_generation_valid]
    bufferOffset = _serializer.bool(obj.lio_generation_valid, buffer, bufferOffset);
    // Serialize message field [lio_generation_sec]
    bufferOffset = _serializer.uint32(obj.lio_generation_sec, buffer, bufferOffset);
    // Serialize message field [lio_generation_nsec]
    bufferOffset = _serializer.uint32(obj.lio_generation_nsec, buffer, bufferOffset);
    // Serialize message field [map_origin_ready]
    bufferOffset = _serializer.bool(obj.map_origin_ready, buffer, bufferOffset);
    // Serialize message field [map_origin_committed]
    bufferOffset = _serializer.bool(obj.map_origin_committed, buffer, bufferOffset);
    // Serialize message field [alignment_ready]
    bufferOffset = _serializer.bool(obj.alignment_ready, buffer, bufferOffset);
    // Serialize message field [alignment_locked]
    bufferOffset = _serializer.bool(obj.alignment_locked, buffer, bufferOffset);
    // Serialize message field [imu_odometry_ready]
    bufferOffset = _serializer.bool(obj.imu_odometry_ready, buffer, bufferOffset);
    // Serialize message field [gps_good]
    bufferOffset = _serializer.bool(obj.gps_good, buffer, bufferOffset);
    // Serialize message field [lio_good]
    bufferOffset = _serializer.bool(obj.lio_good, buffer, bufferOffset);
    // Serialize message field [active_map]
    bufferOffset = _serializer.string(obj.active_map, buffer, bufferOffset);
    // Serialize message field [pending_map]
    bufferOffset = _serializer.string(obj.pending_map, buffer, bufferOffset);
    // Serialize message field [last_created_map]
    bufferOffset = _serializer.string(obj.last_created_map, buffer, bufferOffset);
    // Serialize message field [active_map_uuid]
    bufferOffset = _serializer.string(obj.active_map_uuid, buffer, bufferOffset);
    // Serialize message field [pending_map_uuid]
    bufferOffset = _serializer.string(obj.pending_map_uuid, buffer, bufferOffset);
    // Serialize message field [map_schema_version]
    bufferOffset = _serializer.uint32(obj.map_schema_version, buffer, bufferOffset);
    // Serialize message field [coordinate_frame]
    bufferOffset = _serializer.string(obj.coordinate_frame, buffer, bufferOffset);
    // Serialize message field [origin_checksum]
    bufferOffset = _serializer.string(obj.origin_checksum, buffer, bufferOffset);
    // Serialize message field [map_valid]
    bufferOffset = _serializer.bool(obj.map_valid, buffer, bufferOffset);
    // Serialize message field [algorithm_status_valid]
    bufferOffset = _serializer.bool(obj.algorithm_status_valid, buffer, bufferOffset);
    // Serialize message field [transitioning]
    bufferOffset = _serializer.bool(obj.transitioning, buffer, bufferOffset);
    // Serialize message field [restart_in_progress]
    bufferOffset = _serializer.bool(obj.restart_in_progress, buffer, bufferOffset);
    // Serialize message field [restart_scope]
    bufferOffset = _serializer.uint8(obj.restart_scope, buffer, bufferOffset);
    // Serialize message field [watchdog_position_timeout]
    bufferOffset = _serializer.bool(obj.watchdog_position_timeout, buffer, bufferOffset);
    // Serialize message field [watchdog_low_frequency]
    bufferOffset = _serializer.bool(obj.watchdog_low_frequency, buffer, bufferOffset);
    // Serialize message field [automatic_restart_suspended]
    bufferOffset = _serializer.bool(obj.automatic_restart_suspended, buffer, bufferOffset);
    // Serialize message field [automatic_restart_count]
    bufferOffset = _serializer.uint32(obj.automatic_restart_count, buffer, bufferOffset);
    // Serialize message field [stop_required]
    bufferOffset = _serializer.bool(obj.stop_required, buffer, bufferOffset);
    // Serialize message field [fault_code]
    bufferOffset = _serializer.uint16(obj.fault_code, buffer, bufferOffset);
    // Serialize message field [reason]
    bufferOffset = _serializer.string(obj.reason, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type LocalizationStatus
    let len;
    let data = new LocalizationStatus(null);
    // Deserialize message field [header]
    data.header = std_msgs.msg.Header.deserialize(buffer, bufferOffset);
    // Deserialize message field [reporter]
    data.reporter = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [requested_mode]
    data.requested_mode = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [active_mode]
    data.active_mode = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [workflow_mode]
    data.workflow_mode = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [requested_runtime_mode]
    data.requested_runtime_mode = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [active_runtime_mode]
    data.active_runtime_mode = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [phase]
    data.phase = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [position_source]
    data.position_source = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [position_state]
    data.position_state = _deserializer.uint32(buffer, bufferOffset);
    // Deserialize message field [position_valid]
    data.position_valid = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [ins_status_received]
    data.ins_status_received = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [ins_solution_good]
    data.ins_solution_good = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [ins_initialized]
    data.ins_initialized = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [ins_status]
    data.ins_status = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [ins_vehicle_align]
    data.ins_vehicle_align = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [ins_heading_flag]
    data.ins_heading_flag = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [ins_position_status]
    data.ins_position_status = _deserializer.int32(buffer, bufferOffset);
    // Deserialize message field [fast_lio_initialized]
    data.fast_lio_initialized = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [fusion_generation_valid]
    data.fusion_generation_valid = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [fusion_generation_sec]
    data.fusion_generation_sec = _deserializer.uint32(buffer, bufferOffset);
    // Deserialize message field [fusion_generation_nsec]
    data.fusion_generation_nsec = _deserializer.uint32(buffer, bufferOffset);
    // Deserialize message field [lio_generation_valid]
    data.lio_generation_valid = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [lio_generation_sec]
    data.lio_generation_sec = _deserializer.uint32(buffer, bufferOffset);
    // Deserialize message field [lio_generation_nsec]
    data.lio_generation_nsec = _deserializer.uint32(buffer, bufferOffset);
    // Deserialize message field [map_origin_ready]
    data.map_origin_ready = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [map_origin_committed]
    data.map_origin_committed = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [alignment_ready]
    data.alignment_ready = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [alignment_locked]
    data.alignment_locked = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [imu_odometry_ready]
    data.imu_odometry_ready = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [gps_good]
    data.gps_good = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [lio_good]
    data.lio_good = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [active_map]
    data.active_map = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [pending_map]
    data.pending_map = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [last_created_map]
    data.last_created_map = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [active_map_uuid]
    data.active_map_uuid = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [pending_map_uuid]
    data.pending_map_uuid = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [map_schema_version]
    data.map_schema_version = _deserializer.uint32(buffer, bufferOffset);
    // Deserialize message field [coordinate_frame]
    data.coordinate_frame = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [origin_checksum]
    data.origin_checksum = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [map_valid]
    data.map_valid = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [algorithm_status_valid]
    data.algorithm_status_valid = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [transitioning]
    data.transitioning = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [restart_in_progress]
    data.restart_in_progress = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [restart_scope]
    data.restart_scope = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [watchdog_position_timeout]
    data.watchdog_position_timeout = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [watchdog_low_frequency]
    data.watchdog_low_frequency = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [automatic_restart_suspended]
    data.automatic_restart_suspended = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [automatic_restart_count]
    data.automatic_restart_count = _deserializer.uint32(buffer, bufferOffset);
    // Deserialize message field [stop_required]
    data.stop_required = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [fault_code]
    data.fault_code = _deserializer.uint16(buffer, bufferOffset);
    // Deserialize message field [reason]
    data.reason = _deserializer.string(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += std_msgs.msg.Header.getMessageSize(object.header);
    length += _getByteLength(object.reporter);
    length += _getByteLength(object.active_map);
    length += _getByteLength(object.pending_map);
    length += _getByteLength(object.last_created_map);
    length += _getByteLength(object.active_map_uuid);
    length += _getByteLength(object.pending_map_uuid);
    length += _getByteLength(object.coordinate_frame);
    length += _getByteLength(object.origin_checksum);
    length += _getByteLength(object.reason);
    return length + 103;
  }

  static datatype() {
    // Returns string type for a message object
    return 'util/LocalizationStatus';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '66d83c808f19f476c7a0fb306057009c';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
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
    
    ================================================================================
    MSG: std_msgs/Header
    # Standard metadata for higher-level stamped data types.
    # This is generally used to communicate timestamped data 
    # in a particular coordinate frame.
    # 
    # sequence ID: consecutively increasing ID 
    uint32 seq
    #Two-integer timestamp that is expressed as:
    # * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')
    # * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')
    # time-handling sugar is provided by the client library
    time stamp
    #Frame this data is associated with
    string frame_id
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new LocalizationStatus(null);
    if (msg.header !== undefined) {
      resolved.header = std_msgs.msg.Header.Resolve(msg.header)
    }
    else {
      resolved.header = new std_msgs.msg.Header()
    }

    if (msg.reporter !== undefined) {
      resolved.reporter = msg.reporter;
    }
    else {
      resolved.reporter = ''
    }

    if (msg.requested_mode !== undefined) {
      resolved.requested_mode = msg.requested_mode;
    }
    else {
      resolved.requested_mode = 0
    }

    if (msg.active_mode !== undefined) {
      resolved.active_mode = msg.active_mode;
    }
    else {
      resolved.active_mode = 0
    }

    if (msg.workflow_mode !== undefined) {
      resolved.workflow_mode = msg.workflow_mode;
    }
    else {
      resolved.workflow_mode = 0
    }

    if (msg.requested_runtime_mode !== undefined) {
      resolved.requested_runtime_mode = msg.requested_runtime_mode;
    }
    else {
      resolved.requested_runtime_mode = 0
    }

    if (msg.active_runtime_mode !== undefined) {
      resolved.active_runtime_mode = msg.active_runtime_mode;
    }
    else {
      resolved.active_runtime_mode = 0
    }

    if (msg.phase !== undefined) {
      resolved.phase = msg.phase;
    }
    else {
      resolved.phase = 0
    }

    if (msg.position_source !== undefined) {
      resolved.position_source = msg.position_source;
    }
    else {
      resolved.position_source = 0
    }

    if (msg.position_state !== undefined) {
      resolved.position_state = msg.position_state;
    }
    else {
      resolved.position_state = 0
    }

    if (msg.position_valid !== undefined) {
      resolved.position_valid = msg.position_valid;
    }
    else {
      resolved.position_valid = false
    }

    if (msg.ins_status_received !== undefined) {
      resolved.ins_status_received = msg.ins_status_received;
    }
    else {
      resolved.ins_status_received = false
    }

    if (msg.ins_solution_good !== undefined) {
      resolved.ins_solution_good = msg.ins_solution_good;
    }
    else {
      resolved.ins_solution_good = false
    }

    if (msg.ins_initialized !== undefined) {
      resolved.ins_initialized = msg.ins_initialized;
    }
    else {
      resolved.ins_initialized = false
    }

    if (msg.ins_status !== undefined) {
      resolved.ins_status = msg.ins_status;
    }
    else {
      resolved.ins_status = 0
    }

    if (msg.ins_vehicle_align !== undefined) {
      resolved.ins_vehicle_align = msg.ins_vehicle_align;
    }
    else {
      resolved.ins_vehicle_align = 0
    }

    if (msg.ins_heading_flag !== undefined) {
      resolved.ins_heading_flag = msg.ins_heading_flag;
    }
    else {
      resolved.ins_heading_flag = 0
    }

    if (msg.ins_position_status !== undefined) {
      resolved.ins_position_status = msg.ins_position_status;
    }
    else {
      resolved.ins_position_status = 0
    }

    if (msg.fast_lio_initialized !== undefined) {
      resolved.fast_lio_initialized = msg.fast_lio_initialized;
    }
    else {
      resolved.fast_lio_initialized = false
    }

    if (msg.fusion_generation_valid !== undefined) {
      resolved.fusion_generation_valid = msg.fusion_generation_valid;
    }
    else {
      resolved.fusion_generation_valid = false
    }

    if (msg.fusion_generation_sec !== undefined) {
      resolved.fusion_generation_sec = msg.fusion_generation_sec;
    }
    else {
      resolved.fusion_generation_sec = 0
    }

    if (msg.fusion_generation_nsec !== undefined) {
      resolved.fusion_generation_nsec = msg.fusion_generation_nsec;
    }
    else {
      resolved.fusion_generation_nsec = 0
    }

    if (msg.lio_generation_valid !== undefined) {
      resolved.lio_generation_valid = msg.lio_generation_valid;
    }
    else {
      resolved.lio_generation_valid = false
    }

    if (msg.lio_generation_sec !== undefined) {
      resolved.lio_generation_sec = msg.lio_generation_sec;
    }
    else {
      resolved.lio_generation_sec = 0
    }

    if (msg.lio_generation_nsec !== undefined) {
      resolved.lio_generation_nsec = msg.lio_generation_nsec;
    }
    else {
      resolved.lio_generation_nsec = 0
    }

    if (msg.map_origin_ready !== undefined) {
      resolved.map_origin_ready = msg.map_origin_ready;
    }
    else {
      resolved.map_origin_ready = false
    }

    if (msg.map_origin_committed !== undefined) {
      resolved.map_origin_committed = msg.map_origin_committed;
    }
    else {
      resolved.map_origin_committed = false
    }

    if (msg.alignment_ready !== undefined) {
      resolved.alignment_ready = msg.alignment_ready;
    }
    else {
      resolved.alignment_ready = false
    }

    if (msg.alignment_locked !== undefined) {
      resolved.alignment_locked = msg.alignment_locked;
    }
    else {
      resolved.alignment_locked = false
    }

    if (msg.imu_odometry_ready !== undefined) {
      resolved.imu_odometry_ready = msg.imu_odometry_ready;
    }
    else {
      resolved.imu_odometry_ready = false
    }

    if (msg.gps_good !== undefined) {
      resolved.gps_good = msg.gps_good;
    }
    else {
      resolved.gps_good = false
    }

    if (msg.lio_good !== undefined) {
      resolved.lio_good = msg.lio_good;
    }
    else {
      resolved.lio_good = false
    }

    if (msg.active_map !== undefined) {
      resolved.active_map = msg.active_map;
    }
    else {
      resolved.active_map = ''
    }

    if (msg.pending_map !== undefined) {
      resolved.pending_map = msg.pending_map;
    }
    else {
      resolved.pending_map = ''
    }

    if (msg.last_created_map !== undefined) {
      resolved.last_created_map = msg.last_created_map;
    }
    else {
      resolved.last_created_map = ''
    }

    if (msg.active_map_uuid !== undefined) {
      resolved.active_map_uuid = msg.active_map_uuid;
    }
    else {
      resolved.active_map_uuid = ''
    }

    if (msg.pending_map_uuid !== undefined) {
      resolved.pending_map_uuid = msg.pending_map_uuid;
    }
    else {
      resolved.pending_map_uuid = ''
    }

    if (msg.map_schema_version !== undefined) {
      resolved.map_schema_version = msg.map_schema_version;
    }
    else {
      resolved.map_schema_version = 0
    }

    if (msg.coordinate_frame !== undefined) {
      resolved.coordinate_frame = msg.coordinate_frame;
    }
    else {
      resolved.coordinate_frame = ''
    }

    if (msg.origin_checksum !== undefined) {
      resolved.origin_checksum = msg.origin_checksum;
    }
    else {
      resolved.origin_checksum = ''
    }

    if (msg.map_valid !== undefined) {
      resolved.map_valid = msg.map_valid;
    }
    else {
      resolved.map_valid = false
    }

    if (msg.algorithm_status_valid !== undefined) {
      resolved.algorithm_status_valid = msg.algorithm_status_valid;
    }
    else {
      resolved.algorithm_status_valid = false
    }

    if (msg.transitioning !== undefined) {
      resolved.transitioning = msg.transitioning;
    }
    else {
      resolved.transitioning = false
    }

    if (msg.restart_in_progress !== undefined) {
      resolved.restart_in_progress = msg.restart_in_progress;
    }
    else {
      resolved.restart_in_progress = false
    }

    if (msg.restart_scope !== undefined) {
      resolved.restart_scope = msg.restart_scope;
    }
    else {
      resolved.restart_scope = 0
    }

    if (msg.watchdog_position_timeout !== undefined) {
      resolved.watchdog_position_timeout = msg.watchdog_position_timeout;
    }
    else {
      resolved.watchdog_position_timeout = false
    }

    if (msg.watchdog_low_frequency !== undefined) {
      resolved.watchdog_low_frequency = msg.watchdog_low_frequency;
    }
    else {
      resolved.watchdog_low_frequency = false
    }

    if (msg.automatic_restart_suspended !== undefined) {
      resolved.automatic_restart_suspended = msg.automatic_restart_suspended;
    }
    else {
      resolved.automatic_restart_suspended = false
    }

    if (msg.automatic_restart_count !== undefined) {
      resolved.automatic_restart_count = msg.automatic_restart_count;
    }
    else {
      resolved.automatic_restart_count = 0
    }

    if (msg.stop_required !== undefined) {
      resolved.stop_required = msg.stop_required;
    }
    else {
      resolved.stop_required = false
    }

    if (msg.fault_code !== undefined) {
      resolved.fault_code = msg.fault_code;
    }
    else {
      resolved.fault_code = 0
    }

    if (msg.reason !== undefined) {
      resolved.reason = msg.reason;
    }
    else {
      resolved.reason = ''
    }

    return resolved;
    }
};

// Constants for message
LocalizationStatus.Constants = {
  MODE_UNKNOWN: 0,
  MODE_IDLE: 1,
  MODE_MAPPING: 2,
  MODE_LOCALIZATION: 3,
  WORKFLOW_UNKNOWN: 0,
  WORKFLOW_MAPPING: 1,
  WORKFLOW_LOCALIZATION: 2,
  RUNTIME_UNKNOWN: 0,
  RUNTIME_CREATE_ORIGIN: 1,
  RUNTIME_TRACK_SAVED_ORIGIN: 2,
  PHASE_UNKNOWN: 0,
  PHASE_WAIT_FAST_LIO: 1,
  PHASE_WAIT_INS: 2,
  PHASE_WAIT_MAP: 3,
  PHASE_WAIT_GPS: 4,
  PHASE_WAIT_ALIGNMENT: 5,
  PHASE_READY_RTK: 6,
  PHASE_READY_LIO_LOCKED: 7,
  PHASE_READY_LIO_SEEDED: 8,
  PHASE_READY_IMU_RESTRICTED: 9,
  PHASE_SWITCHING_MAP: 10,
  PHASE_RESTARTING_FUSION: 11,
  PHASE_RESTARTING_ALL: 12,
  PHASE_RECOVERING_LIO: 13,
  PHASE_FAULT: 14,
  PHASE_WAIT_IMU_ODOMETRY: 15,
  PHASE_RESTART_VERIFYING: 16,
  PHASE_WAIT_ALGORITHM: 17,
  PHASE_WAIT_TRUSTED_OUTPUT: 18,
  SOURCE_NONE: 0,
  SOURCE_RTK: 1,
  SOURCE_LIO_LOCKED: 2,
  SOURCE_LIO_SEEDED: 3,
  SOURCE_IMU_RESTRICTED: 4,
  RESTART_NONE: 0,
  RESTART_FUSION: 1,
  RESTART_LIO: 2,
  RESTART_ALL: 3,
  FAULT_NONE: 0,
  FAULT_MAP_INVALID: 1,
  FAULT_RESTART_FAILED: 2,
  FAULT_RESTART_SUPPRESSED: 3,
  FAULT_POSITION_TIMEOUT: 4,
  FAULT_LOW_FREQUENCY: 5,
  FAULT_ALGORITHM: 6,
  FAULT_RESTART_PARTIAL: 7,
}

module.exports = LocalizationStatus;
