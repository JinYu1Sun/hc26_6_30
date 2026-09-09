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

class TaskStatus {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.task_status = null;
    }
    else {
      if (initObj.hasOwnProperty('task_status')) {
        this.task_status = initObj.task_status
      }
      else {
        this.task_status = '';
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type TaskStatus
    // Serialize message field [task_status]
    bufferOffset = _serializer.string(obj.task_status, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type TaskStatus
    let len;
    let data = new TaskStatus(null);
    // Deserialize message field [task_status]
    data.task_status = _deserializer.string(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += _getByteLength(object.task_status);
    return length + 4;
  }

  static datatype() {
    // Returns string type for a message object
    return 'mower_msgs/TaskStatus';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '53109e0b2984975887b08ba65c603762';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    string task_status
    
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new TaskStatus(null);
    if (msg.task_status !== undefined) {
      resolved.task_status = msg.task_status;
    }
    else {
      resolved.task_status = ''
    }

    return resolved;
    }
};

module.exports = TaskStatus;
