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

class PlaningOK {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.is_planing_ok = null;
    }
    else {
      if (initObj.hasOwnProperty('is_planing_ok')) {
        this.is_planing_ok = initObj.is_planing_ok
      }
      else {
        this.is_planing_ok = false;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type PlaningOK
    // Serialize message field [is_planing_ok]
    bufferOffset = _serializer.bool(obj.is_planing_ok, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type PlaningOK
    let len;
    let data = new PlaningOK(null);
    // Deserialize message field [is_planing_ok]
    data.is_planing_ok = _deserializer.bool(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    return 1;
  }

  static datatype() {
    // Returns string type for a message object
    return 'mower_msgs/PlaningOK';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '60643f33fa88814b437453da7ce2bc4b';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    bool is_planing_ok
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new PlaningOK(null);
    if (msg.is_planing_ok !== undefined) {
      resolved.is_planing_ok = msg.is_planing_ok;
    }
    else {
      resolved.is_planing_ok = false
    }

    return resolved;
    }
};

module.exports = PlaningOK;
