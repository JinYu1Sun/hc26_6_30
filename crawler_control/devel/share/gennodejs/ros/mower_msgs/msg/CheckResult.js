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

class CheckResult {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.is_checkresult_ok = null;
    }
    else {
      if (initObj.hasOwnProperty('is_checkresult_ok')) {
        this.is_checkresult_ok = initObj.is_checkresult_ok
      }
      else {
        this.is_checkresult_ok = false;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type CheckResult
    // Serialize message field [is_checkresult_ok]
    bufferOffset = _serializer.bool(obj.is_checkresult_ok, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type CheckResult
    let len;
    let data = new CheckResult(null);
    // Deserialize message field [is_checkresult_ok]
    data.is_checkresult_ok = _deserializer.bool(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    return 1;
  }

  static datatype() {
    // Returns string type for a message object
    return 'mower_msgs/CheckResult';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'd65d3e1f019288ca453b865d8cb24468';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    bool is_checkresult_ok
    
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new CheckResult(null);
    if (msg.is_checkresult_ok !== undefined) {
      resolved.is_checkresult_ok = msg.is_checkresult_ok;
    }
    else {
      resolved.is_checkresult_ok = false
    }

    return resolved;
    }
};

module.exports = CheckResult;
