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

class GpsPosition {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.header = null;
      this.plat_id = null;
      this.error_code = null;
      this.gps_flag = null;
      this.positionStatus = null;
      this.gps_week = null;
      this.gps_millisecond = null;
      this.longitude = null;
      this.latitude = null;
      this.height = null;
      this.gaussX = null;
      this.gaussY = null;
      this.pitch = null;
      this.roll = null;
      this.azimuth = null;
      this.acc_x = null;
      this.acc_y = null;
      this.acc_z = null;
      this.rot_x = null;
      this.rot_y = null;
      this.rot_z = null;
      this.northVelocity = null;
      this.eastVelocity = null;
      this.upVelocity = null;
      this.gps_confidence = null;
      this.INS_GpsFlag_Pos = null;
      this.INS_NumSV = null;
      this.INS_GpsFlag_Heading = null;
      this.INS_Gps_Age = null;
      this.INS_Car_Status = null;
      this.INS_Status = null;
      this.INS_VehicleAlign = null;
      this.INS_Std_Lat = null;
      this.INS_Std_Lon = null;
      this.INS_Std_LocatHeight = null;
      this.INS_Std_Heading = null;
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
      if (initObj.hasOwnProperty('gps_flag')) {
        this.gps_flag = initObj.gps_flag
      }
      else {
        this.gps_flag = 0;
      }
      if (initObj.hasOwnProperty('positionStatus')) {
        this.positionStatus = initObj.positionStatus
      }
      else {
        this.positionStatus = 0;
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
      if (initObj.hasOwnProperty('longitude')) {
        this.longitude = initObj.longitude
      }
      else {
        this.longitude = 0.0;
      }
      if (initObj.hasOwnProperty('latitude')) {
        this.latitude = initObj.latitude
      }
      else {
        this.latitude = 0.0;
      }
      if (initObj.hasOwnProperty('height')) {
        this.height = initObj.height
      }
      else {
        this.height = 0.0;
      }
      if (initObj.hasOwnProperty('gaussX')) {
        this.gaussX = initObj.gaussX
      }
      else {
        this.gaussX = 0.0;
      }
      if (initObj.hasOwnProperty('gaussY')) {
        this.gaussY = initObj.gaussY
      }
      else {
        this.gaussY = 0.0;
      }
      if (initObj.hasOwnProperty('pitch')) {
        this.pitch = initObj.pitch
      }
      else {
        this.pitch = 0.0;
      }
      if (initObj.hasOwnProperty('roll')) {
        this.roll = initObj.roll
      }
      else {
        this.roll = 0.0;
      }
      if (initObj.hasOwnProperty('azimuth')) {
        this.azimuth = initObj.azimuth
      }
      else {
        this.azimuth = 0.0;
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
      if (initObj.hasOwnProperty('northVelocity')) {
        this.northVelocity = initObj.northVelocity
      }
      else {
        this.northVelocity = 0.0;
      }
      if (initObj.hasOwnProperty('eastVelocity')) {
        this.eastVelocity = initObj.eastVelocity
      }
      else {
        this.eastVelocity = 0.0;
      }
      if (initObj.hasOwnProperty('upVelocity')) {
        this.upVelocity = initObj.upVelocity
      }
      else {
        this.upVelocity = 0.0;
      }
      if (initObj.hasOwnProperty('gps_confidence')) {
        this.gps_confidence = initObj.gps_confidence
      }
      else {
        this.gps_confidence = 0;
      }
      if (initObj.hasOwnProperty('INS_GpsFlag_Pos')) {
        this.INS_GpsFlag_Pos = initObj.INS_GpsFlag_Pos
      }
      else {
        this.INS_GpsFlag_Pos = 0;
      }
      if (initObj.hasOwnProperty('INS_NumSV')) {
        this.INS_NumSV = initObj.INS_NumSV
      }
      else {
        this.INS_NumSV = 0;
      }
      if (initObj.hasOwnProperty('INS_GpsFlag_Heading')) {
        this.INS_GpsFlag_Heading = initObj.INS_GpsFlag_Heading
      }
      else {
        this.INS_GpsFlag_Heading = 0;
      }
      if (initObj.hasOwnProperty('INS_Gps_Age')) {
        this.INS_Gps_Age = initObj.INS_Gps_Age
      }
      else {
        this.INS_Gps_Age = 0;
      }
      if (initObj.hasOwnProperty('INS_Car_Status')) {
        this.INS_Car_Status = initObj.INS_Car_Status
      }
      else {
        this.INS_Car_Status = 0;
      }
      if (initObj.hasOwnProperty('INS_Status')) {
        this.INS_Status = initObj.INS_Status
      }
      else {
        this.INS_Status = 0;
      }
      if (initObj.hasOwnProperty('INS_VehicleAlign')) {
        this.INS_VehicleAlign = initObj.INS_VehicleAlign
      }
      else {
        this.INS_VehicleAlign = 0;
      }
      if (initObj.hasOwnProperty('INS_Std_Lat')) {
        this.INS_Std_Lat = initObj.INS_Std_Lat
      }
      else {
        this.INS_Std_Lat = 0.0;
      }
      if (initObj.hasOwnProperty('INS_Std_Lon')) {
        this.INS_Std_Lon = initObj.INS_Std_Lon
      }
      else {
        this.INS_Std_Lon = 0.0;
      }
      if (initObj.hasOwnProperty('INS_Std_LocatHeight')) {
        this.INS_Std_LocatHeight = initObj.INS_Std_LocatHeight
      }
      else {
        this.INS_Std_LocatHeight = 0.0;
      }
      if (initObj.hasOwnProperty('INS_Std_Heading')) {
        this.INS_Std_Heading = initObj.INS_Std_Heading
      }
      else {
        this.INS_Std_Heading = 0.0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type GpsPosition
    // Serialize message field [header]
    bufferOffset = std_msgs.msg.Header.serialize(obj.header, buffer, bufferOffset);
    // Serialize message field [plat_id]
    bufferOffset = _serializer.int32(obj.plat_id, buffer, bufferOffset);
    // Serialize message field [error_code]
    bufferOffset = _serializer.int32(obj.error_code, buffer, bufferOffset);
    // Serialize message field [gps_flag]
    bufferOffset = _serializer.int32(obj.gps_flag, buffer, bufferOffset);
    // Serialize message field [positionStatus]
    bufferOffset = _serializer.int32(obj.positionStatus, buffer, bufferOffset);
    // Serialize message field [gps_week]
    bufferOffset = _serializer.uint32(obj.gps_week, buffer, bufferOffset);
    // Serialize message field [gps_millisecond]
    bufferOffset = _serializer.uint32(obj.gps_millisecond, buffer, bufferOffset);
    // Serialize message field [longitude]
    bufferOffset = _serializer.float64(obj.longitude, buffer, bufferOffset);
    // Serialize message field [latitude]
    bufferOffset = _serializer.float64(obj.latitude, buffer, bufferOffset);
    // Serialize message field [height]
    bufferOffset = _serializer.float64(obj.height, buffer, bufferOffset);
    // Serialize message field [gaussX]
    bufferOffset = _serializer.float64(obj.gaussX, buffer, bufferOffset);
    // Serialize message field [gaussY]
    bufferOffset = _serializer.float64(obj.gaussY, buffer, bufferOffset);
    // Serialize message field [pitch]
    bufferOffset = _serializer.float64(obj.pitch, buffer, bufferOffset);
    // Serialize message field [roll]
    bufferOffset = _serializer.float64(obj.roll, buffer, bufferOffset);
    // Serialize message field [azimuth]
    bufferOffset = _serializer.float64(obj.azimuth, buffer, bufferOffset);
    // Serialize message field [acc_x]
    bufferOffset = _serializer.float64(obj.acc_x, buffer, bufferOffset);
    // Serialize message field [acc_y]
    bufferOffset = _serializer.float64(obj.acc_y, buffer, bufferOffset);
    // Serialize message field [acc_z]
    bufferOffset = _serializer.float64(obj.acc_z, buffer, bufferOffset);
    // Serialize message field [rot_x]
    bufferOffset = _serializer.float64(obj.rot_x, buffer, bufferOffset);
    // Serialize message field [rot_y]
    bufferOffset = _serializer.float64(obj.rot_y, buffer, bufferOffset);
    // Serialize message field [rot_z]
    bufferOffset = _serializer.float64(obj.rot_z, buffer, bufferOffset);
    // Serialize message field [northVelocity]
    bufferOffset = _serializer.float64(obj.northVelocity, buffer, bufferOffset);
    // Serialize message field [eastVelocity]
    bufferOffset = _serializer.float64(obj.eastVelocity, buffer, bufferOffset);
    // Serialize message field [upVelocity]
    bufferOffset = _serializer.float64(obj.upVelocity, buffer, bufferOffset);
    // Serialize message field [gps_confidence]
    bufferOffset = _serializer.int32(obj.gps_confidence, buffer, bufferOffset);
    // Serialize message field [INS_GpsFlag_Pos]
    bufferOffset = _serializer.uint8(obj.INS_GpsFlag_Pos, buffer, bufferOffset);
    // Serialize message field [INS_NumSV]
    bufferOffset = _serializer.uint8(obj.INS_NumSV, buffer, bufferOffset);
    // Serialize message field [INS_GpsFlag_Heading]
    bufferOffset = _serializer.uint8(obj.INS_GpsFlag_Heading, buffer, bufferOffset);
    // Serialize message field [INS_Gps_Age]
    bufferOffset = _serializer.uint8(obj.INS_Gps_Age, buffer, bufferOffset);
    // Serialize message field [INS_Car_Status]
    bufferOffset = _serializer.uint8(obj.INS_Car_Status, buffer, bufferOffset);
    // Serialize message field [INS_Status]
    bufferOffset = _serializer.uint8(obj.INS_Status, buffer, bufferOffset);
    // Serialize message field [INS_VehicleAlign]
    bufferOffset = _serializer.uint8(obj.INS_VehicleAlign, buffer, bufferOffset);
    // Serialize message field [INS_Std_Lat]
    bufferOffset = _serializer.float64(obj.INS_Std_Lat, buffer, bufferOffset);
    // Serialize message field [INS_Std_Lon]
    bufferOffset = _serializer.float64(obj.INS_Std_Lon, buffer, bufferOffset);
    // Serialize message field [INS_Std_LocatHeight]
    bufferOffset = _serializer.float64(obj.INS_Std_LocatHeight, buffer, bufferOffset);
    // Serialize message field [INS_Std_Heading]
    bufferOffset = _serializer.float64(obj.INS_Std_Heading, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type GpsPosition
    let len;
    let data = new GpsPosition(null);
    // Deserialize message field [header]
    data.header = std_msgs.msg.Header.deserialize(buffer, bufferOffset);
    // Deserialize message field [plat_id]
    data.plat_id = _deserializer.int32(buffer, bufferOffset);
    // Deserialize message field [error_code]
    data.error_code = _deserializer.int32(buffer, bufferOffset);
    // Deserialize message field [gps_flag]
    data.gps_flag = _deserializer.int32(buffer, bufferOffset);
    // Deserialize message field [positionStatus]
    data.positionStatus = _deserializer.int32(buffer, bufferOffset);
    // Deserialize message field [gps_week]
    data.gps_week = _deserializer.uint32(buffer, bufferOffset);
    // Deserialize message field [gps_millisecond]
    data.gps_millisecond = _deserializer.uint32(buffer, bufferOffset);
    // Deserialize message field [longitude]
    data.longitude = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [latitude]
    data.latitude = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [height]
    data.height = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [gaussX]
    data.gaussX = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [gaussY]
    data.gaussY = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [pitch]
    data.pitch = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [roll]
    data.roll = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [azimuth]
    data.azimuth = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [acc_x]
    data.acc_x = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [acc_y]
    data.acc_y = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [acc_z]
    data.acc_z = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [rot_x]
    data.rot_x = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [rot_y]
    data.rot_y = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [rot_z]
    data.rot_z = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [northVelocity]
    data.northVelocity = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [eastVelocity]
    data.eastVelocity = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [upVelocity]
    data.upVelocity = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [gps_confidence]
    data.gps_confidence = _deserializer.int32(buffer, bufferOffset);
    // Deserialize message field [INS_GpsFlag_Pos]
    data.INS_GpsFlag_Pos = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [INS_NumSV]
    data.INS_NumSV = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [INS_GpsFlag_Heading]
    data.INS_GpsFlag_Heading = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [INS_Gps_Age]
    data.INS_Gps_Age = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [INS_Car_Status]
    data.INS_Car_Status = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [INS_Status]
    data.INS_Status = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [INS_VehicleAlign]
    data.INS_VehicleAlign = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [INS_Std_Lat]
    data.INS_Std_Lat = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [INS_Std_Lon]
    data.INS_Std_Lon = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [INS_Std_LocatHeight]
    data.INS_Std_LocatHeight = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [INS_Std_Heading]
    data.INS_Std_Heading = _deserializer.float64(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += std_msgs.msg.Header.getMessageSize(object.header);
    return length + 203;
  }

  static datatype() {
    // Returns string type for a message object
    return 'util/GpsPosition';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'f75c3f5644c5aad5a6320ced80a0264c';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    #GpsPosition
    Header header
    int32 plat_id			#标识此消息的平台id
    int32 error_code		#错误标识码
    int32 gps_flag			#GPS状态
    int32 positionStatus		#系统运行状态
    
    uint32 gps_week			#GPS Week
    
    uint32 gps_millisecond		#GPS millisecond in a week
    
    float64 longitude		#经纬度，单位为度
    float64 latitude
    float64 height			#海拔,单位为m
    
    float64 gaussX			#高斯投影位置,cm
    float64 gaussY
    
    
    float64 pitch			#俯仰角,x轴方向(正右方)右手定则四指方向为正,单位为0.01度
    float64 roll			#翻滚角,y轴方向(正前方)右手定则四指方向为正,单位为0.01度
    float64 azimuth			#航向角，单位为0.01度,向东为零度，逆时针0-360
    
    float64 acc_x # m/s2
    float64 acc_y
    float64 acc_z
    
    float64 rot_x # deg/s
    float64 rot_y
    float64 rot_z
    
    float64 northVelocity		#north速度，单位为cm/s
    float64 eastVelocity		#east速度,单位为cm/s
    float64 upVelocity		#up速度,单位为cm/s
    
    int32 gps_confidence	#gps定位精度置信度,根据卫星数量和INS,POS解算状态综合得出, 8-10为良好,6-7为一般,小于等于5为信号差,0为没有信号
    
    uint8 INS_GpsFlag_Pos
    uint8 INS_NumSV
    uint8 INS_GpsFlag_Heading
    uint8 INS_Gps_Age
    uint8 INS_Car_Status
    uint8 INS_Status
    uint8 INS_VehicleAlign
    float64 INS_Std_Lat
    float64 INS_Std_Lon
    float64 INS_Std_LocatHeight
    float64 INS_Std_Heading
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
    const resolved = new GpsPosition(null);
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

    if (msg.gps_flag !== undefined) {
      resolved.gps_flag = msg.gps_flag;
    }
    else {
      resolved.gps_flag = 0
    }

    if (msg.positionStatus !== undefined) {
      resolved.positionStatus = msg.positionStatus;
    }
    else {
      resolved.positionStatus = 0
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

    if (msg.longitude !== undefined) {
      resolved.longitude = msg.longitude;
    }
    else {
      resolved.longitude = 0.0
    }

    if (msg.latitude !== undefined) {
      resolved.latitude = msg.latitude;
    }
    else {
      resolved.latitude = 0.0
    }

    if (msg.height !== undefined) {
      resolved.height = msg.height;
    }
    else {
      resolved.height = 0.0
    }

    if (msg.gaussX !== undefined) {
      resolved.gaussX = msg.gaussX;
    }
    else {
      resolved.gaussX = 0.0
    }

    if (msg.gaussY !== undefined) {
      resolved.gaussY = msg.gaussY;
    }
    else {
      resolved.gaussY = 0.0
    }

    if (msg.pitch !== undefined) {
      resolved.pitch = msg.pitch;
    }
    else {
      resolved.pitch = 0.0
    }

    if (msg.roll !== undefined) {
      resolved.roll = msg.roll;
    }
    else {
      resolved.roll = 0.0
    }

    if (msg.azimuth !== undefined) {
      resolved.azimuth = msg.azimuth;
    }
    else {
      resolved.azimuth = 0.0
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

    if (msg.northVelocity !== undefined) {
      resolved.northVelocity = msg.northVelocity;
    }
    else {
      resolved.northVelocity = 0.0
    }

    if (msg.eastVelocity !== undefined) {
      resolved.eastVelocity = msg.eastVelocity;
    }
    else {
      resolved.eastVelocity = 0.0
    }

    if (msg.upVelocity !== undefined) {
      resolved.upVelocity = msg.upVelocity;
    }
    else {
      resolved.upVelocity = 0.0
    }

    if (msg.gps_confidence !== undefined) {
      resolved.gps_confidence = msg.gps_confidence;
    }
    else {
      resolved.gps_confidence = 0
    }

    if (msg.INS_GpsFlag_Pos !== undefined) {
      resolved.INS_GpsFlag_Pos = msg.INS_GpsFlag_Pos;
    }
    else {
      resolved.INS_GpsFlag_Pos = 0
    }

    if (msg.INS_NumSV !== undefined) {
      resolved.INS_NumSV = msg.INS_NumSV;
    }
    else {
      resolved.INS_NumSV = 0
    }

    if (msg.INS_GpsFlag_Heading !== undefined) {
      resolved.INS_GpsFlag_Heading = msg.INS_GpsFlag_Heading;
    }
    else {
      resolved.INS_GpsFlag_Heading = 0
    }

    if (msg.INS_Gps_Age !== undefined) {
      resolved.INS_Gps_Age = msg.INS_Gps_Age;
    }
    else {
      resolved.INS_Gps_Age = 0
    }

    if (msg.INS_Car_Status !== undefined) {
      resolved.INS_Car_Status = msg.INS_Car_Status;
    }
    else {
      resolved.INS_Car_Status = 0
    }

    if (msg.INS_Status !== undefined) {
      resolved.INS_Status = msg.INS_Status;
    }
    else {
      resolved.INS_Status = 0
    }

    if (msg.INS_VehicleAlign !== undefined) {
      resolved.INS_VehicleAlign = msg.INS_VehicleAlign;
    }
    else {
      resolved.INS_VehicleAlign = 0
    }

    if (msg.INS_Std_Lat !== undefined) {
      resolved.INS_Std_Lat = msg.INS_Std_Lat;
    }
    else {
      resolved.INS_Std_Lat = 0.0
    }

    if (msg.INS_Std_Lon !== undefined) {
      resolved.INS_Std_Lon = msg.INS_Std_Lon;
    }
    else {
      resolved.INS_Std_Lon = 0.0
    }

    if (msg.INS_Std_LocatHeight !== undefined) {
      resolved.INS_Std_LocatHeight = msg.INS_Std_LocatHeight;
    }
    else {
      resolved.INS_Std_LocatHeight = 0.0
    }

    if (msg.INS_Std_Heading !== undefined) {
      resolved.INS_Std_Heading = msg.INS_Std_Heading;
    }
    else {
      resolved.INS_Std_Heading = 0.0
    }

    return resolved;
    }
};

module.exports = GpsPosition;
