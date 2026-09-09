// Auto-generated. Do not edit!

// (in-package mower_msgs.msg)


"use strict";

const _serializer = _ros_msg_utils.Serialize;
const _arraySerializer = _serializer.Array;
const _deserializer = _ros_msg_utils.Deserialize;
const _arrayDeserializer = _deserializer.Array;
const _finder = _ros_msg_utils.Find;
const _getByteLength = _ros_msg_utils.getByteLength;

//-----------------------------------------------------------

class PlanType {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.plan_type_state = null;
    }
    else {
      if (initObj.hasOwnProperty('plan_type_state')) {
        this.plan_type_state = initObj.plan_type_state
      }
      else {
        this.plan_type_state = 0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type PlanType
    // Serialize message field [plan_type_state]
    bufferOffset = _serializer.uint8(obj.plan_type_state, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type PlanType
    let len;
    let data = new PlanType(null);
    // Deserialize message field [plan_type_state]
    data.plan_type_state = _deserializer.uint8(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    return 1;
  }

  static datatype() {
    // Returns string type for a message object
    return 'mower_msgs/PlanType';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '23d6dde9eb5a6d1df00bc1332e5a8aeb';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    uint8 plan_type_state
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new PlanType(null);
    if (msg.plan_type_state !== undefined) {
      resolved.plan_type_state = msg.plan_type_state;
    }
    else {
      resolved.plan_type_state = 0
    }

    return resolved;
    }
};

module.exports = PlanType;
