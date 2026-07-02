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

class Obstacle {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.header = null;
      this.local_x = null;
      this.local_y = null;
      this.global_x = null;
      this.global_y = null;
      this.x1 = null;
      this.y1 = null;
      this.x2 = null;
      this.y2 = null;
      this.x3 = null;
      this.y3 = null;
      this.x4 = null;
      this.y4 = null;
      this.is_dynamic = null;
    }
    else {
      if (initObj.hasOwnProperty('header')) {
        this.header = initObj.header
      }
      else {
        this.header = new std_msgs.msg.Header();
      }
      if (initObj.hasOwnProperty('local_x')) {
        this.local_x = initObj.local_x
      }
      else {
        this.local_x = 0.0;
      }
      if (initObj.hasOwnProperty('local_y')) {
        this.local_y = initObj.local_y
      }
      else {
        this.local_y = 0.0;
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
      if (initObj.hasOwnProperty('x1')) {
        this.x1 = initObj.x1
      }
      else {
        this.x1 = 0.0;
      }
      if (initObj.hasOwnProperty('y1')) {
        this.y1 = initObj.y1
      }
      else {
        this.y1 = 0.0;
      }
      if (initObj.hasOwnProperty('x2')) {
        this.x2 = initObj.x2
      }
      else {
        this.x2 = 0.0;
      }
      if (initObj.hasOwnProperty('y2')) {
        this.y2 = initObj.y2
      }
      else {
        this.y2 = 0.0;
      }
      if (initObj.hasOwnProperty('x3')) {
        this.x3 = initObj.x3
      }
      else {
        this.x3 = 0.0;
      }
      if (initObj.hasOwnProperty('y3')) {
        this.y3 = initObj.y3
      }
      else {
        this.y3 = 0.0;
      }
      if (initObj.hasOwnProperty('x4')) {
        this.x4 = initObj.x4
      }
      else {
        this.x4 = 0.0;
      }
      if (initObj.hasOwnProperty('y4')) {
        this.y4 = initObj.y4
      }
      else {
        this.y4 = 0.0;
      }
      if (initObj.hasOwnProperty('is_dynamic')) {
        this.is_dynamic = initObj.is_dynamic
      }
      else {
        this.is_dynamic = false;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type Obstacle
    // Serialize message field [header]
    bufferOffset = std_msgs.msg.Header.serialize(obj.header, buffer, bufferOffset);
    // Serialize message field [local_x]
    bufferOffset = _serializer.float64(obj.local_x, buffer, bufferOffset);
    // Serialize message field [local_y]
    bufferOffset = _serializer.float64(obj.local_y, buffer, bufferOffset);
    // Serialize message field [global_x]
    bufferOffset = _serializer.float64(obj.global_x, buffer, bufferOffset);
    // Serialize message field [global_y]
    bufferOffset = _serializer.float64(obj.global_y, buffer, bufferOffset);
    // Serialize message field [x1]
    bufferOffset = _serializer.float32(obj.x1, buffer, bufferOffset);
    // Serialize message field [y1]
    bufferOffset = _serializer.float32(obj.y1, buffer, bufferOffset);
    // Serialize message field [x2]
    bufferOffset = _serializer.float32(obj.x2, buffer, bufferOffset);
    // Serialize message field [y2]
    bufferOffset = _serializer.float32(obj.y2, buffer, bufferOffset);
    // Serialize message field [x3]
    bufferOffset = _serializer.float32(obj.x3, buffer, bufferOffset);
    // Serialize message field [y3]
    bufferOffset = _serializer.float32(obj.y3, buffer, bufferOffset);
    // Serialize message field [x4]
    bufferOffset = _serializer.float32(obj.x4, buffer, bufferOffset);
    // Serialize message field [y4]
    bufferOffset = _serializer.float32(obj.y4, buffer, bufferOffset);
    // Serialize message field [is_dynamic]
    bufferOffset = _serializer.bool(obj.is_dynamic, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type Obstacle
    let len;
    let data = new Obstacle(null);
    // Deserialize message field [header]
    data.header = std_msgs.msg.Header.deserialize(buffer, bufferOffset);
    // Deserialize message field [local_x]
    data.local_x = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [local_y]
    data.local_y = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [global_x]
    data.global_x = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [global_y]
    data.global_y = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [x1]
    data.x1 = _deserializer.float32(buffer, bufferOffset);
    // Deserialize message field [y1]
    data.y1 = _deserializer.float32(buffer, bufferOffset);
    // Deserialize message field [x2]
    data.x2 = _deserializer.float32(buffer, bufferOffset);
    // Deserialize message field [y2]
    data.y2 = _deserializer.float32(buffer, bufferOffset);
    // Deserialize message field [x3]
    data.x3 = _deserializer.float32(buffer, bufferOffset);
    // Deserialize message field [y3]
    data.y3 = _deserializer.float32(buffer, bufferOffset);
    // Deserialize message field [x4]
    data.x4 = _deserializer.float32(buffer, bufferOffset);
    // Deserialize message field [y4]
    data.y4 = _deserializer.float32(buffer, bufferOffset);
    // Deserialize message field [is_dynamic]
    data.is_dynamic = _deserializer.bool(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += std_msgs.msg.Header.getMessageSize(object.header);
    return length + 65;
  }

  static datatype() {
    // Returns string type for a message object
    return 'util/Obstacle';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '9cfcfb9d59a900ca50b3ef55c2eb450b';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    Header header
    
    # 2D Box
    float64 local_x # m, in map
    float64 local_y # m, in map
    float64 global_x
    float64 global_y
    float32 x1 # x_min
    float32 y1 # y_max
    float32 x2 # x_max
    float32 y2 # y_max
    float32 x3 # x_max
    float32 y3 # y_min
    float32 x4 # x_min
    float32 y4 # y_min
    bool is_dynamic # is_dynamic
    
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
    const resolved = new Obstacle(null);
    if (msg.header !== undefined) {
      resolved.header = std_msgs.msg.Header.Resolve(msg.header)
    }
    else {
      resolved.header = new std_msgs.msg.Header()
    }

    if (msg.local_x !== undefined) {
      resolved.local_x = msg.local_x;
    }
    else {
      resolved.local_x = 0.0
    }

    if (msg.local_y !== undefined) {
      resolved.local_y = msg.local_y;
    }
    else {
      resolved.local_y = 0.0
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

    if (msg.x1 !== undefined) {
      resolved.x1 = msg.x1;
    }
    else {
      resolved.x1 = 0.0
    }

    if (msg.y1 !== undefined) {
      resolved.y1 = msg.y1;
    }
    else {
      resolved.y1 = 0.0
    }

    if (msg.x2 !== undefined) {
      resolved.x2 = msg.x2;
    }
    else {
      resolved.x2 = 0.0
    }

    if (msg.y2 !== undefined) {
      resolved.y2 = msg.y2;
    }
    else {
      resolved.y2 = 0.0
    }

    if (msg.x3 !== undefined) {
      resolved.x3 = msg.x3;
    }
    else {
      resolved.x3 = 0.0
    }

    if (msg.y3 !== undefined) {
      resolved.y3 = msg.y3;
    }
    else {
      resolved.y3 = 0.0
    }

    if (msg.x4 !== undefined) {
      resolved.x4 = msg.x4;
    }
    else {
      resolved.x4 = 0.0
    }

    if (msg.y4 !== undefined) {
      resolved.y4 = msg.y4;
    }
    else {
      resolved.y4 = 0.0
    }

    if (msg.is_dynamic !== undefined) {
      resolved.is_dynamic = msg.is_dynamic;
    }
    else {
      resolved.is_dynamic = false
    }

    return resolved;
    }
};

module.exports = Obstacle;
