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

class VehicleInfo {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.header = null;
      this.speed_status = null;
      this.leftw_speed = null;
      this.rightw_speed = null;
      this.gear_model = null;
      this.drive_mode = null;
      this.mow_height = null;
      this.battery_soc = null;
    }
    else {
      if (initObj.hasOwnProperty('header')) {
        this.header = initObj.header
      }
      else {
        this.header = new std_msgs.msg.Header();
      }
      if (initObj.hasOwnProperty('speed_status')) {
        this.speed_status = initObj.speed_status
      }
      else {
        this.speed_status = 0.0;
      }
      if (initObj.hasOwnProperty('leftw_speed')) {
        this.leftw_speed = initObj.leftw_speed
      }
      else {
        this.leftw_speed = 0.0;
      }
      if (initObj.hasOwnProperty('rightw_speed')) {
        this.rightw_speed = initObj.rightw_speed
      }
      else {
        this.rightw_speed = 0.0;
      }
      if (initObj.hasOwnProperty('gear_model')) {
        this.gear_model = initObj.gear_model
      }
      else {
        this.gear_model = 0;
      }
      if (initObj.hasOwnProperty('drive_mode')) {
        this.drive_mode = initObj.drive_mode
      }
      else {
        this.drive_mode = 0;
      }
      if (initObj.hasOwnProperty('mow_height')) {
        this.mow_height = initObj.mow_height
      }
      else {
        this.mow_height = 0;
      }
      if (initObj.hasOwnProperty('battery_soc')) {
        this.battery_soc = initObj.battery_soc
      }
      else {
        this.battery_soc = 0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type VehicleInfo
    // Serialize message field [header]
    bufferOffset = std_msgs.msg.Header.serialize(obj.header, buffer, bufferOffset);
    // Serialize message field [speed_status]
    bufferOffset = _serializer.float64(obj.speed_status, buffer, bufferOffset);
    // Serialize message field [leftw_speed]
    bufferOffset = _serializer.float64(obj.leftw_speed, buffer, bufferOffset);
    // Serialize message field [rightw_speed]
    bufferOffset = _serializer.float64(obj.rightw_speed, buffer, bufferOffset);
    // Serialize message field [gear_model]
    bufferOffset = _serializer.uint8(obj.gear_model, buffer, bufferOffset);
    // Serialize message field [drive_mode]
    bufferOffset = _serializer.uint8(obj.drive_mode, buffer, bufferOffset);
    // Serialize message field [mow_height]
    bufferOffset = _serializer.uint8(obj.mow_height, buffer, bufferOffset);
    // Serialize message field [battery_soc]
    bufferOffset = _serializer.uint8(obj.battery_soc, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type VehicleInfo
    let len;
    let data = new VehicleInfo(null);
    // Deserialize message field [header]
    data.header = std_msgs.msg.Header.deserialize(buffer, bufferOffset);
    // Deserialize message field [speed_status]
    data.speed_status = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [leftw_speed]
    data.leftw_speed = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [rightw_speed]
    data.rightw_speed = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [gear_model]
    data.gear_model = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [drive_mode]
    data.drive_mode = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [mow_height]
    data.mow_height = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [battery_soc]
    data.battery_soc = _deserializer.uint8(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += std_msgs.msg.Header.getMessageSize(object.header);
    return length + 28;
  }

  static datatype() {
    // Returns string type for a message object
    return 'mower_msgs/VehicleInfo';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '8a2a4b2a095bc9355e18d7be4b50f3fd';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    Header header
    
    # AGVStatus 0x301  现履带车底控无反馈，均不成立
    float64 speed_status  
    float64 leftw_speed   
    float64 rightw_speed
    uint8 gear_model      
    uint8 drive_mode
    uint8 mow_height
    uint8 battery_soc
    
    #gear_model
    uint8 P_Gear=0 # 0x00： P 档
    uint8 R_Gear=1 # 0x01： 倒档R
    uint8 N_Gear=2 # 0x02： N 档
    uint8 D_Gear=3 # 0x03： D 档
    #drive_model
    uint8 Manualcontrol_mode=0 # 0x00：手动控制模式
    uint8 Automaticcontrol_mode=1 # 0x01：自动控制模式
    uint8 Remotecontrol_mode=2 # 0x02：遥控器调试模式
    #mow_height
    uint8 Low_mode=2 # 0x02：割草高度为低
    uint8 Medium_mode=1 # 0x01：割草高度为中
    uint8 High_mode=0 # 0x00：割草高度为高
    
    
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
    const resolved = new VehicleInfo(null);
    if (msg.header !== undefined) {
      resolved.header = std_msgs.msg.Header.Resolve(msg.header)
    }
    else {
      resolved.header = new std_msgs.msg.Header()
    }

    if (msg.speed_status !== undefined) {
      resolved.speed_status = msg.speed_status;
    }
    else {
      resolved.speed_status = 0.0
    }

    if (msg.leftw_speed !== undefined) {
      resolved.leftw_speed = msg.leftw_speed;
    }
    else {
      resolved.leftw_speed = 0.0
    }

    if (msg.rightw_speed !== undefined) {
      resolved.rightw_speed = msg.rightw_speed;
    }
    else {
      resolved.rightw_speed = 0.0
    }

    if (msg.gear_model !== undefined) {
      resolved.gear_model = msg.gear_model;
    }
    else {
      resolved.gear_model = 0
    }

    if (msg.drive_mode !== undefined) {
      resolved.drive_mode = msg.drive_mode;
    }
    else {
      resolved.drive_mode = 0
    }

    if (msg.mow_height !== undefined) {
      resolved.mow_height = msg.mow_height;
    }
    else {
      resolved.mow_height = 0
    }

    if (msg.battery_soc !== undefined) {
      resolved.battery_soc = msg.battery_soc;
    }
    else {
      resolved.battery_soc = 0
    }

    return resolved;
    }
};

// Constants for message
VehicleInfo.Constants = {
  P_GEAR: 0,
  R_GEAR: 1,
  N_GEAR: 2,
  D_GEAR: 3,
  MANUALCONTROL_MODE: 0,
  AUTOMATICCONTROL_MODE: 1,
  REMOTECONTROL_MODE: 2,
  LOW_MODE: 2,
  MEDIUM_MODE: 1,
  HIGH_MODE: 0,
}

module.exports = VehicleInfo;
