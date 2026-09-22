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

class ImuStaticCalibration {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.header = null;
      this.valid = null;
      this.lio_generation = null;
      this.imu_topic = null;
      this.duration_sec = null;
      this.sample_count = null;
      this.mean_acceleration_mps2 = null;
      this.mean_angular_velocity_radps = null;
      this.acceleration_variance = null;
      this.angular_velocity_variance = null;
      this.max_angular_velocity_norm_radps = null;
    }
    else {
      if (initObj.hasOwnProperty('header')) {
        this.header = initObj.header
      }
      else {
        this.header = new std_msgs.msg.Header();
      }
      if (initObj.hasOwnProperty('valid')) {
        this.valid = initObj.valid
      }
      else {
        this.valid = false;
      }
      if (initObj.hasOwnProperty('lio_generation')) {
        this.lio_generation = initObj.lio_generation
      }
      else {
        this.lio_generation = {secs: 0, nsecs: 0};
      }
      if (initObj.hasOwnProperty('imu_topic')) {
        this.imu_topic = initObj.imu_topic
      }
      else {
        this.imu_topic = '';
      }
      if (initObj.hasOwnProperty('duration_sec')) {
        this.duration_sec = initObj.duration_sec
      }
      else {
        this.duration_sec = 0.0;
      }
      if (initObj.hasOwnProperty('sample_count')) {
        this.sample_count = initObj.sample_count
      }
      else {
        this.sample_count = 0;
      }
      if (initObj.hasOwnProperty('mean_acceleration_mps2')) {
        this.mean_acceleration_mps2 = initObj.mean_acceleration_mps2
      }
      else {
        this.mean_acceleration_mps2 = new Array(3).fill(0);
      }
      if (initObj.hasOwnProperty('mean_angular_velocity_radps')) {
        this.mean_angular_velocity_radps = initObj.mean_angular_velocity_radps
      }
      else {
        this.mean_angular_velocity_radps = new Array(3).fill(0);
      }
      if (initObj.hasOwnProperty('acceleration_variance')) {
        this.acceleration_variance = initObj.acceleration_variance
      }
      else {
        this.acceleration_variance = new Array(3).fill(0);
      }
      if (initObj.hasOwnProperty('angular_velocity_variance')) {
        this.angular_velocity_variance = initObj.angular_velocity_variance
      }
      else {
        this.angular_velocity_variance = new Array(3).fill(0);
      }
      if (initObj.hasOwnProperty('max_angular_velocity_norm_radps')) {
        this.max_angular_velocity_norm_radps = initObj.max_angular_velocity_norm_radps
      }
      else {
        this.max_angular_velocity_norm_radps = 0.0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type ImuStaticCalibration
    // Serialize message field [header]
    bufferOffset = std_msgs.msg.Header.serialize(obj.header, buffer, bufferOffset);
    // Serialize message field [valid]
    bufferOffset = _serializer.bool(obj.valid, buffer, bufferOffset);
    // Serialize message field [lio_generation]
    bufferOffset = _serializer.time(obj.lio_generation, buffer, bufferOffset);
    // Serialize message field [imu_topic]
    bufferOffset = _serializer.string(obj.imu_topic, buffer, bufferOffset);
    // Serialize message field [duration_sec]
    bufferOffset = _serializer.float64(obj.duration_sec, buffer, bufferOffset);
    // Serialize message field [sample_count]
    bufferOffset = _serializer.uint32(obj.sample_count, buffer, bufferOffset);
    // Check that the constant length array field [mean_acceleration_mps2] has the right length
    if (obj.mean_acceleration_mps2.length !== 3) {
      throw new Error('Unable to serialize array field mean_acceleration_mps2 - length must be 3')
    }
    // Serialize message field [mean_acceleration_mps2]
    bufferOffset = _arraySerializer.float64(obj.mean_acceleration_mps2, buffer, bufferOffset, 3);
    // Check that the constant length array field [mean_angular_velocity_radps] has the right length
    if (obj.mean_angular_velocity_radps.length !== 3) {
      throw new Error('Unable to serialize array field mean_angular_velocity_radps - length must be 3')
    }
    // Serialize message field [mean_angular_velocity_radps]
    bufferOffset = _arraySerializer.float64(obj.mean_angular_velocity_radps, buffer, bufferOffset, 3);
    // Check that the constant length array field [acceleration_variance] has the right length
    if (obj.acceleration_variance.length !== 3) {
      throw new Error('Unable to serialize array field acceleration_variance - length must be 3')
    }
    // Serialize message field [acceleration_variance]
    bufferOffset = _arraySerializer.float64(obj.acceleration_variance, buffer, bufferOffset, 3);
    // Check that the constant length array field [angular_velocity_variance] has the right length
    if (obj.angular_velocity_variance.length !== 3) {
      throw new Error('Unable to serialize array field angular_velocity_variance - length must be 3')
    }
    // Serialize message field [angular_velocity_variance]
    bufferOffset = _arraySerializer.float64(obj.angular_velocity_variance, buffer, bufferOffset, 3);
    // Serialize message field [max_angular_velocity_norm_radps]
    bufferOffset = _serializer.float64(obj.max_angular_velocity_norm_radps, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type ImuStaticCalibration
    let len;
    let data = new ImuStaticCalibration(null);
    // Deserialize message field [header]
    data.header = std_msgs.msg.Header.deserialize(buffer, bufferOffset);
    // Deserialize message field [valid]
    data.valid = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [lio_generation]
    data.lio_generation = _deserializer.time(buffer, bufferOffset);
    // Deserialize message field [imu_topic]
    data.imu_topic = _deserializer.string(buffer, bufferOffset);
    // Deserialize message field [duration_sec]
    data.duration_sec = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [sample_count]
    data.sample_count = _deserializer.uint32(buffer, bufferOffset);
    // Deserialize message field [mean_acceleration_mps2]
    data.mean_acceleration_mps2 = _arrayDeserializer.float64(buffer, bufferOffset, 3)
    // Deserialize message field [mean_angular_velocity_radps]
    data.mean_angular_velocity_radps = _arrayDeserializer.float64(buffer, bufferOffset, 3)
    // Deserialize message field [acceleration_variance]
    data.acceleration_variance = _arrayDeserializer.float64(buffer, bufferOffset, 3)
    // Deserialize message field [angular_velocity_variance]
    data.angular_velocity_variance = _arrayDeserializer.float64(buffer, bufferOffset, 3)
    // Deserialize message field [max_angular_velocity_norm_radps]
    data.max_angular_velocity_norm_radps = _deserializer.float64(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += std_msgs.msg.Header.getMessageSize(object.header);
    length += _getByteLength(object.imu_topic);
    return length + 129;
  }

  static datatype() {
    // Returns string type for a message object
    return 'util/ImuStaticCalibration';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '18a255ee627c55f5a95de2c4231b1512';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    # FAST-LIO权威静止初始化窗口统计。消息由/laserMapping锁存发布；
    # header.stamp是该静止窗口最后一帧IMU的传感器时间，header.frame_id是原始IMU坐标系。
    Header header
    bool valid
    time lio_generation
    string imu_topic
    float64 duration_sec
    uint32 sample_count
    float64[3] mean_acceleration_mps2
    float64[3] mean_angular_velocity_radps
    float64[3] acceleration_variance
    float64[3] angular_velocity_variance
    float64 max_angular_velocity_norm_radps
    
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
    const resolved = new ImuStaticCalibration(null);
    if (msg.header !== undefined) {
      resolved.header = std_msgs.msg.Header.Resolve(msg.header)
    }
    else {
      resolved.header = new std_msgs.msg.Header()
    }

    if (msg.valid !== undefined) {
      resolved.valid = msg.valid;
    }
    else {
      resolved.valid = false
    }

    if (msg.lio_generation !== undefined) {
      resolved.lio_generation = msg.lio_generation;
    }
    else {
      resolved.lio_generation = {secs: 0, nsecs: 0}
    }

    if (msg.imu_topic !== undefined) {
      resolved.imu_topic = msg.imu_topic;
    }
    else {
      resolved.imu_topic = ''
    }

    if (msg.duration_sec !== undefined) {
      resolved.duration_sec = msg.duration_sec;
    }
    else {
      resolved.duration_sec = 0.0
    }

    if (msg.sample_count !== undefined) {
      resolved.sample_count = msg.sample_count;
    }
    else {
      resolved.sample_count = 0
    }

    if (msg.mean_acceleration_mps2 !== undefined) {
      resolved.mean_acceleration_mps2 = msg.mean_acceleration_mps2;
    }
    else {
      resolved.mean_acceleration_mps2 = new Array(3).fill(0)
    }

    if (msg.mean_angular_velocity_radps !== undefined) {
      resolved.mean_angular_velocity_radps = msg.mean_angular_velocity_radps;
    }
    else {
      resolved.mean_angular_velocity_radps = new Array(3).fill(0)
    }

    if (msg.acceleration_variance !== undefined) {
      resolved.acceleration_variance = msg.acceleration_variance;
    }
    else {
      resolved.acceleration_variance = new Array(3).fill(0)
    }

    if (msg.angular_velocity_variance !== undefined) {
      resolved.angular_velocity_variance = msg.angular_velocity_variance;
    }
    else {
      resolved.angular_velocity_variance = new Array(3).fill(0)
    }

    if (msg.max_angular_velocity_norm_radps !== undefined) {
      resolved.max_angular_velocity_norm_radps = msg.max_angular_velocity_norm_radps;
    }
    else {
      resolved.max_angular_velocity_norm_radps = 0.0
    }

    return resolved;
    }
};

module.exports = ImuStaticCalibration;
