// Auto-generated. Do not edit!

// (in-package read_waypoints.srv)


"use strict";

const _serializer = _ros_msg_utils.Serialize;
const _arraySerializer = _serializer.Array;
const _deserializer = _ros_msg_utils.Deserialize;
const _arrayDeserializer = _deserializer.Array;
const _finder = _ros_msg_utils.Find;
const _getByteLength = _ros_msg_utils.getByteLength;

//-----------------------------------------------------------


//-----------------------------------------------------------

class TurnCompletedRequest {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.x = null;
      this.y = null;
    }
    else {
      if (initObj.hasOwnProperty('x')) {
        this.x = initObj.x
      }
      else {
        this.x = 0.0;
      }
      if (initObj.hasOwnProperty('y')) {
        this.y = initObj.y
      }
      else {
        this.y = 0.0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type TurnCompletedRequest
    // Serialize message field [x]
    bufferOffset = _serializer.float64(obj.x, buffer, bufferOffset);
    // Serialize message field [y]
    bufferOffset = _serializer.float64(obj.y, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type TurnCompletedRequest
    let len;
    let data = new TurnCompletedRequest(null);
    // Deserialize message field [x]
    data.x = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [y]
    data.y = _deserializer.float64(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    return 16;
  }

  static datatype() {
    // Returns string type for a service object
    return 'read_waypoints/TurnCompletedRequest';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '209f516d3eb691f0663e25cb750d67c1';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    float64 x
    float64 y
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new TurnCompletedRequest(null);
    if (msg.x !== undefined) {
      resolved.x = msg.x;
    }
    else {
      resolved.x = 0.0
    }

    if (msg.y !== undefined) {
      resolved.y = msg.y;
    }
    else {
      resolved.y = 0.0
    }

    return resolved;
    }
};

class TurnCompletedResponse {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.turn_finish = null;
    }
    else {
      if (initObj.hasOwnProperty('turn_finish')) {
        this.turn_finish = initObj.turn_finish
      }
      else {
        this.turn_finish = false;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type TurnCompletedResponse
    // Serialize message field [turn_finish]
    bufferOffset = _serializer.bool(obj.turn_finish, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type TurnCompletedResponse
    let len;
    let data = new TurnCompletedResponse(null);
    // Deserialize message field [turn_finish]
    data.turn_finish = _deserializer.bool(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    return 1;
  }

  static datatype() {
    // Returns string type for a service object
    return 'read_waypoints/TurnCompletedResponse';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'dae0339b58aaeeee6587b8e576a925bb';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    bool turn_finish
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new TurnCompletedResponse(null);
    if (msg.turn_finish !== undefined) {
      resolved.turn_finish = msg.turn_finish;
    }
    else {
      resolved.turn_finish = false
    }

    return resolved;
    }
};

module.exports = {
  Request: TurnCompletedRequest,
  Response: TurnCompletedResponse,
  md5sum() { return '5677d53f5f233f7e8f08a8788d1eb1c6'; },
  datatype() { return 'read_waypoints/TurnCompleted'; }
};
