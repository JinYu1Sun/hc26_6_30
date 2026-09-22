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

class Position {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.header = null;
      this.position_x = null;
      this.position_y = null;
      this.position_z = null;
      this.roll = null;
      this.pitch = null;
      this.yaw = null;
      this.position_state = null;
    }
    else {
      if (initObj.hasOwnProperty('header')) {
        this.header = initObj.header
      }
      else {
        this.header = new std_msgs.msg.Header();
      }
      if (initObj.hasOwnProperty('position_x')) {
        this.position_x = initObj.position_x
      }
      else {
        this.position_x = 0.0;
      }
      if (initObj.hasOwnProperty('position_y')) {
        this.position_y = initObj.position_y
      }
      else {
        this.position_y = 0.0;
      }
      if (initObj.hasOwnProperty('position_z')) {
        this.position_z = initObj.position_z
      }
      else {
        this.position_z = 0.0;
      }
      if (initObj.hasOwnProperty('roll')) {
        this.roll = initObj.roll
      }
      else {
        this.roll = 0.0;
      }
      if (initObj.hasOwnProperty('pitch')) {
        this.pitch = initObj.pitch
      }
      else {
        this.pitch = 0.0;
      }
      if (initObj.hasOwnProperty('yaw')) {
        this.yaw = initObj.yaw
      }
      else {
        this.yaw = 0.0;
      }
      if (initObj.hasOwnProperty('position_state')) {
        this.position_state = initObj.position_state
      }
      else {
        this.position_state = 0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type Position
    // Serialize message field [header]
    bufferOffset = std_msgs.msg.Header.serialize(obj.header, buffer, bufferOffset);
    // Serialize message field [position_x]
    bufferOffset = _serializer.float64(obj.position_x, buffer, bufferOffset);
    // Serialize message field [position_y]
    bufferOffset = _serializer.float64(obj.position_y, buffer, bufferOffset);
    // Serialize message field [position_z]
    bufferOffset = _serializer.float64(obj.position_z, buffer, bufferOffset);
    // Serialize message field [roll]
    bufferOffset = _serializer.float64(obj.roll, buffer, bufferOffset);
    // Serialize message field [pitch]
    bufferOffset = _serializer.float64(obj.pitch, buffer, bufferOffset);
    // Serialize message field [yaw]
    bufferOffset = _serializer.float64(obj.yaw, buffer, bufferOffset);
    // Serialize message field [position_state]
    bufferOffset = _serializer.uint32(obj.position_state, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type Position
    let len;
    let data = new Position(null);
    // Deserialize message field [header]
    data.header = std_msgs.msg.Header.deserialize(buffer, bufferOffset);
    // Deserialize message field [position_x]
    data.position_x = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [position_y]
    data.position_y = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [position_z]
    data.position_z = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [roll]
    data.roll = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [pitch]
    data.pitch = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [yaw]
    data.yaw = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [position_state]
    data.position_state = _deserializer.uint32(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += std_msgs.msg.Header.getMessageSize(object.header);
    return length + 52;
  }

  static datatype() {
    // Returns string type for a message object
    return 'util/Position';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'b162b5fae3a9f45d1742394d3c0bb3a6';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    # 通用定位输出。修改注释不会改变ROS消息MD5；字段、类型和顺序不得随意修改，
    # 否则所有util/Position生产者、消费者必须同步重新编译和部署。
    Header header
    
    # 位置单位：m；姿态单位：rad。
    # 当前fusion_4dof在/Mower/position上发布的是“base_link在地图ENU中的位姿”：
    # position_x=East，position_y=North，position_z=Up；roll/pitch/yaw遵循ROS
    # base_link约定。为兼容既有下游，header.frame_id当前仍填"base_link"，因此不能
    # 仅按frame_id把position_*误解为车体系坐标。
    # 旧SE2生产者的地图坐标定义不同；两版并行对照时必须使用不同输出话题，且不能
    # 不经坐标转换直接逐点相减。
    float64 position_x
    float64 position_y
    float64 position_z
    float64 roll
    float64 pitch
    float64 yaw
    
    # /Mower/position的position_state取值（调试话题可能复用该字段表达自身布尔状态，
    # 不得把调试话题的0/1直接套用以下生产定位语义）：
    #   0 = 无有效生产定位/保留值；不得驱动车辆作业
    #   1 = GPS/RTK正常：x/y/yaw以GPS/组合导航为主；同步RTK短暂缺帧的配置宽限内，
    #       使用“最后可信RTK锚点+LIO相对增量”短时延拓；z使用LIO相对高程+稳健tz
    #   2 = 4DoF LIO回退，水平yaw已锁定（经过二维展布充分的鲁棒Umeyama拟合）
    #   3 = 纯GPS调试模式（pub_fusion==2），不得等同于正常自动融合状态1
    #   4 = 纯LIO调试模式（pub_fusion==3）；只有完整4DoF有效时才具有全局ENU语义
    #   5 = 4DoF LIO回退，水平yaw仅播种未锁定（沿迹基线或RTK航向种子）；短时
    #       可用，但长直线长时间丢RTK时误差风险高于状态2
    #   6 = MID360 IMU受限积分里程计；仅用于制动期位姿连续，
    #       不得自动作业，超过时间/距离/数据新鲜度门限立即失效
    #   9 = 航向尚未初始化（heading_flag_为假时覆盖输出状态）；不得驱动车辆作业
    # 当前生产控制中只有1/2/5属于“有效可用位置”；所有按状态判断定位有效性的
    # 消费者都必须显式包含5，并可对状态5采用更保守的速度/作业策略。
    uint32 position_state
    
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
    const resolved = new Position(null);
    if (msg.header !== undefined) {
      resolved.header = std_msgs.msg.Header.Resolve(msg.header)
    }
    else {
      resolved.header = new std_msgs.msg.Header()
    }

    if (msg.position_x !== undefined) {
      resolved.position_x = msg.position_x;
    }
    else {
      resolved.position_x = 0.0
    }

    if (msg.position_y !== undefined) {
      resolved.position_y = msg.position_y;
    }
    else {
      resolved.position_y = 0.0
    }

    if (msg.position_z !== undefined) {
      resolved.position_z = msg.position_z;
    }
    else {
      resolved.position_z = 0.0
    }

    if (msg.roll !== undefined) {
      resolved.roll = msg.roll;
    }
    else {
      resolved.roll = 0.0
    }

    if (msg.pitch !== undefined) {
      resolved.pitch = msg.pitch;
    }
    else {
      resolved.pitch = 0.0
    }

    if (msg.yaw !== undefined) {
      resolved.yaw = msg.yaw;
    }
    else {
      resolved.yaw = 0.0
    }

    if (msg.position_state !== undefined) {
      resolved.position_state = msg.position_state;
    }
    else {
      resolved.position_state = 0
    }

    return resolved;
    }
};

module.exports = Position;
