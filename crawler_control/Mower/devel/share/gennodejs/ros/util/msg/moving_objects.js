// Auto-generated. Do not edit!

// (in-package util.msg)


"use strict";

const _serializer = _ros_msg_utils.Serialize;
const _arraySerializer = _serializer.Array;
const _deserializer = _ros_msg_utils.Deserialize;
const _arrayDeserializer = _deserializer.Array;
const _finder = _ros_msg_utils.Find;
const _getByteLength = _ros_msg_utils.getByteLength;
let moving_object = require('./moving_object.js');
let std_msgs = _finder('std_msgs');

//-----------------------------------------------------------

class moving_objects {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.header = null;
      this.num = null;
      this.moving_obj = null;
    }
    else {
      if (initObj.hasOwnProperty('header')) {
        this.header = initObj.header
      }
      else {
        this.header = new std_msgs.msg.Header();
      }
      if (initObj.hasOwnProperty('num')) {
        this.num = initObj.num
      }
      else {
        this.num = 0;
      }
      if (initObj.hasOwnProperty('moving_obj')) {
        this.moving_obj = initObj.moving_obj
      }
      else {
        this.moving_obj = [];
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type moving_objects
    // Serialize message field [header]
    bufferOffset = std_msgs.msg.Header.serialize(obj.header, buffer, bufferOffset);
    // Serialize message field [num]
    bufferOffset = _serializer.int32(obj.num, buffer, bufferOffset);
    // Serialize message field [moving_obj]
    // Serialize the length for message field [moving_obj]
    bufferOffset = _serializer.uint32(obj.moving_obj.length, buffer, bufferOffset);
    obj.moving_obj.forEach((val) => {
      bufferOffset = moving_object.serialize(val, buffer, bufferOffset);
    });
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type moving_objects
    let len;
    let data = new moving_objects(null);
    // Deserialize message field [header]
    data.header = std_msgs.msg.Header.deserialize(buffer, bufferOffset);
    // Deserialize message field [num]
    data.num = _deserializer.int32(buffer, bufferOffset);
    // Deserialize message field [moving_obj]
    // Deserialize array length for message field [moving_obj]
    len = _deserializer.uint32(buffer, bufferOffset);
    data.moving_obj = new Array(len);
    for (let i = 0; i < len; ++i) {
      data.moving_obj[i] = moving_object.deserialize(buffer, bufferOffset)
    }
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += std_msgs.msg.Header.getMessageSize(object.header);
    object.moving_obj.forEach((val) => {
      length += moving_object.getMessageSize(val);
    });
    return length + 8;
  }

  static datatype() {
    // Returns string type for a message object
    return 'util/moving_objects';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '3c433c9a0e5b7d198518fcd1e6805968';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    Header header
    int32 num
    moving_object[] moving_obj
    
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
    MSG: util/moving_object
    #relative position, m
    float32 x1 # x_min
    float32 y1 # y_min
    float32 x2 # x_min
    float32 y2 # y_max
    float32 x3 # x_max
    float32 y3 # y_max
    float32 x4 # x_max
    float32 y4 # y_min
    
    # height, cm
    float32 zmin
    
    float32 zmax
    
    # absolute speed, m/s
    float32 speed
    
    # deg
    float32 speed_direction
    
    uint32 id
    string label
    
    # center point coordinate in localpose frame, m
    float32 global_x
    float32 global_y
    
    float32 local_x
    float32 local_y
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new moving_objects(null);
    if (msg.header !== undefined) {
      resolved.header = std_msgs.msg.Header.Resolve(msg.header)
    }
    else {
      resolved.header = new std_msgs.msg.Header()
    }

    if (msg.num !== undefined) {
      resolved.num = msg.num;
    }
    else {
      resolved.num = 0
    }

    if (msg.moving_obj !== undefined) {
      resolved.moving_obj = new Array(msg.moving_obj.length);
      for (let i = 0; i < resolved.moving_obj.length; ++i) {
        resolved.moving_obj[i] = moving_object.Resolve(msg.moving_obj[i]);
      }
    }
    else {
      resolved.moving_obj = []
    }

    return resolved;
    }
};

module.exports = moving_objects;
