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

class VehicleReport {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.header = null;
      this.vehicle_gear = null;
      this.clamping_brake_status = null;
      this.drive_mode_state = null;
      this.vcu_speed_req = null;
      this.vehicle_soc = null;
      this.vehicle_status_msgcntr = null;
      this.vehicle_speed = null;
      this.vehicle_brake_pressure = null;
      this.vehicle_steering_angle = null;
      this.vehicle_status_2_msgcntr = null;
    }
    else {
      if (initObj.hasOwnProperty('header')) {
        this.header = initObj.header
      }
      else {
        this.header = new std_msgs.msg.Header();
      }
      if (initObj.hasOwnProperty('vehicle_gear')) {
        this.vehicle_gear = initObj.vehicle_gear
      }
      else {
        this.vehicle_gear = 0;
      }
      if (initObj.hasOwnProperty('clamping_brake_status')) {
        this.clamping_brake_status = initObj.clamping_brake_status
      }
      else {
        this.clamping_brake_status = 0;
      }
      if (initObj.hasOwnProperty('drive_mode_state')) {
        this.drive_mode_state = initObj.drive_mode_state
      }
      else {
        this.drive_mode_state = 0;
      }
      if (initObj.hasOwnProperty('vcu_speed_req')) {
        this.vcu_speed_req = initObj.vcu_speed_req
      }
      else {
        this.vcu_speed_req = 0.0;
      }
      if (initObj.hasOwnProperty('vehicle_soc')) {
        this.vehicle_soc = initObj.vehicle_soc
      }
      else {
        this.vehicle_soc = 0;
      }
      if (initObj.hasOwnProperty('vehicle_status_msgcntr')) {
        this.vehicle_status_msgcntr = initObj.vehicle_status_msgcntr
      }
      else {
        this.vehicle_status_msgcntr = 0;
      }
      if (initObj.hasOwnProperty('vehicle_speed')) {
        this.vehicle_speed = initObj.vehicle_speed
      }
      else {
        this.vehicle_speed = 0.0;
      }
      if (initObj.hasOwnProperty('vehicle_brake_pressure')) {
        this.vehicle_brake_pressure = initObj.vehicle_brake_pressure
      }
      else {
        this.vehicle_brake_pressure = 0.0;
      }
      if (initObj.hasOwnProperty('vehicle_steering_angle')) {
        this.vehicle_steering_angle = initObj.vehicle_steering_angle
      }
      else {
        this.vehicle_steering_angle = 0.0;
      }
      if (initObj.hasOwnProperty('vehicle_status_2_msgcntr')) {
        this.vehicle_status_2_msgcntr = initObj.vehicle_status_2_msgcntr
      }
      else {
        this.vehicle_status_2_msgcntr = 0.0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type VehicleReport
    // Serialize message field [header]
    bufferOffset = std_msgs.msg.Header.serialize(obj.header, buffer, bufferOffset);
    // Serialize message field [vehicle_gear]
    bufferOffset = _serializer.uint8(obj.vehicle_gear, buffer, bufferOffset);
    // Serialize message field [clamping_brake_status]
    bufferOffset = _serializer.uint8(obj.clamping_brake_status, buffer, bufferOffset);
    // Serialize message field [drive_mode_state]
    bufferOffset = _serializer.uint8(obj.drive_mode_state, buffer, bufferOffset);
    // Serialize message field [vcu_speed_req]
    bufferOffset = _serializer.float32(obj.vcu_speed_req, buffer, bufferOffset);
    // Serialize message field [vehicle_soc]
    bufferOffset = _serializer.uint8(obj.vehicle_soc, buffer, bufferOffset);
    // Serialize message field [vehicle_status_msgcntr]
    bufferOffset = _serializer.uint8(obj.vehicle_status_msgcntr, buffer, bufferOffset);
    // Serialize message field [vehicle_speed]
    bufferOffset = _serializer.float32(obj.vehicle_speed, buffer, bufferOffset);
    // Serialize message field [vehicle_brake_pressure]
    bufferOffset = _serializer.float32(obj.vehicle_brake_pressure, buffer, bufferOffset);
    // Serialize message field [vehicle_steering_angle]
    bufferOffset = _serializer.float32(obj.vehicle_steering_angle, buffer, bufferOffset);
    // Serialize message field [vehicle_status_2_msgcntr]
    bufferOffset = _serializer.float32(obj.vehicle_status_2_msgcntr, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type VehicleReport
    let len;
    let data = new VehicleReport(null);
    // Deserialize message field [header]
    data.header = std_msgs.msg.Header.deserialize(buffer, bufferOffset);
    // Deserialize message field [vehicle_gear]
    data.vehicle_gear = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [clamping_brake_status]
    data.clamping_brake_status = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [drive_mode_state]
    data.drive_mode_state = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [vcu_speed_req]
    data.vcu_speed_req = _deserializer.float32(buffer, bufferOffset);
    // Deserialize message field [vehicle_soc]
    data.vehicle_soc = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [vehicle_status_msgcntr]
    data.vehicle_status_msgcntr = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [vehicle_speed]
    data.vehicle_speed = _deserializer.float32(buffer, bufferOffset);
    // Deserialize message field [vehicle_brake_pressure]
    data.vehicle_brake_pressure = _deserializer.float32(buffer, bufferOffset);
    // Deserialize message field [vehicle_steering_angle]
    data.vehicle_steering_angle = _deserializer.float32(buffer, bufferOffset);
    // Deserialize message field [vehicle_status_2_msgcntr]
    data.vehicle_status_2_msgcntr = _deserializer.float32(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += std_msgs.msg.Header.getMessageSize(object.header);
    return length + 25;
  }

  static datatype() {
    // Returns string type for a message object
    return 'util/VehicleReport';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '8141fbcf1c26eb79107a5ea125eaf0a9';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    Header header
    
    # 0x303
    uint8 vehicle_gear
    uint8 clamping_brake_status
    uint8 drive_mode_state
    float32 vcu_speed_req
    uint8 vehicle_soc
    uint8 vehicle_status_msgcntr
    
    # 0x304
    float32 vehicle_speed
    float32 vehicle_brake_pressure
    float32 vehicle_steering_angle
    float32 vehicle_status_2_msgcntr
    
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
    const resolved = new VehicleReport(null);
    if (msg.header !== undefined) {
      resolved.header = std_msgs.msg.Header.Resolve(msg.header)
    }
    else {
      resolved.header = new std_msgs.msg.Header()
    }

    if (msg.vehicle_gear !== undefined) {
      resolved.vehicle_gear = msg.vehicle_gear;
    }
    else {
      resolved.vehicle_gear = 0
    }

    if (msg.clamping_brake_status !== undefined) {
      resolved.clamping_brake_status = msg.clamping_brake_status;
    }
    else {
      resolved.clamping_brake_status = 0
    }

    if (msg.drive_mode_state !== undefined) {
      resolved.drive_mode_state = msg.drive_mode_state;
    }
    else {
      resolved.drive_mode_state = 0
    }

    if (msg.vcu_speed_req !== undefined) {
      resolved.vcu_speed_req = msg.vcu_speed_req;
    }
    else {
      resolved.vcu_speed_req = 0.0
    }

    if (msg.vehicle_soc !== undefined) {
      resolved.vehicle_soc = msg.vehicle_soc;
    }
    else {
      resolved.vehicle_soc = 0
    }

    if (msg.vehicle_status_msgcntr !== undefined) {
      resolved.vehicle_status_msgcntr = msg.vehicle_status_msgcntr;
    }
    else {
      resolved.vehicle_status_msgcntr = 0
    }

    if (msg.vehicle_speed !== undefined) {
      resolved.vehicle_speed = msg.vehicle_speed;
    }
    else {
      resolved.vehicle_speed = 0.0
    }

    if (msg.vehicle_brake_pressure !== undefined) {
      resolved.vehicle_brake_pressure = msg.vehicle_brake_pressure;
    }
    else {
      resolved.vehicle_brake_pressure = 0.0
    }

    if (msg.vehicle_steering_angle !== undefined) {
      resolved.vehicle_steering_angle = msg.vehicle_steering_angle;
    }
    else {
      resolved.vehicle_steering_angle = 0.0
    }

    if (msg.vehicle_status_2_msgcntr !== undefined) {
      resolved.vehicle_status_2_msgcntr = msg.vehicle_status_2_msgcntr;
    }
    else {
      resolved.vehicle_status_2_msgcntr = 0.0
    }

    return resolved;
    }
};

module.exports = VehicleReport;
