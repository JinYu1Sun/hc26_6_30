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

class CameraState {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.is_camera_ok = null;
    }
    else {
      if (initObj.hasOwnProperty('is_camera_ok')) {
        this.is_camera_ok = initObj.is_camera_ok
      }
      else {
        this.is_camera_ok = false;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type CameraState
    // Serialize message field [is_camera_ok]
    bufferOffset = _serializer.bool(obj.is_camera_ok, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type CameraState
    let len;
    let data = new CameraState(null);
    // Deserialize message field [is_camera_ok]
    data.is_camera_ok = _deserializer.bool(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    return 1;
  }

  static datatype() {
    // Returns string type for a message object
    return 'mower_msgs/CameraState';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '56198635b7732d260a91b05411870558';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    bool is_camera_ok
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new CameraState(null);
    if (msg.is_camera_ok !== undefined) {
      resolved.is_camera_ok = msg.is_camera_ok;
    }
    else {
      resolved.is_camera_ok = false
    }

    return resolved;
    }
};

module.exports = CameraState;
