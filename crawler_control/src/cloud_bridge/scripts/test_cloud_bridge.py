#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
cloud_bridge 云平台公网联调测试脚本（模拟云平台侧）

协议依据: docs/CLOUD_INTERFACE.md
  - 主题前缀: mower/{device_id}/, payload 为 UTF-8 JSON
  - 上行(车->云): state/location, state/vehicle   (QoS 0)
  - 下行(云->车): cmd/move, cmd/blade, cmd/task    (QoS 1)
  - 车端 0.5s 收不到 move 指令会自动停车(看门狗)

使用前确认:
  1. 车端 cloud_bridge.launch 的 broker_host/broker_port 指向公网 broker
  2. 本脚本与车端使用同一个 broker、同一个 device_id

用法示例:
  python test_cloud_bridge.py check                       # 被动检查上行是否到达(10s)
  python test_cloud_bridge.py sub                         # 持续打印所有上行消息
  python test_cloud_bridge.py move --linear 0.3 -t 3      # 前进3秒(默认10Hz),随后发停车
  python test_cloud_bridge.py blade --state 1 --height 6  # 开刀盘
  python test_cloud_bridge.py task --action start --map-name map_0630
  python test_cloud_bridge.py watchdog                    # 看门狗测试:发2s move后停发
  python test_cloud_bridge.py --host 1.2.3.4 sub          # 覆盖 broker 地址
