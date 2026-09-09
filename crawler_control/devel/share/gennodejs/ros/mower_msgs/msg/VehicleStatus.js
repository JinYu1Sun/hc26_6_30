// Auto-generated. Do not edit!

// (in-package mower_msgs.msg)


"use strict";

const _serializer = _ros_msg_utils.Serialize;
const _arraySerializer = _serializer.Array;
const _deserializer = _ros_msg_utils.Deserialize;
const _arrayDeserializer = _deserializer.Array;
const _finder = _ros_msg_utils.Find;
const _getByteLength = _ros_msg_utils.getByteLength;
let std_msgs = _finder('std_msgs');

//-----------------------------------------------------------

class VehicleStatus {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.header = null;
      this.battery_soc = null;
      this.warning_state_one = null;
      this.warning_state_two = null;
    }
    else {
      if (initObj.hasOwnProperty('header')) {
        this.header = initObj.header
      }
      else {
        this.header = new std_msgs.msg.Header();
      }
      if (initObj.hasOwnProperty('battery_soc')) {
        this.battery_soc = initObj.battery_soc
      }
      else {
        this.battery_soc = 0;
      }
      if (initObj.hasOwnProperty('warning_state_one')) {
        this.warning_state_one = initObj.warning_state_one
      }
      else {
        this.warning_state_one = 0;
      }
      if (initObj.hasOwnProperty('warning_state_two')) {
        this.warning_state_two = initObj.warning_state_two
      }
      else {
        this.warning_state_two = 0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type VehicleStatus
    // Serialize message field [header]
    bufferOffset = std_msgs.msg.Header.serialize(obj.header, buffer, bufferOffset);
    // Serialize message field [battery_soc]
    bufferOffset = _serializer.uint8(obj.battery_soc, buffer, bufferOffset);
    // Serialize message field [warning_state_one]
    bufferOffset = _serializer.uint8(obj.warning_state_one, buffer, bufferOffset);
    // Serialize message field [warning_state_two]
    bufferOffset = _serializer.uint8(obj.warning_state_two, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type VehicleStatus
    let len;
    let data = new VehicleStatus(null);
    // Deserialize message field [header]
    data.header = std_msgs.msg.Header.deserialize(buffer, bufferOffset);
    // Deserialize message field [battery_soc]
    data.battery_soc = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [warning_state_one]
    data.warning_state_one = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [warning_state_two]
    data.warning_state_two = _deserializer.uint8(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += std_msgs.msg.Header.getMessageSize(object.header);
    return length + 3;
  }

  static datatype() {
    // Returns string type for a message object
    return 'mower_msgs/VehicleStatus';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '7d07e9e512dc3175789403cb7342112b';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    Header header
    uint8 battery_soc
    uint8 warning_state_one
    uint8 warning_state_two
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
    const resolved = new VehicleStatus(null);
    if (msg.header !== undefined) {
      resolved.header = std_msgs.msg.Header.Resolve(msg.header)
    }
    else {
      resolved.header = new std_msgs.msg.Header()
    }

    if (msg.battery_soc !== undefined) {
      resolved.battery_soc = msg.battery_soc;
    }
    else {
      resolved.battery_soc = 0
    }

    if (msg.warning_state_one !== undefined) {
      resolved.warning_state_one = msg.warning_state_one;
    }
    else {
      resolved.warning_state_one = 0
    }

    if (msg.warning_state_two !== undefined) {
      resolved.warning_state_two = msg.warning_state_two;
    }
    else {
      resolved.warning_state_two = 0
    }

    return resolved;
    }
};

module.exports = VehicleStatus;
