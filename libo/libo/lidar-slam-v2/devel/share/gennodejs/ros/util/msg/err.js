// Auto-generated. Do not edit!

// (in-package util.msg)


"use strict";

const _serializer = _ros_msg_utils.Serialize;
const _arraySerializer = _serializer.Array;
const _deserializer = _ros_msg_utils.Deserialize;
const _arrayDeserializer = _deserializer.Array;
const _finder = _ros_msg_utils.Find;
const _getByteLength = _ros_msg_utils.getByteLength;
let std_msgs = _finder('std_msgs');

//-----------------------------------------------------------

class err {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.header = null;
      this.ant_e = null;
      this.ant_n = null;
      this.ant_u = null;
      this.ant_yaw = null;
      this.dual_yaw = null;
      this.dual_e = null;
      this.dual_n = null;
      this.dual_u = null;
      this.slam_x = null;
      this.slam_y = null;
      this.err_e = null;
      this.err_n = null;
      this.err_u = null;
      this.err_yaw = null;
      this.err_slame = null;
      this.err_slamn = null;
      this.err_slamu = null;
      this.err_slamyaw = null;
    }
    else {
      if (initObj.hasOwnProperty('header')) {
        this.header = initObj.header
      }
      else {
        this.header = new std_msgs.msg.Header();
      }
      if (initObj.hasOwnProperty('ant_e')) {
        this.ant_e = initObj.ant_e
      }
      else {
        this.ant_e = 0.0;
      }
      if (initObj.hasOwnProperty('ant_n')) {
        this.ant_n = initObj.ant_n
      }
      else {
        this.ant_n = 0.0;
      }
      if (initObj.hasOwnProperty('ant_u')) {
        this.ant_u = initObj.ant_u
      }
      else {
        this.ant_u = 0.0;
      }
      if (initObj.hasOwnProperty('ant_yaw')) {
        this.ant_yaw = initObj.ant_yaw
      }
      else {
        this.ant_yaw = 0.0;
      }
      if (initObj.hasOwnProperty('dual_yaw')) {
        this.dual_yaw = initObj.dual_yaw
      }
      else {
        this.dual_yaw = 0.0;
      }
      if (initObj.hasOwnProperty('dual_e')) {
        this.dual_e = initObj.dual_e
      }
      else {
        this.dual_e = 0.0;
      }
      if (initObj.hasOwnProperty('dual_n')) {
        this.dual_n = initObj.dual_n
      }
      else {
        this.dual_n = 0.0;
      }
      if (initObj.hasOwnProperty('dual_u')) {
        this.dual_u = initObj.dual_u
      }
      else {
        this.dual_u = 0.0;
      }
      if (initObj.hasOwnProperty('slam_x')) {
        this.slam_x = initObj.slam_x
      }
      else {
        this.slam_x = 0.0;
      }
      if (initObj.hasOwnProperty('slam_y')) {
        this.slam_y = initObj.slam_y
      }
      else {
        this.slam_y = 0.0;
      }
      if (initObj.hasOwnProperty('err_e')) {
        this.err_e = initObj.err_e
      }
      else {
        this.err_e = 0.0;
      }
      if (initObj.hasOwnProperty('err_n')) {
        this.err_n = initObj.err_n
      }
      else {
        this.err_n = 0.0;
      }
      if (initObj.hasOwnProperty('err_u')) {
        this.err_u = initObj.err_u
      }
      else {
        this.err_u = 0.0;
      }
      if (initObj.hasOwnProperty('err_yaw')) {
        this.err_yaw = initObj.err_yaw
      }
      else {
        this.err_yaw = 0.0;
      }
      if (initObj.hasOwnProperty('err_slame')) {
        this.err_slame = initObj.err_slame
      }
      else {
        this.err_slame = 0.0;
      }
      if (initObj.hasOwnProperty('err_slamn')) {
        this.err_slamn = initObj.err_slamn
      }
      else {
        this.err_slamn = 0.0;
      }
      if (initObj.hasOwnProperty('err_slamu')) {
        this.err_slamu = initObj.err_slamu
      }
      else {
        this.err_slamu = 0.0;
      }
      if (initObj.hasOwnProperty('err_slamyaw')) {
        this.err_slamyaw = initObj.err_slamyaw
      }
      else {
        this.err_slamyaw = 0.0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type err
    // Serialize message field [header]
    bufferOffset = std_msgs.msg.Header.serialize(obj.header, buffer, bufferOffset);
    // Serialize message field [ant_e]
    bufferOffset = _serializer.float64(obj.ant_e, buffer, bufferOffset);
    // Serialize message field [ant_n]
    bufferOffset = _serializer.float64(obj.ant_n, buffer, bufferOffset);
    // Serialize message field [ant_u]
    bufferOffset = _serializer.float64(obj.ant_u, buffer, bufferOffset);
    // Serialize message field [ant_yaw]
    bufferOffset = _serializer.float64(obj.ant_yaw, buffer, bufferOffset);
    // Serialize message field [dual_yaw]
    bufferOffset = _serializer.float64(obj.dual_yaw, buffer, bufferOffset);
    // Serialize message field [dual_e]
    bufferOffset = _serializer.float64(obj.dual_e, buffer, bufferOffset);
    // Serialize message field [dual_n]
    bufferOffset = _serializer.float64(obj.dual_n, buffer, bufferOffset);
    // Serialize message field [dual_u]
    bufferOffset = _serializer.float64(obj.dual_u, buffer, bufferOffset);
    // Serialize message field [slam_x]
    bufferOffset = _serializer.float64(obj.slam_x, buffer, bufferOffset);
    // Serialize message field [slam_y]
    bufferOffset = _serializer.float64(obj.slam_y, buffer, bufferOffset);
    // Serialize message field [err_e]
    bufferOffset = _serializer.float64(obj.err_e, buffer, bufferOffset);
    // Serialize message field [err_n]
    bufferOffset = _serializer.float64(obj.err_n, buffer, bufferOffset);
    // Serialize message field [err_u]
    bufferOffset = _serializer.float64(obj.err_u, buffer, bufferOffset);
    // Serialize message field [err_yaw]
    bufferOffset = _serializer.float64(obj.err_yaw, buffer, bufferOffset);
    // Serialize message field [err_slame]
    bufferOffset = _serializer.float64(obj.err_slame, buffer, bufferOffset);
    // Serialize message field [err_slamn]
    bufferOffset = _serializer.float64(obj.err_slamn, buffer, bufferOffset);
    // Serialize message field [err_slamu]
    bufferOffset = _serializer.float64(obj.err_slamu, buffer, bufferOffset);
    // Serialize message field [err_slamyaw]
    bufferOffset = _serializer.float64(obj.err_slamyaw, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type err
    let len;
    let data = new err(null);
    // Deserialize message field [header]
    data.header = std_msgs.msg.Header.deserialize(buffer, bufferOffset);
    // Deserialize message field [ant_e]
    data.ant_e = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [ant_n]
    data.ant_n = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [ant_u]
    data.ant_u = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [ant_yaw]
    data.ant_yaw = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [dual_yaw]
    data.dual_yaw = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [dual_e]
    data.dual_e = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [dual_n]
    data.dual_n = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [dual_u]
    data.dual_u = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [slam_x]
    data.slam_x = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [slam_y]
    data.slam_y = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [err_e]
    data.err_e = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [err_n]
    data.err_n = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [err_u]
    data.err_u = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [err_yaw]
    data.err_yaw = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [err_slame]
    data.err_slame = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [err_slamn]
    data.err_slamn = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [err_slamu]
    data.err_slamu = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [err_slamyaw]
    data.err_slamyaw = _deserializer.float64(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += std_msgs.msg.Header.getMessageSize(object.header);
    return length + 144;
  }

  static datatype() {
    // Returns string type for a message object
    return 'util/err';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '1e17d8731f8e0158fd085ddb259f3fb3';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    Header header
    float64 ant_e
    float64 ant_n
    float64 ant_u
    float64 ant_yaw
    float64 dual_yaw
    float64 dual_e
    float64 dual_n
    float64 dual_u
    float64 slam_x
    float64 slam_y
    float64 err_e 
    float64 err_n
    float64 err_u
    float64 err_yaw
    float64 err_slame
    float64 err_slamn
    float64 err_slamu
    float64 err_slamyaw 
    ================================================================================
    MSG: std_msgs/Header
    # Standard metadata for higher-level stamped data types.
    # This is generally used to communicate timestamped data 
    # in a particular coordinate frame.
    # 
    # sequence ID: consecutively increasing ID 
    uint32 seq
    #Two-integer timestamp that is expressed as:
    # * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')
    # * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')
    # time-handling sugar is provided by the client library
    time stamp
    #Frame this data is associated with
    string frame_id
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new err(null);
    if (msg.header !== undefined) {
      resolved.header = std_msgs.msg.Header.Resolve(msg.header)
    }
    else {
      resolved.header = new std_msgs.msg.Header()
    }

    if (msg.ant_e !== undefined) {
      resolved.ant_e = msg.ant_e;
    }
    else {
      resolved.ant_e = 0.0
    }

    if (msg.ant_n !== undefined) {
      resolved.ant_n = msg.ant_n;
    }
    else {
      resolved.ant_n = 0.0
    }

    if (msg.ant_u !== undefined) {
      resolved.ant_u = msg.ant_u;
    }
    else {
      resolved.ant_u = 0.0
    }

    if (msg.ant_yaw !== undefined) {
      resolved.ant_yaw = msg.ant_yaw;
    }
    else {
      resolved.ant_yaw = 0.0
    }

    if (msg.dual_yaw !== undefined) {
      resolved.dual_yaw = msg.dual_yaw;
    }
    else {
      resolved.dual_yaw = 0.0
    }

    if (msg.dual_e !== undefined) {
      resolved.dual_e = msg.dual_e;
    }
    else {
      resolved.dual_e = 0.0
    }

    if (msg.dual_n !== undefined) {
      resolved.dual_n = msg.dual_n;
    }
    else {
      resolved.dual_n = 0.0
    }

    if (msg.dual_u !== undefined) {
      resolved.dual_u = msg.dual_u;
    }
    else {
      resolved.dual_u = 0.0
    }

    if (msg.slam_x !== undefined) {
      resolved.slam_x = msg.slam_x;
    }
    else {
      resolved.slam_x = 0.0
    }

    if (msg.slam_y !== undefined) {
      resolved.slam_y = msg.slam_y;
    }
    else {
      resolved.slam_y = 0.0
    }

    if (msg.err_e !== undefined) {
      resolved.err_e = msg.err_e;
    }
    else {
      resolved.err_e = 0.0
    }

    if (msg.err_n !== undefined) {
      resolved.err_n = msg.err_n;
    }
    else {
      resolved.err_n = 0.0
    }

    if (msg.err_u !== undefined) {
      resolved.err_u = msg.err_u;
    }
    else {
      resolved.err_u = 0.0
    }

    if (msg.err_yaw !== undefined) {
      resolved.err_yaw = msg.err_yaw;
    }
    else {
      resolved.err_yaw = 0.0
    }

    if (msg.err_slame !== undefined) {
      resolved.err_slame = msg.err_slame;
    }
    else {
      resolved.err_slame = 0.0
    }

    if (msg.err_slamn !== undefined) {
      resolved.err_slamn = msg.err_slamn;
    }
    else {
      resolved.err_slamn = 0.0
    }

    if (msg.err_slamu !== undefined) {
      resolved.err_slamu = msg.err_slamu;
    }
    else {
      resolved.err_slamu = 0.0
    }

    if (msg.err_slamyaw !== undefined) {
      resolved.err_slamyaw = msg.err_slamyaw;
    }
    else {
      resolved.err_slamyaw = 0.0
    }

    return resolved;
    }
};

module.exports = err;
