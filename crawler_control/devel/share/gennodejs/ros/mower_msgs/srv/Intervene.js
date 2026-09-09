// Auto-generated. Do not edit!

// (in-package mower_msgs.srv)


"use strict";

const _serializer = _ros_msg_utils.Serialize;
const _arraySerializer = _serializer.Array;
const _deserializer = _ros_msg_utils.Deserialize;
const _arrayDeserializer = _deserializer.Array;
const _finder = _ros_msg_utils.Find;
const _getByteLength = _ros_msg_utils.getByteLength;

//-----------------------------------------------------------


//-----------------------------------------------------------

class InterveneRequest {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.node_id = null;
      this.node_status = null;
      this.msg = null;
    }
    else {
      if (initObj.hasOwnProperty('node_id')) {
        this.node_id = initObj.node_id
      }
      else {
        this.node_id = 0;
      }
      if (initObj.hasOwnProperty('node_status')) {
        this.node_status = initObj.node_status
      }
      else {
        this.node_status = 0;
      }
      if (initObj.hasOwnProperty('msg')) {
        this.msg = initObj.msg
      }
      else {
        this.msg = '';
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type InterveneRequest
    // Serialize message field [node_id]
    bufferOffset = _serializer.int32(obj.node_id, buffer, bufferOffset);
    // Serialize message field [node_status]
    bufferOffset = _serializer.int32(obj.node_status, buffer, bufferOffset);
    // Serialize message field [msg]
    bufferOffset = _serializer.string(obj.msg, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type InterveneRequest
    let len;
    let data = new InterveneRequest(null);
    // Deserialize message field [node_id]
    data.node_id = _deserializer.int32(buffer, bufferOffset);
    // Deserialize message field [node_status]
    data.node_status = _deserializer.int32(buffer, bufferOffset);
    // Deserialize message field [msg]
    data.msg = _deserializer.string(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += _getByteLength(object.msg);
    return length + 12;
  }

  static datatype() {
    // Returns string type for a service object
    return 'mower_msgs/InterveneRequest';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '9ecca3ff4c064eed4df95f8ca41820d4';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    int32  node_id
    int32  node_status
    string msg
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new InterveneRequest(null);
    if (msg.node_id !== undefined) {
      resolved.node_id = msg.node_id;
    }
    else {
      resolved.node_id = 0
    }

    if (msg.node_status !== undefined) {
      resolved.node_status = msg.node_status;
    }
    else {
      resolved.node_status = 0
    }

    if (msg.msg !== undefined) {
      resolved.msg = msg.msg;
    }
    else {
      resolved.msg = ''
    }

    return resolved;
    }
};

class InterveneResponse {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.node_event = null;
    }
    else {
      if (initObj.hasOwnProperty('node_event')) {
        this.node_event = initObj.node_event
      }
      else {
        this.node_event = 0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type InterveneResponse
    // Serialize message field [node_event]
    bufferOffset = _serializer.int32(obj.node_event, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type InterveneResponse
    let len;
    let data = new InterveneResponse(null);
    // Deserialize message field [node_event]
    data.node_event = _deserializer.int32(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    return 4;
  }

  static datatype() {
    // Returns string type for a service object
    return 'mower_msgs/InterveneResponse';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'a2e1c3ce22be5a404a01a0373710b326';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    int32  node_event
    
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new InterveneResponse(null);
    if (msg.node_event !== undefined) {
      resolved.node_event = msg.node_event;
    }
    else {
      resolved.node_event = 0
    }

    return resolved;
    }
};

module.exports = {
  Request: InterveneRequest,
  Response: InterveneResponse,
  md5sum() { return '15f1f9695525704904993d3c41cc8ed1'; },
  datatype() { return 'mower_msgs/Intervene'; }
};
