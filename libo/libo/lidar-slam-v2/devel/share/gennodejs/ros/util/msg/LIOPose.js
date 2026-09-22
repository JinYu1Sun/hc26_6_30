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

class LIOPose {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.header = null;
      this.position_x = null;
      this.position_y = null;
      this.position_z = null;
      this.q_w = null;
      this.q_x = null;
      this.q_y = null;
      this.q_z = null;
      this.lio_state = null;
      this.imu_speed = null;
      this.imt_static = null;
    }
    else {
      if (initObj.hasOwnProperty('header')) {
        this.header = initObj.header
      }
      else {
        this.header = new std_msgs.msg.Header();
      }
      if (initObj.hasOwnProperty('position_x')) {
        this.position_x = initObj.position_x
      }
      else {
        this.position_x = 0.0;
      }
      if (initObj.hasOwnProperty('position_y')) {
        this.position_y = initObj.position_y
      }
      else {
        this.position_y = 0.0;
      }
      if (initObj.hasOwnProperty('position_z')) {
        this.position_z = initObj.position_z
      }
      else {
        this.position_z = 0.0;
      }
      if (initObj.hasOwnProperty('q_w')) {
        this.q_w = initObj.q_w
      }
      else {
        this.q_w = 0.0;
      }
      if (initObj.hasOwnProperty('q_x')) {
        this.q_x = initObj.q_x
      }
      else {
        this.q_x = 0.0;
      }
      if (initObj.hasOwnProperty('q_y')) {
        this.q_y = initObj.q_y
      }
      else {
        this.q_y = 0.0;
      }
      if (initObj.hasOwnProperty('q_z')) {
        this.q_z = initObj.q_z
      }
      else {
        this.q_z = 0.0;
      }
      if (initObj.hasOwnProperty('lio_state')) {
        this.lio_state = initObj.lio_state
      }
      else {
        this.lio_state = false;
      }
      if (initObj.hasOwnProperty('imu_speed')) {
        this.imu_speed = initObj.imu_speed
      }
      else {
        this.imu_speed = 0.0;
      }
      if (initObj.hasOwnProperty('imt_static')) {
        this.imt_static = initObj.imt_static
      }
      else {
        this.imt_static = false;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type LIOPose
    // Serialize message field [header]
    bufferOffset = std_msgs.msg.Header.serialize(obj.header, buffer, bufferOffset);
    // Serialize message field [position_x]
    bufferOffset = _serializer.float64(obj.position_x, buffer, bufferOffset);
    // Serialize message field [position_y]
    bufferOffset = _serializer.float64(obj.position_y, buffer, bufferOffset);
    // Serialize message field [position_z]
    bufferOffset = _serializer.float64(obj.position_z, buffer, bufferOffset);
    // Serialize message field [q_w]
    bufferOffset = _serializer.float64(obj.q_w, buffer, bufferOffset);
    // Serialize message field [q_x]
    bufferOffset = _serializer.float64(obj.q_x, buffer, bufferOffset);
    // Serialize message field [q_y]
    bufferOffset = _serializer.float64(obj.q_y, buffer, bufferOffset);
    // Serialize message field [q_z]
    bufferOffset = _serializer.float64(obj.q_z, buffer, bufferOffset);
    // Serialize message field [lio_state]
    bufferOffset = _serializer.bool(obj.lio_state, buffer, bufferOffset);
    // Serialize message field [imu_speed]
    bufferOffset = _serializer.float64(obj.imu_speed, buffer, bufferOffset);
    // Serialize message field [imt_static]
    bufferOffset = _serializer.bool(obj.imt_static, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type LIOPose
    let len;
    let data = new LIOPose(null);
    // Deserialize message field [header]
    data.header = std_msgs.msg.Header.deserialize(buffer, bufferOffset);
    // Deserialize message field [position_x]
    data.position_x = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [position_y]
    data.position_y = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [position_z]
    data.position_z = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [q_w]
    data.q_w = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [q_x]
    data.q_x = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [q_y]
    data.q_y = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [q_z]
    data.q_z = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [lio_state]
    data.lio_state = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [imu_speed]
    data.imu_speed = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [imt_static]
    data.imt_static = _deserializer.bool(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += std_msgs.msg.Header.getMessageSize(object.header);
    return length + 66;
  }

  static datatype() {
    // Returns string type for a message object
    return 'util/LIOPose';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'cc6fba92d9eec750f98200ca1300a109';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    # Lidar slam msg
    Header header
    float64 position_x
    float64 position_y
    float64 position_z
    float64 q_w
    float64 q_x
    float64 q_y
    float64 q_z
    bool  lio_state
    float64 imu_speed
    bool imt_static
    
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
    const resolved = new LIOPose(null);
    if (msg.header !== undefined) {
      resolved.header = std_msgs.msg.Header.Resolve(msg.header)
    }
    else {
      resolved.header = new std_msgs.msg.Header()
    }

    if (msg.position_x !== undefined) {
      resolved.position_x = msg.position_x;
    }
    else {
      resolved.position_x = 0.0
    }

    if (msg.position_y !== undefined) {
      resolved.position_y = msg.position_y;
    }
    else {
      resolved.position_y = 0.0
    }

    if (msg.position_z !== undefined) {
      resolved.position_z = msg.position_z;
    }
    else {
      resolved.position_z = 0.0
    }

    if (msg.q_w !== undefined) {
      resolved.q_w = msg.q_w;
    }
    else {
      resolved.q_w = 0.0
    }

    if (msg.q_x !== undefined) {
      resolved.q_x = msg.q_x;
    }
    else {
      resolved.q_x = 0.0
    }

    if (msg.q_y !== undefined) {
      resolved.q_y = msg.q_y;
    }
    else {
      resolved.q_y = 0.0
    }

    if (msg.q_z !== undefined) {
      resolved.q_z = msg.q_z;
    }
    else {
      resolved.q_z = 0.0
    }

    if (msg.lio_state !== undefined) {
      resolved.lio_state = msg.lio_state;
    }
    else {
      resolved.lio_state = false
    }

    if (msg.imu_speed !== undefined) {
      resolved.imu_speed = msg.imu_speed;
    }
    else {
      resolved.imu_speed = 0.0
    }

    if (msg.imt_static !== undefined) {
      resolved.imt_static = msg.imt_static;
    }
    else {
      resolved.imt_static = false
    }

    return resolved;
    }
};

module.exports = LIOPose;
