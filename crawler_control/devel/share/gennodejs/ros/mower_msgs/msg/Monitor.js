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

class Monitor {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.node_normal = null;
    }
    else {
      if (initObj.hasOwnProperty('node_normal')) {
        this.node_normal = initObj.node_normal
      }
      else {
        this.node_normal = new Array(13).fill(0);
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type Monitor
    // Check that the constant length array field [node_normal] has the right length
    if (obj.node_normal.length !== 13) {
      throw new Error('Unable to serialize array field node_normal - length must be 13')
    }
    // Serialize message field [node_normal]
    bufferOffset = _arraySerializer.bool(obj.node_normal, buffer, bufferOffset, 13);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type Monitor
    let len;
    let data = new Monitor(null);
    // Deserialize message field [node_normal]
    data.node_normal = _arrayDeserializer.bool(buffer, bufferOffset, 13)
    return data;
  }

  static getMessageSize(object) {
    return 13;
  }

  static datatype() {
    // Returns string type for a message object
    return 'mower_msgs/Monitor';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '8fa716a375e1d424b9eff8c8c0b57ef5';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    bool[13] node_normal
    
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new Monitor(null);
    if (msg.node_normal !== undefined) {
      resolved.node_normal = msg.node_normal;
    }
    else {
      resolved.node_normal = new Array(13).fill(0)
    }

    return resolved;
    }
};

module.exports = Monitor;
