// Auto-generated. Do not edit!

// (in-package mower_msgs.msg)


"use strict";

const _serializer = _ros_msg_utils.Serialize;
const _arraySerializer = _serializer.Array;
const _deserializer = _ros_msg_utils.Deserialize;
const _arrayDeserializer = _deserializer.Array;
const _finder = _ros_msg_utils.Find;
const _getByteLength = _ros_msg_utils.getByteLength;

//-----------------------------------------------------------

class Manual_Driving_Cmd {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.ad_control_enable = null;
      this.turn_value = null;
      this.drive_value = null;
      this.gear_model = null;
      this.mover_bool = null;
      this.mow_height = null;
    }
    else {
      if (initObj.hasOwnProperty('ad_control_enable')) {
        this.ad_control_enable = initObj.ad_control_enable
      }
      else {
        this.ad_control_enable = 0;
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
      if (initObj.hasOwnProperty('gear_model')) {
        this.gear_model = initObj.gear_model
      }
      else {
        this.gear_model = 0;
      }
      if (initObj.hasOwnProperty('mover_bool')) {
        this.mover_bool = initObj.mover_bool
      }
      else {
        this.mover_bool = 0;
      }
      if (initObj.hasOwnProperty('mow_height')) {
        this.mow_height = initObj.mow_height
      }
      else {
        this.mow_height = 0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type Manual_Driving_Cmd
    // Serialize message field [ad_control_enable]
    bufferOffset = _serializer.uint8(obj.ad_control_enable, buffer, bufferOffset);
    // Serialize message field [turn_value]
    bufferOffset = _serializer.uint16(obj.turn_value, buffer, bufferOffset);
    // Serialize message field [drive_value]
    bufferOffset = _serializer.uint16(obj.drive_value, buffer, bufferOffset);
    // Serialize message field [gear_model]
    bufferOffset = _serializer.uint8(obj.gear_model, buffer, bufferOffset);
    // Serialize message field [mover_bool]
    bufferOffset = _serializer.uint8(obj.mover_bool, buffer, bufferOffset);
    // Serialize message field [mow_height]
    bufferOffset = _serializer.uint8(obj.mow_height, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type Manual_Driving_Cmd
    let len;
    let data = new Manual_Driving_Cmd(null);
    // Deserialize message field [ad_control_enable]
    data.ad_control_enable = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [turn_value]
    data.turn_value = _deserializer.uint16(buffer, bufferOffset);
    // Deserialize message field [drive_value]
    data.drive_value = _deserializer.uint16(buffer, bufferOffset);
    // Deserialize message field [gear_model]
    data.gear_model = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [mover_bool]
    data.mover_bool = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [mow_height]
    data.mow_height = _deserializer.uint8(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    return 8;
  }

  static datatype() {
    // Returns string type for a message object
    return 'mower_msgs/Manual_Driving_Cmd';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '2904fb773f7150886bc5690b6fc3e977';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    #Manual_Driving_Cmd
    uint8 ad_control_enable
    uint16 turn_value
    uint16 drive_value
    uint8 gear_model
    uint8 mover_bool
    uint8 mow_height
    
    
    
    
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new Manual_Driving_Cmd(null);
    if (msg.ad_control_enable !== undefined) {
      resolved.ad_control_enable = msg.ad_control_enable;
    }
    else {
      resolved.ad_control_enable = 0
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

    if (msg.gear_model !== undefined) {
      resolved.gear_model = msg.gear_model;
    }
    else {
      resolved.gear_model = 0
    }

    if (msg.mover_bool !== undefined) {
      resolved.mover_bool = msg.mover_bool;
    }
    else {
      resolved.mover_bool = 0
    }

    if (msg.mow_height !== undefined) {
      resolved.mow_height = msg.mow_height;
    }
    else {
      resolved.mow_height = 0
    }

    return resolved;
    }
};

module.exports = Manual_Driving_Cmd;
