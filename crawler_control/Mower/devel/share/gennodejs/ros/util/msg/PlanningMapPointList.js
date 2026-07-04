// Auto-generated. Do not edit!

// (in-package util.msg)


"use strict";

const _serializer = _ros_msg_utils.Serialize;
const _arraySerializer = _serializer.Array;
const _deserializer = _ros_msg_utils.Deserialize;
const _arrayDeserializer = _deserializer.Array;
const _finder = _ros_msg_utils.Find;
const _getByteLength = _ros_msg_utils.getByteLength;
let PlanningMapPoint = require('./PlanningMapPoint.js');
let std_msgs = _finder('std_msgs');

//-----------------------------------------------------------

class PlanningMapPointList {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.header = null;
      this.global_point_list = null;
    }
    else {
      if (initObj.hasOwnProperty('header')) {
        this.header = initObj.header
      }
      else {
        this.header = new std_msgs.msg.Header();
      }
      if (initObj.hasOwnProperty('global_point_list')) {
        this.global_point_list = initObj.global_point_list
      }
      else {
        this.global_point_list = [];
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type PlanningMapPointList
    // Serialize message field [header]
    bufferOffset = std_msgs.msg.Header.serialize(obj.header, buffer, bufferOffset);
    // Serialize message field [global_point_list]
    // Serialize the length for message field [global_point_list]
    bufferOffset = _serializer.uint32(obj.global_point_list.length, buffer, bufferOffset);
    obj.global_point_list.forEach((val) => {
      bufferOffset = PlanningMapPoint.serialize(val, buffer, bufferOffset);
    });
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type PlanningMapPointList
    let len;
    let data = new PlanningMapPointList(null);
    // Deserialize message field [header]
    data.header = std_msgs.msg.Header.deserialize(buffer, bufferOffset);
    // Deserialize message field [global_point_list]
    // Deserialize array length for message field [global_point_list]
    len = _deserializer.uint32(buffer, bufferOffset);
    data.global_point_list = new Array(len);
    for (let i = 0; i < len; ++i) {
      data.global_point_list[i] = PlanningMapPoint.deserialize(buffer, bufferOffset)
    }
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += std_msgs.msg.Header.getMessageSize(object.header);
    object.global_point_list.forEach((val) => {
      length += PlanningMapPoint.getMessageSize(val);
    });
    return length + 4;
  }

  static datatype() {
    // Returns string type for a message object
    return 'util/PlanningMapPointList';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'e74b8cb2e54b9905429e9e007420b366';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    Header header
    PlanningMapPoint[] global_point_list
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
    
    ================================================================================
    MSG: util/PlanningMapPoint
    Header header
    
    # Global_Map_Point
    float64 global_x
    float64 global_y
    bool obstacle_status
    bool track_status
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new PlanningMapPointList(null);
    if (msg.header !== undefined) {
      resolved.header = std_msgs.msg.Header.Resolve(msg.header)
    }
    else {
      resolved.header = new std_msgs.msg.Header()
    }

    if (msg.global_point_list !== undefined) {
      resolved.global_point_list = new Array(msg.global_point_list.length);
      for (let i = 0; i < resolved.global_point_list.length; ++i) {
        resolved.global_point_list[i] = PlanningMapPoint.Resolve(msg.global_point_list[i]);
      }
    }
    else {
      resolved.global_point_list = []
    }

    return resolved;
    }
};

module.exports = PlanningMapPointList;
