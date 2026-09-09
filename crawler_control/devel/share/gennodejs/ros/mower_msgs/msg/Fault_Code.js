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

class Fault_Code {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.fault_code = null;
    }
    else {
      if (initObj.hasOwnProperty('fault_code')) {
        this.fault_code = initObj.fault_code
      }
      else {
        this.fault_code = new Array(13).fill(0);
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type Fault_Code
    // Check that the constant length array field [fault_code] has the right length
    if (obj.fault_code.length !== 13) {
      throw new Error('Unable to serialize array field fault_code - length must be 13')
    }
    // Serialize message field [fault_code]
    bufferOffset = _arraySerializer.string(obj.fault_code, buffer, bufferOffset, 13);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type Fault_Code
    let len;
    let data = new Fault_Code(null);
    // Deserialize message field [fault_code]
    data.fault_code = _arrayDeserializer.string(buffer, bufferOffset, 13)
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    object.fault_code.forEach((val) => {
      length += 4 + _getByteLength(val);
    });
    return length;
  }

  static datatype() {
    // Returns string type for a message object
    return 'mower_msgs/Fault_Code';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '42556c67ec9fdf90479d909abb5c32a1';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    string[13] fault_code
    
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new Fault_Code(null);
    if (msg.fault_code !== undefined) {
      resolved.fault_code = msg.fault_code;
    }
    else {
      resolved.fault_code = new Array(13).fill(0)
    }

    return resolved;
    }
};

module.exports = Fault_Code;
