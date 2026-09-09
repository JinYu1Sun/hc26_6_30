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

class TimeAreaLeft {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.time_left = null;
      this.area_left = null;
    }
    else {
      if (initObj.hasOwnProperty('time_left')) {
        this.time_left = initObj.time_left
      }
      else {
        this.time_left = 0.0;
      }
      if (initObj.hasOwnProperty('area_left')) {
        this.area_left = initObj.area_left
      }
      else {
        this.area_left = 0.0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type TimeAreaLeft
    // Serialize message field [time_left]
    bufferOffset = _serializer.float64(obj.time_left, buffer, bufferOffset);
    // Serialize message field [area_left]
    bufferOffset = _serializer.float64(obj.area_left, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type TimeAreaLeft
    let len;
    let data = new TimeAreaLeft(null);
    // Deserialize message field [time_left]
    data.time_left = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [area_left]
    data.area_left = _deserializer.float64(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    return 16;
  }

  static datatype() {
    // Returns string type for a message object
    return 'mower_msgs/TimeAreaLeft';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '3450601933b56d4bc50d61223a1754f6';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    float64 time_left
    float64 area_left
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new TimeAreaLeft(null);
    if (msg.time_left !== undefined) {
      resolved.time_left = msg.time_left;
    }
    else {
      resolved.time_left = 0.0
    }

    if (msg.area_left !== undefined) {
      resolved.area_left = msg.area_left;
    }
    else {
      resolved.area_left = 0.0
    }

    return resolved;
    }
};

module.exports = TimeAreaLeft;