"""

import argparse
import json
import sys
import threading
import time

import paho.mqtt.client as mqtt

DEFAULT_HOST = "broker.emqx.io"   # 云平台公网 broker(取自 mqtt_config.yaml)
DEFAULT_PORT = 1883
DEFAULT_USER = ""
DEFAULT_PASS = ""


def make_client(args, on_message=None):
    client = mqtt.Client(
        mqtt.CallbackAPIVersion.VERSION2,
        client_id=args.client_id or f"cloud_test_{int(time.time())}",
    )
    if args.user:
        client.username_pw_set(args.user, args.password or "")
    if on_message:
        client.on_message = on_message

    rc_holder = {}

    def on_connect(c, userdata, flags, rc, properties=None):
        rc_holder["rc"] = rc
        if rc == 0:
            print(f"[OK] 已连接 {args.host}:{args.port}")
        else:
            print(f"[FAIL] 连接被拒绝 rc={rc}", file=sys.stderr)

    client.on_connect = on_connect
    client.connect(args.host, args.port, keepalive=30)
    client.loop_start()
    t0 = time.time()
    while "rc" not in rc_holder and time.time() - t0 < 5:
        time.sleep(0.05)
    if rc_holder.get("rc") != 0:
        print("[FAIL] 无法连接 broker,请检查地址/端口/账号", file=sys.stderr)
        sys.exit(1)
    return client


def pub(client, topic, payload):
    data = json.dumps(payload)
    info = client.publish(topic, data, qos=1)
    info.wait_for_publish(timeout=3)
    print(f"[PUB] {topic} {data}")


def fmt_msg(topic, payload):
    try:
        obj = json.loads(payload.decode("utf-8"))
        return f"{topic}  {json.dumps(obj, ensure_ascii=False)}"
    except Exception:
        return f"{topic}  <非JSON, {len(payload)}字节>"


def cmd_sub(args):
    def on_message(c, u, msg):
        print(f"[{time.strftime('%H:%M:%S')}] {fmt_msg(msg.topic, msg.payload)}")

    client = make_client(args, on_message)
    client.subscribe(f"{args.prefix}/{args.device_id}/#", qos=1)
    print(f"订阅 {args.prefix}/{args.device_id}/# ,Ctrl+C 退出")
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        pass
    finally:
        client.loop_stop()
        client.disconnect()


def cmd_check(args):
    stats = {}
    lock = threading.Lock()
    samples = {}

    def on_message(c, u, msg):
        key = msg.topic
        with lock:
            stats.setdefault(key, []).append(time.time())
            samples[key] = fmt_msg(msg.topic, msg.payload)

    client = make_client(args, on_message)
    client.subscribe(f"{args.prefix}/{args.device_id}/#", qos=1)
    print(f"监听 {args.duration}s ...")
    time.sleep(args.duration)
    client.loop_stop()
    client.disconnect()

    print("\n===== 检查结果 =====")
    if not stats:
        print("未收到任何上行消息。请检查:")
        print("  1. 车端 cloud_bridge 是否已启动且 broker_host 指向公网地址")
        print("  2. device_id 是否一致(当前: %s)" % args.device_id)
        print("  3. 车端机器能否访问公网 broker %s:%s" % (args.host, args.port))
        return 1
    now = time.time()
    for topic, ts_list in sorted(stats.items()):
        dur = max(ts_list[-1] - ts_list[0], 1e-6)
        rate = (len(ts_list) - 1) / dur if len(ts_list) > 1 else 0
        age = now - ts_list[-1]
        print(f"{topic}: {len(ts_list)} 条, 约 {rate:.1f} Hz, 最近一条 {age:.1f}s 前")
        print(f"  示例: {samples[topic]}")
    expect = [f"{args.prefix}/{args.device_id}/state/location",
              f"{args.prefix}/{args.device_id}/state/vehicle"]
    missing = [t for t in expect if t not in stats]
    if missing:
        print("缺少预期话题: " + ", ".join(missing))
        return 1
    print("上行链路正常。")
    return 0


def cmd_move(args):
    client = make_client(args)
    topic = f"{args.prefix}/{args.device_id}/cmd/move"
    payload = {"linear": args.linear, "angular": args.angular}
    interval = 1.0 / args.hz
    n = int(args.duration * args.hz)
    print(f"以 {args.hz}Hz 发送 {args.duration}s: {payload}")
    for i in range(n):
        pub(client, topic, payload) if i == 0 or (i + 1) % args.hz == 0 else \
            client.publish(topic, json.dumps(payload), qos=1)
        time.sleep(interval)
    pub(client, topic, {"linear": 0, "angular": 0})
    print("已发送停车指令。")
    client.loop_stop()
    client.disconnect()


def cmd_blade(args):
    client = make_client(args)
    topic = f"{args.prefix}/{args.device_id}/cmd/blade"
    payload = {"state": args.state}
    if args.height is not None:
        payload["height"] = args.height
    pub(client, topic, payload)
    client.loop_stop()
    client.disconnect()


def cmd_task(args):
    client = make_client(args)
    topic = f"{args.prefix}/{args.device_id}/cmd/task"
    payload = {"action": args.action}
    if args.action == "start":
        if args.map_name:
            payload["map_name"] = args.map_name
        payload["map_mode"] = args.map_mode
    pub(client, topic, payload)
    client.loop_stop()
    client.disconnect()


def cmd_watchdog(args):
    client = make_client(args)
    topic = f"{args.prefix}/{args.device_id}/cmd/move"
    payload = {"linear": args.linear, "angular": 0}
    print(f"发送 move {args.duration}s 后【突然停发】,车端应在 0.5s 内自动停车")
    interval = 1.0 / args.hz
    for _ in range(int(args.duration * args.hz)):
        client.publish(topic, json.dumps(payload), qos=1)
        time.sleep(interval)
    print("已停发。请到车端观察 /vehicle/cmd 是否在 0.5s 后自动回零。")
    client.loop_stop()
    client.disconnect()


def main():
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--host", default=DEFAULT_HOST)
    p.add_argument("--port", type=int, default=DEFAULT_PORT)
    p.add_argument("--user", default=DEFAULT_USER)
    p.add_argument("--password", default=DEFAULT_PASS)
    p.add_argument("--device-id", default="mower_001")
    p.add_argument("--prefix", default="mower")
    p.add_argument("--client-id", default="")
    sub = p.add_subparsers(dest="cmd", required=True)

    s = sub.add_parser("sub", help="持续打印所有上行消息")
    s.set_defaults(func=cmd_sub)

    s = sub.add_parser("check", help="被动检查上行链路与上报频率")
    s.add_argument("-d", "--duration", type=float, default=10)
    s.set_defaults(func=cmd_check)

    s = sub.add_parser("move", help="遥控移动(持续发送后停车)")
    s.add_argument("--linear", type=float, default=0.3)
    s.add_argument("--angular", type=float, default=0.0)
    s.add_argument("-t", "--duration", type=float, default=3.0)
    s.add_argument("--hz", type=float, default=10.0)
    s.set_defaults(func=cmd_move)

    s = sub.add_parser("blade", help="刀盘控制")
    s.add_argument("--state", type=int, required=True, choices=[0, 1])
    s.add_argument("--height", type=int, default=None)
    s.set_defaults(func=cmd_blade)

    s = sub.add_parser("task", help="自动割草任务控制")
    s.add_argument("--action", required=True,
                   choices=["start", "stop", "pause", "continue"])
    s.add_argument("--map-name", default="")
    s.add_argument("--map-mode", default="single_map",
                   choices=["single_map", "multi_map"])
    s.set_defaults(func=cmd_task)

    s = sub.add_parser("watchdog", help="看门狗测试(停发后应自动停车)")
    s.add_argument("--linear", type=float, default=0.3)
    s.add_argument("-t", "--duration", type=float, default=2.0)
    s.add_argument("--hz", type=float, default=10.0)
    s.set_defaults(func=cmd_watchdog)

    args = p.parse_args()
    sys.exit(args.func(args) or 0)


if __name__ == "__main__":
    main()
