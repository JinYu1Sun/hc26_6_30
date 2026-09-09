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

class PerceptionSelfDetect {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.is_perception_ok = null;
    }
    else {
      if (initObj.hasOwnProperty('is_perception_ok')) {
        this.is_perception_ok = initObj.is_perception_ok
      }
      else {
        this.is_perception_ok = false;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type PerceptionSelfDetect
    // Serialize message field [is_perception_ok]
    bufferOffset = _serializer.bool(obj.is_perception_ok, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type PerceptionSelfDetect
    let len;
    let data = new PerceptionSelfDetect(null);
    // Deserialize message field [is_perception_ok]
    data.is_perception_ok = _deserializer.bool(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    return 1;
  }

  static datatype() {
    // Returns string type for a message object
    return 'mower_msgs/PerceptionSelfDetect';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '1e4f85ff5f6ce4dd7e761543975830a4';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    bool is_perception_ok
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new PerceptionSelfDetect(null);
    if (msg.is_perception_ok !== undefined) {
      resolved.is_perception_ok = msg.is_perception_ok;
    }
    else {
      resolved.is_perception_ok = false
    }

    return resolved;
    }
};

module.exports = PerceptionSelfDetect;
