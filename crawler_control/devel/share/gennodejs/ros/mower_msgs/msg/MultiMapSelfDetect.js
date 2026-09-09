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

class MultiMapSelfDetect {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.is_multi_map_ok = null;
    }
    else {
      if (initObj.hasOwnProperty('is_multi_map_ok')) {
        this.is_multi_map_ok = initObj.is_multi_map_ok
      }
      else {
        this.is_multi_map_ok = false;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type MultiMapSelfDetect
    // Serialize message field [is_multi_map_ok]
    bufferOffset = _serializer.bool(obj.is_multi_map_ok, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type MultiMapSelfDetect
    let len;
    let data = new MultiMapSelfDetect(null);
    // Deserialize message field [is_multi_map_ok]
    data.is_multi_map_ok = _deserializer.bool(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    return 1;
  }

  static datatype() {
    // Returns string type for a message object
    return 'mower_msgs/MultiMapSelfDetect';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '31d7c6c31bbf8db62e247ca761103d55';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    bool is_multi_map_ok
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new MultiMapSelfDetect(null);
    if (msg.is_multi_map_ok !== undefined) {
      resolved.is_multi_map_ok = msg.is_multi_map_ok;
    }
    else {
      resolved.is_multi_map_ok = false
    }

    return resolved;
    }
};

module.exports = MultiMapSelfDetect;
