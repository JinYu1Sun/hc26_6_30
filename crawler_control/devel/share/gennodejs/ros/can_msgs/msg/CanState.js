// Auto-generated. Do not edit!

// (in-package can_msgs.msg)


"use strict";

const _serializer = _ros_msg_utils.Serialize;
const _arraySerializer = _serializer.Array;
const _deserializer = _ros_msg_utils.Deserialize;
const _arrayDeserializer = _deserializer.Array;
const _finder = _ros_msg_utils.Find;
const _getByteLength = _ros_msg_utils.getByteLength;

//-----------------------------------------------------------

class CanState {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.driver_state = null;
    }
    else {
      if (initObj.hasOwnProperty('driver_state')) {
        this.driver_state = initObj.driver_state
      }
      else {
        this.driver_state = 0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type CanState
    // Serialize message field [driver_state]
    bufferOffset = _serializer.uint8(obj.driver_state, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type CanState
    let len;
    let data = new CanState(null);
    // Deserialize message field [driver_state]
    data.driver_state = _deserializer.uint8(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    return 1;
  }

  static datatype() {
    // Returns string type for a message object
    return 'can_msgs/CanState';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'ae2e3357d3e85e9ae470ea447e6ef3b6';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    # CanState.msg
    #
    # Message denoting the state of the CAN Bus
    #
    # @author Joe Adkisson
    # @version 0.1
    
    uint8  driver_state
    
    # enumeration values for status:
    uint8 CLOSED=0
    uint8 OPEN=1
    uint8 READY=2
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new CanState(null);
    if (msg.driver_state !== undefined) {
      resolved.driver_state = msg.driver_state;
    }
    else {
      resolved.driver_state = 0
    }

    return resolved;
    }
};

// Constants for message
CanState.Constants = {
  CLOSED: 0,
  OPEN: 1,
  READY: 2,
}

module.exports = CanState;
