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

class GnssOK {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.is_gnss_ok = null;
    }
    else {
      if (initObj.hasOwnProperty('is_gnss_ok')) {
        this.is_gnss_ok = initObj.is_gnss_ok
      }
      else {
        this.is_gnss_ok = false;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type GnssOK
    // Serialize message field [is_gnss_ok]
    bufferOffset = _serializer.bool(obj.is_gnss_ok, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type GnssOK
    let len;
    let data = new GnssOK(null);
    // Deserialize message field [is_gnss_ok]
    data.is_gnss_ok = _deserializer.bool(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    return 1;
  }

  static datatype() {
    // Returns string type for a message object
    return 'mower_msgs/GnssOK';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '928ba1b0bbd91610cb8761c3d298eeac';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    bool is_gnss_ok 
    
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new GnssOK(null);
    if (msg.is_gnss_ok !== undefined) {
      resolved.is_gnss_ok = msg.is_gnss_ok;
    }
    else {
      resolved.is_gnss_ok = false
    }

    return resolved;
    }
};

module.exports = GnssOK;
