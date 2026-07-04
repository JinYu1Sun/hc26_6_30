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

class LocalPose {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.header = null;
      this.plat_id = null;
      this.error_code = null;
      this.gps_week = null;
      this.gps_millisecond = null;
      this.dr_x = null;
      this.dr_y = null;
      this.dr_z = null;
      this.dr_heading = null;
      this.dr_roll = null;
      this.dr_pitch = null;
      this.vehicle_speed = null;
      this.rot_x = null;
      this.rot_y = null;
      this.rot_z = null;
      this.acc_x = null;
      this.acc_y = null;
      this.acc_z = null;
    }
    else {
      if (initObj.hasOwnProperty('header')) {
        this.header = initObj.header
      }
      else {
        this.header = new std_msgs.msg.Header();
      }
      if (initObj.hasOwnProperty('plat_id')) {
        this.plat_id = initObj.plat_id
      }
      else {
        this.plat_id = 0;
      }
      if (initObj.hasOwnProperty('error_code')) {
        this.error_code = initObj.error_code
      }
      else {
        this.error_code = 0;
      }
      if (initObj.hasOwnProperty('gps_week')) {
        this.gps_week = initObj.gps_week
      }
      else {
        this.gps_week = 0;
      }
      if (initObj.hasOwnProperty('gps_millisecond')) {
        this.gps_millisecond = initObj.gps_millisecond
      }
      else {
        this.gps_millisecond = 0;
      }
      if (initObj.hasOwnProperty('dr_x')) {
        this.dr_x = initObj.dr_x
      }
      else {
        this.dr_x = 0.0;
      }
      if (initObj.hasOwnProperty('dr_y')) {
        this.dr_y = initObj.dr_y
      }
      else {
        this.dr_y = 0.0;
      }
      if (initObj.hasOwnProperty('dr_z')) {
        this.dr_z = initObj.dr_z
      }
      else {
        this.dr_z = 0.0;
      }
      if (initObj.hasOwnProperty('dr_heading')) {
        this.dr_heading = initObj.dr_heading
      }
      else {
        this.dr_heading = 0.0;
      }
      if (initObj.hasOwnProperty('dr_roll')) {
        this.dr_roll = initObj.dr_roll
      }
      else {
        this.dr_roll = 0.0;
      }
      if (initObj.hasOwnProperty('dr_pitch')) {
        this.dr_pitch = initObj.dr_pitch
      }
      else {
        this.dr_pitch = 0.0;
      }
      if (initObj.hasOwnProperty('vehicle_speed')) {
        this.vehicle_speed = initObj.vehicle_speed
      }
      else {
        this.vehicle_speed = 0;
      }
      if (initObj.hasOwnProperty('rot_x')) {
        this.rot_x = initObj.rot_x
      }
      else {
        this.rot_x = 0.0;
      }
      if (initObj.hasOwnProperty('rot_y')) {
        this.rot_y = initObj.rot_y
      }
      else {
        this.rot_y = 0.0;
      }
      if (initObj.hasOwnProperty('rot_z')) {
        this.rot_z = initObj.rot_z
      }
      else {
        this.rot_z = 0.0;
      }
      if (initObj.hasOwnProperty('acc_x')) {
        this.acc_x = initObj.acc_x
      }
      else {
        this.acc_x = 0.0;
      }
      if (initObj.hasOwnProperty('acc_y')) {
        this.acc_y = initObj.acc_y
      }
      else {
        this.acc_y = 0.0;
      }
      if (initObj.hasOwnProperty('acc_z')) {
        this.acc_z = initObj.acc_z
      }
      else {
        this.acc_z = 0.0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type LocalPose
    // Serialize message field [header]
    bufferOffset = std_msgs.msg.Header.serialize(obj.header, buffer, bufferOffset);
    // Serialize message field [plat_id]
    bufferOffset = _serializer.int32(obj.plat_id, buffer, bufferOffset);
    // Serialize message field [error_code]
    bufferOffset = _serializer.int32(obj.error_code, buffer, bufferOffset);
    // Serialize message field [gps_week]
    bufferOffset = _serializer.int32(obj.gps_week, buffer, bufferOffset);
    // Serialize message field [gps_millisecond]
    bufferOffset = _serializer.int32(obj.gps_millisecond, buffer, bufferOffset);
    // Serialize message field [dr_x]
    bufferOffset = _serializer.float64(obj.dr_x, buffer, bufferOffset);
    // Serialize message field [dr_y]
    bufferOffset = _serializer.float64(obj.dr_y, buffer, bufferOffset);
    // Serialize message field [dr_z]
    bufferOffset = _serializer.float64(obj.dr_z, buffer, bufferOffset);
    // Serialize message field [dr_heading]
    bufferOffset = _serializer.float64(obj.dr_heading, buffer, bufferOffset);
    // Serialize message field [dr_roll]
    bufferOffset = _serializer.float64(obj.dr_roll, buffer, bufferOffset);
    // Serialize message field [dr_pitch]
    bufferOffset = _serializer.float64(obj.dr_pitch, buffer, bufferOffset);
    // Serialize message field [vehicle_speed]
    bufferOffset = _serializer.int32(obj.vehicle_speed, buffer, bufferOffset);
    // Serialize message field [rot_x]
    bufferOffset = _serializer.float64(obj.rot_x, buffer, bufferOffset);
    // Serialize message field [rot_y]
    bufferOffset = _serializer.float64(obj.rot_y, buffer, bufferOffset);
    // Serialize message field [rot_z]
    bufferOffset = _serializer.float64(obj.rot_z, buffer, bufferOffset);
    // Serialize message field [acc_x]
    bufferOffset = _serializer.float64(obj.acc_x, buffer, bufferOffset);
    // Serialize message field [acc_y]
    bufferOffset = _serializer.float64(obj.acc_y, buffer, bufferOffset);
    // Serialize message field [acc_z]
    bufferOffset = _serializer.float64(obj.acc_z, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type LocalPose
    let len;
    let data = new LocalPose(null);
    // Deserialize message field [header]
    data.header = std_msgs.msg.Header.deserialize(buffer, bufferOffset);
    // Deserialize message field [plat_id]
    data.plat_id = _deserializer.int32(buffer, bufferOffset);
    // Deserialize message field [error_code]
    data.error_code = _deserializer.int32(buffer, bufferOffset);
    // Deserialize message field [gps_week]
    data.gps_week = _deserializer.int32(buffer, bufferOffset);
    // Deserialize message field [gps_millisecond]
    data.gps_millisecond = _deserializer.int32(buffer, bufferOffset);
    // Deserialize message field [dr_x]
    data.dr_x = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [dr_y]
    data.dr_y = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [dr_z]
    data.dr_z = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [dr_heading]
    data.dr_heading = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [dr_roll]
    data.dr_roll = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [dr_pitch]
    data.dr_pitch = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [vehicle_speed]
    data.vehicle_speed = _deserializer.int32(buffer, bufferOffset);
    // Deserialize message field [rot_x]
    data.rot_x = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [rot_y]
    data.rot_y = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [rot_z]
    data.rot_z = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [acc_x]
    data.acc_x = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [acc_y]
    data.acc_y = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [acc_z]
    data.acc_z = _deserializer.float64(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += std_msgs.msg.Header.getMessageSize(object.header);
    return length + 116;
  }

  static datatype() {
    // Returns string type for a message object
    return 'util/LocalPose';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '89af9872db18309af0c18302f7e3f52e';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    #LocalPose
    Header header
    int32 	plat_id				#标识此消息对应的平台编号
    
    int32 	error_code   		#错误标识码
    
    int32 	gps_week			#GPS Week
    int32 	gps_millisecond     #GPS millionsecond in a week ,单位：ms
    
    float64 dr_x           		#航迹推算车辆位置,单位：cm
    float64 dr_y
    float64 dr_z
    
    float64 dr_heading     		#车体姿态,单位:0.01degree	, 航向角正东方向为零度,逆时针方向为正,0-360度	
    float64 dr_roll				
    float64 dr_pitch		
    
    
    int32 	vehicle_speed   	#车速：cm/s  负值表示倒车
    
    #imu三轴陀螺速度 
    float64 rot_x       		#x轴方向右手定则,四指方向为正,单位0.01 deg/s
    float64 rot_y				#y轴方向右手定则,四指方向为正,单位0.01 deg/s
    float64 rot_z				#z轴方向右手定则,四指方向为正,单位0.01 deg/s
    
    #imu三轴加速度 
    float64 acc_x				#x轴方向加速度(车体正右方)	单位0.01m/s^2
    float64 acc_y				#y轴方向加速度(车体正前方)	单位0.01m/s^2
    float64 acc_z				#z轴方向加速度(车体正上方)	单位0.01m/s^2
    
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
    const resolved = new LocalPose(null);
    if (msg.header !== undefined) {
      resolved.header = std_msgs.msg.Header.Resolve(msg.header)
    }
    else {
      resolved.header = new std_msgs.msg.Header()
    }

    if (msg.plat_id !== undefined) {
      resolved.plat_id = msg.plat_id;
    }
    else {
      resolved.plat_id = 0
    }

    if (msg.error_code !== undefined) {
      resolved.error_code = msg.error_code;
    }
    else {
      resolved.error_code = 0
    }

    if (msg.gps_week !== undefined) {
      resolved.gps_week = msg.gps_week;
    }
    else {
      resolved.gps_week = 0
    }

    if (msg.gps_millisecond !== undefined) {
      resolved.gps_millisecond = msg.gps_millisecond;
    }
    else {
      resolved.gps_millisecond = 0
    }

    if (msg.dr_x !== undefined) {
      resolved.dr_x = msg.dr_x;
    }
    else {
      resolved.dr_x = 0.0
    }

    if (msg.dr_y !== undefined) {
      resolved.dr_y = msg.dr_y;
    }
    else {
      resolved.dr_y = 0.0
    }

    if (msg.dr_z !== undefined) {
      resolved.dr_z = msg.dr_z;
    }
    else {
      resolved.dr_z = 0.0
    }

    if (msg.dr_heading !== undefined) {
      resolved.dr_heading = msg.dr_heading;
    }
    else {
      resolved.dr_heading = 0.0
    }

    if (msg.dr_roll !== undefined) {
      resolved.dr_roll = msg.dr_roll;
    }
    else {
      resolved.dr_roll = 0.0
    }

    if (msg.dr_pitch !== undefined) {
      resolved.dr_pitch = msg.dr_pitch;
    }
    else {
      resolved.dr_pitch = 0.0
    }

    if (msg.vehicle_speed !== undefined) {
      resolved.vehicle_speed = msg.vehicle_speed;
    }
    else {
      resolved.vehicle_speed = 0
    }

    if (msg.rot_x !== undefined) {
      resolved.rot_x = msg.rot_x;
    }
    else {
      resolved.rot_x = 0.0
    }

    if (msg.rot_y !== undefined) {
      resolved.rot_y = msg.rot_y;
    }
    else {
      resolved.rot_y = 0.0
    }

    if (msg.rot_z !== undefined) {
      resolved.rot_z = msg.rot_z;
    }
    else {
      resolved.rot_z = 0.0
    }

    if (msg.acc_x !== undefined) {
      resolved.acc_x = msg.acc_x;
    }
    else {
      resolved.acc_x = 0.0
    }

    if (msg.acc_y !== undefined) {
      resolved.acc_y = msg.acc_y;
    }
    else {
      resolved.acc_y = 0.0
    }

    if (msg.acc_z !== undefined) {
      resolved.acc_z = msg.acc_z;
    }
    else {
      resolved.acc_z = 0.0
    }

    return resolved;
    }
};

module.exports = LocalPose;
