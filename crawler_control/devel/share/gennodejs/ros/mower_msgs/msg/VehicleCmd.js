// Auto-generated. Do not edit!

// (in-package mower_msgs.msg)


"use strict";

const _serializer = _ros_msg_utils.Serialize;
const _arraySerializer = _serializer.Array;
const _deserializer = _ros_msg_utils.Deserialize;
const _arrayDeserializer = _deserializer.Array;
const _finder = _ros_msg_utils.Find;
const _getByteLength = _ros_msg_utils.getByteLength;
let std_msgs = _finder('std_msgs');

//-----------------------------------------------------------

class VehicleCmd {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.header = null;
      this.mover_bool = null;
      this.ad_control_enable = null;
      this.gear_model = null;
      this.turn_value = null;
      this.drive_value = null;
      this.mower_height = null;
      this.left_wheel_speed = null;
      this.right_wheel_speed = null;
      this.speed_cmd = null;
    }
    else {
      if (initObj.hasOwnProperty('header')) {
        this.header = initObj.header
      }
      else {
        this.header = new std_msgs.msg.Header();
      }
      if (initObj.hasOwnProperty('mover_bool')) {
        this.mover_bool = initObj.mover_bool
      }
      else {
        this.mover_bool = 0;
      }
      if (initObj.hasOwnProperty('ad_control_enable')) {
        this.ad_control_enable = initObj.ad_control_enable
      }
      else {
        this.ad_control_enable = 0;
      }
      if (initObj.hasOwnProperty('gear_model')) {
        this.gear_model = initObj.gear_model
      }
      else {
        this.gear_model = 0;
      }
      if (initObj.hasOwnProperty('turn_value')) {
        this.turn_value = initObj.turn_value
      }
      else {
        this.turn_value = 0;
      }
      if (initObj.hasOwnProperty('drive_value')) {
        this.drive_value = initObj.drive_value
      }
      else {
        this.drive_value = 0;
      }
      if (initObj.hasOwnProperty('mower_height')) {
        this.mower_height = initObj.mower_height
      }
      else {
        this.mower_height = 0;
      }
      if (initObj.hasOwnProperty('left_wheel_speed')) {
        this.left_wheel_speed = initObj.left_wheel_speed
      }
      else {
        this.left_wheel_speed = 0.0;
      }
      if (initObj.hasOwnProperty('right_wheel_speed')) {
        this.right_wheel_speed = initObj.right_wheel_speed
      }
      else {
        this.right_wheel_speed = 0.0;
      }
      if (initObj.hasOwnProperty('speed_cmd')) {
        this.speed_cmd = initObj.speed_cmd
      }
      else {
        this.speed_cmd = 0.0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type VehicleCmd
    // Serialize message field [header]
    bufferOffset = std_msgs.msg.Header.serialize(obj.header, buffer, bufferOffset);
    // Serialize message field [mover_bool]
    bufferOffset = _serializer.uint8(obj.mover_bool, buffer, bufferOffset);
    // Serialize message field [ad_control_enable]
    bufferOffset = _serializer.uint8(obj.ad_control_enable, buffer, bufferOffset);
    // Serialize message field [gear_model]
    bufferOffset = _serializer.uint8(obj.gear_model, buffer, bufferOffset);
    // Serialize message field [turn_value]
    bufferOffset = _serializer.int16(obj.turn_value, buffer, bufferOffset);
    // Serialize message field [drive_value]
    bufferOffset = _serializer.int16(obj.drive_value, buffer, bufferOffset);
    // Serialize message field [mower_height]
    bufferOffset = _serializer.uint8(obj.mower_height, buffer, bufferOffset);
    // Serialize message field [left_wheel_speed]
    bufferOffset = _serializer.float64(obj.left_wheel_speed, buffer, bufferOffset);
    // Serialize message field [right_wheel_speed]
    bufferOffset = _serializer.float64(obj.right_wheel_speed, buffer, bufferOffset);
    // Serialize message field [speed_cmd]
    bufferOffset = _serializer.float64(obj.speed_cmd, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type VehicleCmd
    let len;
    let data = new VehicleCmd(null);
    // Deserialize message field [header]
    data.header = std_msgs.msg.Header.deserialize(buffer, bufferOffset);
    // Deserialize message field [mover_bool]
    data.mover_bool = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [ad_control_enable]
    data.ad_control_enable = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [gear_model]
    data.gear_model = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [turn_value]
    data.turn_value = _deserializer.int16(buffer, bufferOffset);
    // Deserialize message field [drive_value]
    data.drive_value = _deserializer.int16(buffer, bufferOffset);
    // Deserialize message field [mower_height]
    data.mower_height = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [left_wheel_speed]
    data.left_wheel_speed = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [right_wheel_speed]
    data.right_wheel_speed = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [speed_cmd]
    data.speed_cmd = _deserializer.float64(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += std_msgs.msg.Header.getMessageSize(object.header);
    return length + 32;
  }

  static datatype() {
    // Returns string type for a message object
    return 'mower_msgs/VehicleCmd';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'cd0b96d90dfa95f8decc74c9ae3e5d02';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    Header header
    # VehicleCmd
    # Gear_model
    uint8 p_Gear = 0 # 0x00： P 档
    uint8 R_Gear = 1 # 0x01： 无
    uint8 N_Gear = 2 # 0x02： 转弯信号
    uint8 D_Gear = 3 # 0x03： D 前进档
    
    # ADCmd 0x310
    uint8 mover_bool
    uint8 ad_control_enable 
    uint8 gear_model
    int16 turn_value
    int16 drive_value
    uint8 mower_height
    
    float64 left_wheel_speed  # 无
    float64 right_wheel_speed # 无
    float64 speed_cmd   # int8 [-128~127]
    
    
    
    
    
    
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
    const resolved = new VehicleCmd(null);
    if (msg.header !== undefined) {
      resolved.header = std_msgs.msg.Header.Resolve(msg.header)
    }
    else {
      resolved.header = new std_msgs.msg.Header()
    }

    if (msg.mover_bool !== undefined) {
      resolved.mover_bool = msg.mover_bool;
    }
    else {
      resolved.mover_bool = 0
    }

    if (msg.ad_control_enable !== undefined) {
      resolved.ad_control_enable = msg.ad_control_enable;
    }
    else {
      resolved.ad_control_enable = 0
    }

    if (msg.gear_model !== undefined) {
      resolved.gear_model = msg.gear_model;
    }
    else {
      resolved.gear_model = 0
    }

    if (msg.turn_value !== undefined) {
      resolved.turn_value = msg.turn_value;
    }
    else {
      resolved.turn_value = 0
    }

    if (msg.drive_value !== undefined) {
      resolved.drive_value = msg.drive_value;
    }
    else {
      resolved.drive_value = 0
    }

    if (msg.mower_height !== undefined) {
      resolved.mower_height = msg.mower_height;
    }
    else {
      resolved.mower_height = 0
    }

    if (msg.left_wheel_speed !== undefined) {
      resolved.left_wheel_speed = msg.left_wheel_speed;
    }
    else {
      resolved.left_wheel_speed = 0.0
    }

    if (msg.right_wheel_speed !== undefined) {
      resolved.right_wheel_speed = msg.right_wheel_speed;
    }
    else {
      resolved.right_wheel_speed = 0.0
    }

    if (msg.speed_cmd !== undefined) {
      resolved.speed_cmd = msg.speed_cmd;
    }
    else {
      resolved.speed_cmd = 0.0
    }

    return resolved;
    }
};

// Constants for message
VehicleCmd.Constants = {
  P_GEAR: 0,
  R_GEAR: 1,
  N_GEAR: 2,
  D_GEAR: 3,
}

module.exports = VehicleCmd;
