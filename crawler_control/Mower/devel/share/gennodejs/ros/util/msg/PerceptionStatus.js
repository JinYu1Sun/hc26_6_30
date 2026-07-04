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

class PerceptionStatus {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.header = null;
      this.lidar2BaselinkStatus = null;
      this.lidar2CameraStatus = null;
      this.cameraIntrinsicStatus = null;
      this.distcoeffStatus = null;
      this.pointcloudStatus = null;
      this.poseStatus = null;
      this.yoloStatus = null;
    }
    else {
      if (initObj.hasOwnProperty('header')) {
        this.header = initObj.header
      }
      else {
        this.header = new std_msgs.msg.Header();
      }
      if (initObj.hasOwnProperty('lidar2BaselinkStatus')) {
        this.lidar2BaselinkStatus = initObj.lidar2BaselinkStatus
      }
      else {
        this.lidar2BaselinkStatus = false;
      }
      if (initObj.hasOwnProperty('lidar2CameraStatus')) {
        this.lidar2CameraStatus = initObj.lidar2CameraStatus
      }
      else {
        this.lidar2CameraStatus = false;
      }
      if (initObj.hasOwnProperty('cameraIntrinsicStatus')) {
        this.cameraIntrinsicStatus = initObj.cameraIntrinsicStatus
      }
      else {
        this.cameraIntrinsicStatus = false;
      }
      if (initObj.hasOwnProperty('distcoeffStatus')) {
        this.distcoeffStatus = initObj.distcoeffStatus
      }
      else {
        this.distcoeffStatus = false;
      }
      if (initObj.hasOwnProperty('pointcloudStatus')) {
        this.pointcloudStatus = initObj.pointcloudStatus
      }
      else {
        this.pointcloudStatus = false;
      }
      if (initObj.hasOwnProperty('poseStatus')) {
        this.poseStatus = initObj.poseStatus
      }
      else {
        this.poseStatus = false;
      }
      if (initObj.hasOwnProperty('yoloStatus')) {
        this.yoloStatus = initObj.yoloStatus
      }
      else {
        this.yoloStatus = false;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type PerceptionStatus
    // Serialize message field [header]
    bufferOffset = std_msgs.msg.Header.serialize(obj.header, buffer, bufferOffset);
    // Serialize message field [lidar2BaselinkStatus]
    bufferOffset = _serializer.bool(obj.lidar2BaselinkStatus, buffer, bufferOffset);
    // Serialize message field [lidar2CameraStatus]
    bufferOffset = _serializer.bool(obj.lidar2CameraStatus, buffer, bufferOffset);
    // Serialize message field [cameraIntrinsicStatus]
    bufferOffset = _serializer.bool(obj.cameraIntrinsicStatus, buffer, bufferOffset);
    // Serialize message field [distcoeffStatus]
    bufferOffset = _serializer.bool(obj.distcoeffStatus, buffer, bufferOffset);
    // Serialize message field [pointcloudStatus]
    bufferOffset = _serializer.bool(obj.pointcloudStatus, buffer, bufferOffset);
    // Serialize message field [poseStatus]
    bufferOffset = _serializer.bool(obj.poseStatus, buffer, bufferOffset);
    // Serialize message field [yoloStatus]
    bufferOffset = _serializer.bool(obj.yoloStatus, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type PerceptionStatus
    let len;
    let data = new PerceptionStatus(null);
    // Deserialize message field [header]
    data.header = std_msgs.msg.Header.deserialize(buffer, bufferOffset);
    // Deserialize message field [lidar2BaselinkStatus]
    data.lidar2BaselinkStatus = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [lidar2CameraStatus]
    data.lidar2CameraStatus = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [cameraIntrinsicStatus]
    data.cameraIntrinsicStatus = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [distcoeffStatus]
    data.distcoeffStatus = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [pointcloudStatus]
    data.pointcloudStatus = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [poseStatus]
    data.poseStatus = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [yoloStatus]
    data.yoloStatus = _deserializer.bool(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += std_msgs.msg.Header.getMessageSize(object.header);
    return length + 7;
  }

  static datatype() {
    // Returns string type for a message object
    return 'util/PerceptionStatus';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'ab035d44aff8214d3f26c118b7695eb3';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    Header header
    bool lidar2BaselinkStatus
    bool lidar2CameraStatus
    bool cameraIntrinsicStatus
    bool distcoeffStatus
    bool pointcloudStatus
    bool poseStatus
    bool yoloStatus
    
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
    const resolved = new PerceptionStatus(null);
    if (msg.header !== undefined) {
      resolved.header = std_msgs.msg.Header.Resolve(msg.header)
    }
    else {
      resolved.header = new std_msgs.msg.Header()
    }

    if (msg.lidar2BaselinkStatus !== undefined) {
      resolved.lidar2BaselinkStatus = msg.lidar2BaselinkStatus;
    }
    else {
      resolved.lidar2BaselinkStatus = false
    }

    if (msg.lidar2CameraStatus !== undefined) {
      resolved.lidar2CameraStatus = msg.lidar2CameraStatus;
    }
    else {
      resolved.lidar2CameraStatus = false
    }

    if (msg.cameraIntrinsicStatus !== undefined) {
      resolved.cameraIntrinsicStatus = msg.cameraIntrinsicStatus;
    }
    else {
      resolved.cameraIntrinsicStatus = false
    }

    if (msg.distcoeffStatus !== undefined) {
      resolved.distcoeffStatus = msg.distcoeffStatus;
    }
    else {
      resolved.distcoeffStatus = false
    }

    if (msg.pointcloudStatus !== undefined) {
      resolved.pointcloudStatus = msg.pointcloudStatus;
    }
    else {
      resolved.pointcloudStatus = false
    }

    if (msg.poseStatus !== undefined) {
      resolved.poseStatus = msg.poseStatus;
    }
    else {
      resolved.poseStatus = false
    }

    if (msg.yoloStatus !== undefined) {
      resolved.yoloStatus = msg.yoloStatus;
    }
    else {
      resolved.yoloStatus = false
    }

    return resolved;
    }
};

module.exports = PerceptionStatus;
