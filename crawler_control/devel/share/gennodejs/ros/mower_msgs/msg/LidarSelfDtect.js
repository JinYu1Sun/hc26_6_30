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

class LidarSelfDtect {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.is_lidar_ok = null;
    }
    else {
      if (initObj.hasOwnProperty('is_lidar_ok')) {
        this.is_lidar_ok = initObj.is_lidar_ok
      }
      else {
        this.is_lidar_ok = false;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type LidarSelfDtect
    // Serialize message field [is_lidar_ok]
    bufferOffset = _serializer.bool(obj.is_lidar_ok, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type LidarSelfDtect
    let len;
    let data = new LidarSelfDtect(null);
    // Deserialize message field [is_lidar_ok]
    data.is_lidar_ok = _deserializer.bool(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    return 1;
  }

  static datatype() {
    // Returns string type for a message object
    return 'mower_msgs/LidarSelfDtect';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '61b1182e9e63e0b66c126403f70148e4';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    bool is_lidar_ok
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new LidarSelfDtect(null);
    if (msg.is_lidar_ok !== undefined) {
      resolved.is_lidar_ok = msg.is_lidar_ok;
    }
    else {
      resolved.is_lidar_ok = false
    }

    return resolved;
    }
};

module.exports = LidarSelfDtect;
