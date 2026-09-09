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

class VslamState {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.is_vslam_ok = null;
    }
    else {
      if (initObj.hasOwnProperty('is_vslam_ok')) {
        this.is_vslam_ok = initObj.is_vslam_ok
      }
      else {
        this.is_vslam_ok = false;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type VslamState
    // Serialize message field [is_vslam_ok]
    bufferOffset = _serializer.bool(obj.is_vslam_ok, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type VslamState
    let len;
    let data = new VslamState(null);
    // Deserialize message field [is_vslam_ok]
    data.is_vslam_ok = _deserializer.bool(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    return 1;
  }

  static datatype() {
    // Returns string type for a message object
    return 'mower_msgs/VslamState';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'd58db3b3d9624dbcc385f025b40a612c';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    bool is_vslam_ok
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new VslamState(null);
    if (msg.is_vslam_ok !== undefined) {
      resolved.is_vslam_ok = msg.is_vslam_ok;
    }
    else {
      resolved.is_vslam_ok = false
    }

    return resolved;
    }
};

module.exports = VslamState;
