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

class Direct_Control {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.drive_mode = null;
      this.speed_max = null;
      this.mow_height = null;
    }
    else {
      if (initObj.hasOwnProperty('drive_mode')) {
        this.drive_mode = initObj.drive_mode
      }
      else {
        this.drive_mode = 0;
      }
      if (initObj.hasOwnProperty('speed_max')) {
        this.speed_max = initObj.speed_max
      }
      else {
        this.speed_max = 0.0;
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
    // Serializes a message object of type Direct_Control
    // Serialize message field [drive_mode]
    bufferOffset = _serializer.uint8(obj.drive_mode, buffer, bufferOffset);
    // Serialize message field [speed_max]
    bufferOffset = _serializer.float64(obj.speed_max, buffer, bufferOffset);
    // Serialize message field [mow_height]
    bufferOffset = _serializer.uint8(obj.mow_height, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type Direct_Control
    let len;
    let data = new Direct_Control(null);
    // Deserialize message field [drive_mode]
    data.drive_mode = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [speed_max]
    data.speed_max = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [mow_height]
    data.mow_height = _deserializer.uint8(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    return 10;
  }

  static datatype() {
    // Returns string type for a message object
    return 'mower_msgs/Direct_Control';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '10579a0a02d1c1d2b089c0bf3df04393';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    uint8 drive_mode
    float64 speed_max
    uint8 mow_height
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new Direct_Control(null);
    if (msg.drive_mode !== undefined) {
      resolved.drive_mode = msg.drive_mode;
    }
    else {
      resolved.drive_mode = 0
    }

    if (msg.speed_max !== undefined) {
      resolved.speed_max = msg.speed_max;
    }
    else {
      resolved.speed_max = 0.0
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

module.exports = Direct_Control;
