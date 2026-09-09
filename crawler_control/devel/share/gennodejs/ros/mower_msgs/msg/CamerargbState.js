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

class CamerargbState {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.is_camerargb_ok = null;
    }
    else {
      if (initObj.hasOwnProperty('is_camerargb_ok')) {
        this.is_camerargb_ok = initObj.is_camerargb_ok
      }
      else {
        this.is_camerargb_ok = false;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type CamerargbState
    // Serialize message field [is_camerargb_ok]
    bufferOffset = _serializer.bool(obj.is_camerargb_ok, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type CamerargbState
    let len;
    let data = new CamerargbState(null);
    // Deserialize message field [is_camerargb_ok]
    data.is_camerargb_ok = _deserializer.bool(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    return 1;
  }

  static datatype() {
    // Returns string type for a message object
    return 'mower_msgs/CamerargbState';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '8721ee57cc9045b375b087d6e5ab5399';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    bool is_camerargb_ok
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new CamerargbState(null);
    if (msg.is_camerargb_ok !== undefined) {
      resolved.is_camerargb_ok = msg.is_camerargb_ok;
    }
    else {
      resolved.is_camerargb_ok = false
    }

    return resolved;
    }
};

module.exports = CamerargbState;
