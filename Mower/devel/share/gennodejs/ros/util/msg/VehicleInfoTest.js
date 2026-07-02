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

class VehicleInfoTest {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.header = null;
      this.speed_status = null;
      this.angle_status = null;
    }
    else {
      if (initObj.hasOwnProperty('header')) {
        this.header = initObj.header
      }
      else {
        this.header = new std_msgs.msg.Header();
      }
      if (initObj.hasOwnProperty('speed_status')) {
        this.speed_status = initObj.speed_status
      }
      else {
        this.speed_status = 0.0;
      }
      if (initObj.hasOwnProperty('angle_status')) {
        this.angle_status = initObj.angle_status
      }
      else {
        this.angle_status = 0.0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type VehicleInfoTest
    // Serialize message field [header]
    bufferOffset = std_msgs.msg.Header.serialize(obj.header, buffer, bufferOffset);
    // Serialize message field [speed_status]
    bufferOffset = _serializer.float64(obj.speed_status, buffer, bufferOffset);
    // Serialize message field [angle_status]
    bufferOffset = _serializer.float64(obj.angle_status, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type VehicleInfoTest
    let len;
    let data = new VehicleInfoTest(null);
    // Deserialize message field [header]
    data.header = std_msgs.msg.Header.deserialize(buffer, bufferOffset);
    // Deserialize message field [speed_status]
    data.speed_status = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [angle_status]
    data.angle_status = _deserializer.float64(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += std_msgs.msg.Header.getMessageSize(object.header);
    return length + 16;
  }

  static datatype() {
    // Returns string type for a message object
    return 'util/VehicleInfoTest';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '9c82651e7ec09ce256b0ec03b6f2bab7';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    Header header
    
    # AGVStatus 0x201
    float64 speed_status
    float64 angle_status
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
    const resolved = new VehicleInfoTest(null);
    if (msg.header !== undefined) {
      resolved.header = std_msgs.msg.Header.Resolve(msg.header)
    }
    else {
      resolved.header = new std_msgs.msg.Header()
    }

    if (msg.speed_status !== undefined) {
      resolved.speed_status = msg.speed_status;
    }
    else {
      resolved.speed_status = 0.0
    }

    if (msg.angle_status !== undefined) {
      resolved.angle_status = msg.angle_status;
    }
    else {
      resolved.angle_status = 0.0
    }

    return resolved;
    }
};

module.exports = VehicleInfoTest;
