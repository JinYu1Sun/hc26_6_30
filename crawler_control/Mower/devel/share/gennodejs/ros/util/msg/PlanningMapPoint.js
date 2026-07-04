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

class PlanningMapPoint {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.header = null;
      this.global_x = null;
      this.global_y = null;
      this.obstacle_status = null;
      this.track_status = null;
    }
    else {
      if (initObj.hasOwnProperty('header')) {
        this.header = initObj.header
      }
      else {
        this.header = new std_msgs.msg.Header();
      }
      if (initObj.hasOwnProperty('global_x')) {
        this.global_x = initObj.global_x
      }
      else {
        this.global_x = 0.0;
      }
      if (initObj.hasOwnProperty('global_y')) {
        this.global_y = initObj.global_y
      }
      else {
        this.global_y = 0.0;
      }
      if (initObj.hasOwnProperty('obstacle_status')) {
        this.obstacle_status = initObj.obstacle_status
      }
      else {
        this.obstacle_status = false;
      }
      if (initObj.hasOwnProperty('track_status')) {
        this.track_status = initObj.track_status
      }
      else {
        this.track_status = false;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type PlanningMapPoint
    // Serialize message field [header]
    bufferOffset = std_msgs.msg.Header.serialize(obj.header, buffer, bufferOffset);
    // Serialize message field [global_x]
    bufferOffset = _serializer.float64(obj.global_x, buffer, bufferOffset);
    // Serialize message field [global_y]
    bufferOffset = _serializer.float64(obj.global_y, buffer, bufferOffset);
    // Serialize message field [obstacle_status]
    bufferOffset = _serializer.bool(obj.obstacle_status, buffer, bufferOffset);
    // Serialize message field [track_status]
    bufferOffset = _serializer.bool(obj.track_status, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type PlanningMapPoint
    let len;
    let data = new PlanningMapPoint(null);
    // Deserialize message field [header]
    data.header = std_msgs.msg.Header.deserialize(buffer, bufferOffset);
    // Deserialize message field [global_x]
    data.global_x = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [global_y]
    data.global_y = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [obstacle_status]
    data.obstacle_status = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [track_status]
    data.track_status = _deserializer.bool(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += std_msgs.msg.Header.getMessageSize(object.header);
    return length + 18;
  }

  static datatype() {
    // Returns string type for a message object
    return 'util/PlanningMapPoint';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'ba8804762a910657104542279037ab6a';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    Header header
    
    # Global_Map_Point
    float64 global_x
    float64 global_y
    bool obstacle_status
    bool track_status
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
    const resolved = new PlanningMapPoint(null);
    if (msg.header !== undefined) {
      resolved.header = std_msgs.msg.Header.Resolve(msg.header)
    }
    else {
      resolved.header = new std_msgs.msg.Header()
    }

    if (msg.global_x !== undefined) {
      resolved.global_x = msg.global_x;
    }
    else {
      resolved.global_x = 0.0
    }

    if (msg.global_y !== undefined) {
      resolved.global_y = msg.global_y;
    }
    else {
      resolved.global_y = 0.0
    }

    if (msg.obstacle_status !== undefined) {
      resolved.obstacle_status = msg.obstacle_status;
    }
    else {
      resolved.obstacle_status = false
    }

    if (msg.track_status !== undefined) {
      resolved.track_status = msg.track_status;
    }
    else {
      resolved.track_status = false
    }

    return resolved;
    }
};

module.exports = PlanningMapPoint;
