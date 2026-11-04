#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
发送 sysid_recorder 测试配置的小工具。

用法：
  rosrun sysid_recorder send_sysid_config.py <profile|json文件|stop>

示例：
  rosrun sysid_recorder send_sysid_config.py static_forward_coarse
  rosrun sysid_recorder send_sysid_config.py static_forward_fine
  rosrun sysid_recorder send_sysid_config.py static_turn
  rosrun sysid_recorder send_sysid_config.py coupling
  rosrun sysid_recorder send_sysid_config.py stop
  rosrun sysid_recorder send_sysid_config.py /path/to/my_test.json
"""

import json
import os
import sys

import rospy
from std_msgs.msg import String


# -----------------------------------------------------------------------------
# 下面是一组辅助函数，用于快速构造常用波形配置
# -----------------------------------------------------------------------------
def _stair(offset, min_amp, max_amp, step_amp, step_time=5.0, stop_time=1.0):
    return {
        "type": "stair",
        "offset": offset,
        "min_amp": min_amp,
        "max_amp": max_amp,
        "step_amp": step_amp,
        "step_time": step_time,
        "stop_time": stop_time,
    }


def _stair_manual(offset, amplitudes, step_time=5.0, stop_time=1.0):
    return {
        "type": "stair",
        "offset": offset,
        "amplitudes": amplitudes,
        "step_time": step_time,
        "stop_time": stop_time,
    }


def _step(offset, A, T=2.0):
    return {"type": "step", "offset": offset, "A": A, "T": T}


def _dwell(offset, A, Tr=1.0, Th=3.0, Tz=2.0):
    return {"type": "dwell", "offset": offset, "A": A, "Tr": Tr, "Th": Th, "Tz": Tz}


def _sine(offset, A, B, T=0.0):
    return {"type": "sine", "offset": offset, "A": A, "B": B, "T": T}


def _const(offset, A):
    return {"type": "const", "offset": offset, "A": A}


def _multisine_drive(offset, A2, A4, T0=60, fc=20):
    return {
        "type": "multisine",
        "offset": offset,
        "A2": A2,
        "A4": A4,
        "T0": T0,
        "fc": fc,
        "P_ZERO": 0.10,
        "P_PEAK": 0.12,
    }


def _multisine_turn(offset, A1, A3, A5, T0=60, fc=20):
    return {
        "type": "multisine",
        "offset": offset,
        "A1": A1,
        "A3": A3,
        "A5": A5,
        "T0": T0,
        "fc": fc,
        "P_ZERO": 0.10,
        "P_PEAK": 0.12,
    }


# -----------------------------------------------------------------------------
# 预定义测试配置。命名规则：
#   static_ / dynamic_ / coupling_ / baseline_ / const_ 开头
#   forward / turn 表示通道
#   coarse / medium / fine / deadzone 表示扫描粒度
#   small / medium / large 表示激励幅值
# -----------------------------------------------------------------------------
PROFILES = {
    # =====================================================================
    # 1. 静态标定：stair 阶梯，覆盖死区到饱和区
    # =====================================================================
    "static_forward_coarse": {
        "mode": "static_forward",
        "drive": _stair(1500, -300, 300, 10),
    },
    "static_forward_medium": {
        "mode": "static_forward",
        "drive": _stair(1500, -200, 200, 20),
    },
    "static_forward_fine": {
        "mode": "static_forward",
        "drive": _stair(1500, -50, 50, 5),
    },
    "static_forward_deadzone": {
        "mode": "static_forward",
        "drive": _stair(1500, -20, 20, 2, step_time=5.0, stop_time=3.0),
    },
    "static_forward_asymmetric": {
        "mode": "static_forward",
        "drive": _stair_manual(
            1500, [10, -10, 30, -30, 80, -20, 150, -40, 250, -60],
            step_time=5.0, stop_time=1.0
        ),
    },

    "static_turn_coarse": {
        "mode": "static_turn",
        "drive": _const(1500, 0),
        "turn": _stair(1500, -330, 330, 10),
    },
    "static_turn_medium": {
        "mode": "static_turn",
        "drive": _const(1500, 0),
        "turn": _stair(1500, -200, 200, 20),
    },
    "static_turn_fine": {
        "mode": "static_turn",
        "drive": _const(1500, 0),
        "turn": _stair(1500, -50, 50, 5),
    },
    "static_turn_deadzone": {
        "mode": "static_turn",
        "drive": _const(1500, 0),
        "turn": _stair(1500, -20, 20, 2, step_time=5.0, stop_time=3.0),
    },

    # =====================================================================
    # 2. 动态辨识：step / dwell，不同幅值
    # =====================================================================
    "dynamic_forward_step_small": {
        "mode": "dynamic_forward",
        "drive": _step(1500, -100, T=2.0),
    },
    "dynamic_forward_step_medium": {
        "mode": "dynamic_forward",
        "drive": _step(1500, -200, T=2.0),
    },
    "dynamic_forward_step_large": {
        "mode": "dynamic_forward",
        "drive": _step(1500, -300, T=2.0),
    },
    "dynamic_forward_dwell_small": {
        "mode": "dynamic_forward",
        "drive": _dwell(1500, -100, Tr=1.0, Th=3.0, Tz=2.0),
    },
    "dynamic_forward_dwell_medium": {
        "mode": "dynamic_forward",
        "drive": _dwell(1500, -200, Tr=1.0, Th=3.0, Tz=2.0),
    },
    "dynamic_forward_dwell_large": {
        "mode": "dynamic_forward",
        "drive": _dwell(1500, -300, Tr=1.0, Th=3.0, Tz=2.0),
    },

    "dynamic_turn_step_small": {
        "mode": "dynamic_turn",
        "drive": _const(1500, 0),
        "turn": _step(1500, 100, T=2.0),
    },
    "dynamic_turn_step_medium": {
        "mode": "dynamic_turn",
        "drive": _const(1500, 0),
        "turn": _step(1500, 200, T=2.0),
    },
    "dynamic_turn_step_large": {
        "mode": "dynamic_turn",
        "drive": _const(1500, 0),
        "turn": _step(1500, 300, T=2.0),
    },
    "dynamic_turn_dwell_small": {
        "mode": "dynamic_turn",
        "drive": _const(1500, 0),
        "turn": _dwell(1500, 100, Tr=1.0, Th=3.0, Tz=2.0),
    },
    "dynamic_turn_dwell_medium": {
        "mode": "dynamic_turn",
        "drive": _const(1500, 0),
        "turn": _dwell(1500, 200, Tr=1.0, Th=3.0, Tz=2.0),
    },
    "dynamic_turn_dwell_large": {
        "mode": "dynamic_turn",
        "drive": _const(1500, 0),
        "turn": _dwell(1500, 300, Tr=1.0, Th=3.0, Tz=2.0),
    },

    # =====================================================================
    # 3. 正弦扫频：同一幅值，不同频率
    # =====================================================================
    "sine_forward_slow": {
        "mode": "dynamic_forward",
        "drive": _sine(1500, -100, B=0.5),
    },
    "sine_forward_medium": {
        "mode": "dynamic_forward",
        "drive": _sine(1500, -100, B=1.0),
    },
    "sine_forward_fast": {
        "mode": "dynamic_forward",
        "drive": _sine(1500, -100, B=2.0),
    },

    "sine_turn_slow": {
        "mode": "dynamic_turn",
        "drive": _const(1500, 0),
        "turn": _sine(1500, 100, B=0.5),
    },
    "sine_turn_medium": {
        "mode": "dynamic_turn",
        "drive": _const(1500, 0),
        "turn": _sine(1500, 100, B=1.0),
    },
    "sine_turn_fast": {
        "mode": "dynamic_turn",
        "drive": _const(1500, 0),
        "turn": _sine(1500, 100, B=2.0),
    },

    # =====================================================================
    # 4. 耦合辨识：multisine，不同幅值/周期
    # =====================================================================
    "coupling_small": {
        "mode": "coupling",
        "drive": _multisine_drive(1500, A2=100, A4=50, T0=60),
        "turn": _multisine_turn(1500, A1=150, A3=50, A5=50, T0=60),
    },
    "coupling_medium": {
        "mode": "coupling",
        "drive": _multisine_drive(1500, A2=200, A4=100, T0=60),
        "turn": _multisine_turn(1500, A1=300, A3=100, A5=100, T0=60),
    },
    "coupling_large": {
        "mode": "coupling",
        "drive": _multisine_drive(1500, A2=300, A4=150, T0=60),
        "turn": _multisine_turn(1500, A1=450, A3=150, A5=150, T0=60),
    },
    "coupling_long": {
        "mode": "coupling",
        "drive": _multisine_drive(1500, A2=200, A4=100, T0=120),
        "turn": _multisine_turn(1500, A1=300, A3=100, A5=100, T0=120),
    },

    # =====================================================================
    # 5. 基线/稳态参考：常值指令
    # =====================================================================
    "baseline_zero": {
        "mode": "static_forward",
        "drive": _const(1500, 0),
        "turn": _const(1500, 0),
    },
    "const_forward_low": {
        "mode": "static_forward",
        "drive": _const(1500, -100),
    },
    "const_forward_mid": {
        "mode": "static_forward",
        "drive": _const(1500, -200),
    },
    "const_forward_high": {
        "mode": "static_forward",
        "drive": _const(1500, -300),
    },
    "const_turn_left": {
        "mode": "static_turn",
        "drive": _const(1500, 0),
        "turn": _const(1500, -150),
    },
    "const_turn_right": {
        "mode": "static_turn",
        "drive": _const(1500, 0),
        "turn": _const(1500, 150),
    },

    # =====================================================================
    # 6. 停车
    # =====================================================================
    "stop": {"mode": "stop"},
}


def load_config(arg):
    """解析输入：可能是预定义 profile、json 文件路径，或 stop。"""
    if arg in PROFILES:
        return PROFILES[arg]

    if not os.path.isfile(arg):
        rospy.logerr("不认识的名字，也不是文件: %s", arg)
        rospy.logerr("可用 profile: %s", ", ".join(PROFILES.keys()))
        sys.exit(1)

    with open(arg, "r", encoding="utf-8") as f:
        cfg = json.load(f)
    return cfg


def main():
    rospy.init_node("send_sysid_config", anonymous=True)

    if len(sys.argv) < 2:
        print(__doc__)
        print("可用 profile:")
        for name in PROFILES:
            print(f"  {name}")
        sys.exit(0)

    arg = sys.argv[1]
    cfg = load_config(arg)

    topic = rospy.get_param("~config_topic", "/sysid_recorder/config")
    pub = rospy.Publisher(topic, String, queue_size=1, latch=True)

    # 等 publisher 建立连接，避免消息丢失
    rospy.sleep(0.3)

    msg = String()
    msg.data = json.dumps(cfg, separators=(",", ":"))
    pub.publish(msg)

    rospy.loginfo("已发送配置到 %s: %s", topic, msg.data)


if __name__ == "__main__":
    main()
