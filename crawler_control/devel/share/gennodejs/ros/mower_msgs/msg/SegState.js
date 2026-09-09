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

class SegState {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.is_seg_ok = null;
    }
    else {
      if (initObj.hasOwnProperty('is_seg_ok')) {
        this.is_seg_ok = initObj.is_seg_ok
      }
      else {
        this.is_seg_ok = false;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type SegState
    // Serialize message field [is_seg_ok]
    bufferOffset = _serializer.bool(obj.is_seg_ok, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type SegState
    let len;
    let data = new SegState(null);
    // Deserialize message field [is_seg_ok]
    data.is_seg_ok = _deserializer.bool(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    return 1;
  }

  static datatype() {
    // Returns string type for a message object
    return 'mower_msgs/SegState';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '2a12252519f1037241c6783756d61d0f';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    bool is_seg_ok
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new SegState(null);
    if (msg.is_seg_ok !== undefined) {
      resolved.is_seg_ok = msg.is_seg_ok;
    }
    else {
      resolved.is_seg_ok = false
    }

    return resolved;
    }
};

module.exports = SegState;
