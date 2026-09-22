// Auto-generated. Do not edit!

// (in-package util.srv)


"use strict";

const _serializer = _ros_msg_utils.Serialize;
const _arraySerializer = _serializer.Array;
const _deserializer = _ros_msg_utils.Deserialize;
const _arrayDeserializer = _deserializer.Array;
const _finder = _ros_msg_utils.Find;
const _getByteLength = _ros_msg_utils.getByteLength;

//-----------------------------------------------------------


//-----------------------------------------------------------

class ManageLocalizationMapRequest {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.command = null;
      this.map_name = null;
      this.overwrite = null;
    }
    else {
      if (initObj.hasOwnProperty('command')) {
        this.command = initObj.command
      }
      else {
        this.command = 0;
      }
      if (initObj.hasOwnProperty('map_name')) {
        this.map_name = initObj.map_name
      }
      else {
        this.map_name = '';
      }
      if (initObj.hasOwnProperty('overwrite')) {
        this.overwrite = initObj.overwrite
      }
      else {
        this.overwrite = false;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type ManageLocalizationMapRequest
    // Serialize message field [command]
    bufferOffset = _serializer.uint8(obj.command, buffer, bufferOffset);
    // Serialize message field [map_name]
    bufferOffset = _serializer.string(obj.map_name, buffer, bufferOffset);
    // Serialize message field [overwrite]
    bufferOffset = _serializer.bool(obj.overwrite, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type ManageLocalizationMapRequest
    let len;
    let data = new ManageLocalizationMapRequest(null);
    // Deserialize message field [command]
    data.command = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [map_name]
    data.map_name = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [overwrite]
    data.overwrite = _deserializer.bool(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += _getByteLength(object.map_name);
    return length + 6;
  }

  static datatype() {
    // Returns string type for a service object
    return 'util/ManageLocalizationMapRequest';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '97835c85afd8e1d5520dfc73f4fbb216';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    # Transactional map lifecycle API owned by fusion_4dof.
    uint8 SAVE_CURRENT_AND_ACTIVATE=1
    uint8 ACTIVATE_EXISTING=2
    
    uint8 command
    string map_name
    bool overwrite
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new ManageLocalizationMapRequest(null);
    if (msg.command !== undefined) {
      resolved.command = msg.command;
    }
    else {
      resolved.command = 0
    }

    if (msg.map_name !== undefined) {
      resolved.map_name = msg.map_name;
    }
    else {
      resolved.map_name = ''
    }

    if (msg.overwrite !== undefined) {
      resolved.overwrite = msg.overwrite;
    }
    else {
      resolved.overwrite = false
    }

    return resolved;
    }
};

// Constants for message
ManageLocalizationMapRequest.Constants = {
  SAVE_CURRENT_AND_ACTIVATE: 1,
  ACTIVATE_EXISTING: 2,
}

class ManageLocalizationMapResponse {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.success = null;
      this.alignment_preserved = null;
      this.switch_pending = null;
      this.active_map = null;
      this.active_map_uuid = null;
      this.pending_map = null;
      this.pending_map_uuid = null;
      this.map_schema_version = null;
      this.coordinate_frame = null;
      this.origin_checksum = null;
      this.reason = null;
    }
    else {
      if (initObj.hasOwnProperty('success')) {
        this.success = initObj.success
      }
      else {
        this.success = false;
      }
      if (initObj.hasOwnProperty('alignment_preserved')) {
        this.alignment_preserved = initObj.alignment_preserved
      }
      else {
        this.alignment_preserved = false;
      }
      if (initObj.hasOwnProperty('switch_pending')) {
        this.switch_pending = initObj.switch_pending
      }
      else {
        this.switch_pending = false;
      }
      if (initObj.hasOwnProperty('active_map')) {
        this.active_map = initObj.active_map
      }
      else {
        this.active_map = '';
      }
      if (initObj.hasOwnProperty('active_map_uuid')) {
        this.active_map_uuid = initObj.active_map_uuid
      }
      else {
        this.active_map_uuid = '';
      }
      if (initObj.hasOwnProperty('pending_map')) {
        this.pending_map = initObj.pending_map
      }
      else {
        this.pending_map = '';
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
      if (initObj.hasOwnProperty('reason')) {
        this.reason = initObj.reason
      }
      else {
        this.reason = '';
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type ManageLocalizationMapResponse
    // Serialize message field [success]
    bufferOffset = _serializer.bool(obj.success, buffer, bufferOffset);
    // Serialize message field [alignment_preserved]
    bufferOffset = _serializer.bool(obj.alignment_preserved, buffer, bufferOffset);
    // Serialize message field [switch_pending]
    bufferOffset = _serializer.bool(obj.switch_pending, buffer, bufferOffset);
    // Serialize message field [active_map]
    bufferOffset = _serializer.string(obj.active_map, buffer, bufferOffset);
    // Serialize message field [active_map_uuid]
    bufferOffset = _serializer.string(obj.active_map_uuid, buffer, bufferOffset);
    // Serialize message field [pending_map]
    bufferOffset = _serializer.string(obj.pending_map, buffer, bufferOffset);
    // Serialize message field [pending_map_uuid]
    bufferOffset = _serializer.string(obj.pending_map_uuid, buffer, bufferOffset);
    // Serialize message field [map_schema_version]
    bufferOffset = _serializer.uint32(obj.map_schema_version, buffer, bufferOffset);
    // Serialize message field [coordinate_frame]
    bufferOffset = _serializer.string(obj.coordinate_frame, buffer, bufferOffset);
    // Serialize message field [origin_checksum]
    bufferOffset = _serializer.string(obj.origin_checksum, buffer, bufferOffset);
    // Serialize message field [reason]
    bufferOffset = _serializer.string(obj.reason, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type ManageLocalizationMapResponse
    let len;
    let data = new ManageLocalizationMapResponse(null);
    // Deserialize message field [success]
    data.success = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [alignment_preserved]
    data.alignment_preserved = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [switch_pending]
    data.switch_pending = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [active_map]
    data.active_map = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [active_map_uuid]
    data.active_map_uuid = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [pending_map]
    data.pending_map = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [pending_map_uuid]
    data.pending_map_uuid = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [map_schema_version]
    data.map_schema_version = _deserializer.uint32(buffer, bufferOffset);
    // Deserialize message field [coordinate_frame]
    data.coordinate_frame = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [origin_checksum]
    data.origin_checksum = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [reason]
    data.reason = _deserializer.string(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += _getByteLength(object.active_map);
    length += _getByteLength(object.active_map_uuid);
    length += _getByteLength(object.pending_map);
    length += _getByteLength(object.pending_map_uuid);
    length += _getByteLength(object.coordinate_frame);
    length += _getByteLength(object.origin_checksum);
    length += _getByteLength(object.reason);
    return length + 35;
  }

  static datatype() {
    // Returns string type for a service object
    return 'util/ManageLocalizationMapResponse';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '839a6a6f72eca859c62078e8cacaeaf4';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
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
    
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new ManageLocalizationMapResponse(null);
    if (msg.success !== undefined) {
      resolved.success = msg.success;
    }
    else {
      resolved.success = false
    }

    if (msg.alignment_preserved !== undefined) {
      resolved.alignment_preserved = msg.alignment_preserved;
    }
    else {
      resolved.alignment_preserved = false
    }

    if (msg.switch_pending !== undefined) {
      resolved.switch_pending = msg.switch_pending;
    }
    else {
      resolved.switch_pending = false
    }

    if (msg.active_map !== undefined) {
      resolved.active_map = msg.active_map;
    }
    else {
      resolved.active_map = ''
    }

    if (msg.active_map_uuid !== undefined) {
      resolved.active_map_uuid = msg.active_map_uuid;
    }
    else {
      resolved.active_map_uuid = ''
    }

    if (msg.pending_map !== undefined) {
      resolved.pending_map = msg.pending_map;
    }
    else {
      resolved.pending_map = ''
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

    if (msg.reason !== undefined) {
      resolved.reason = msg.reason;
    }
    else {
      resolved.reason = ''
    }

    return resolved;
    }
};

module.exports = {
  Request: ManageLocalizationMapRequest,
  Response: ManageLocalizationMapResponse,
  md5sum() { return 'f6eb786c2817a22f906c5e66a8ae611a'; },
  datatype() { return 'util/ManageLocalizationMap'; }
};
