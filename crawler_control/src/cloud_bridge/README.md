# cloud_bridge —— 割草机云平台桥接功能包（ROS1 / Noetic）

连接云平台，实现：

- 远程遥控：前进 / 后退 / 转向，割草刀盘启停与高度调节（MQTT）
- 远程启动 / 停止 / 暂停 / 继续自动割草任务（MQTT）
- 开机持续上报定位与车辆状态（MQTT）
- 实时视频流推送（SRT 协议，H.264，可远程调 fps、分辨率、码率）
- 明文传输，无加密无认证

与云平台的接口协议见 [docs/CLOUD_INTERFACE.md](docs/CLOUD_INTERFACE.md)。

## 依赖安装

```bash
# MQTT / JSON / 图像
sudo apt install libpaho-mqtt-dev libpaho-mqttpp-dev nlohmann-json3-dev \
                 ros-noetic-cv-bridge

# 视频编码与 SRT 推流（ffmpeg 需带 libx264 和 libsrt 支持）
sudo apt install ffmpeg libsrt-openssl-dev
ffmpeg -protocols | grep srt    # 输出里有 srt 即支持
```

若 apt 源里没有 `libpaho-mqttpp-dev`，用源码安装（装到 /usr/local）：

```bash
# C 库
git clone https://github.com/eclipse/paho.mqtt.c.git && cd paho.mqtt.c
cmake -Bbuild -DPAHO_WITH_SSL=OFF && sudo cmake --build build --target install
# C++ 库
git clone https://github.com/eclipse/paho.mqtt.cpp.git && cd paho.mqtt.cpp
cmake -Bbuild && sudo cmake --build build --target install
sudo ldconfig
```

## 编译

本包在 crawler_control 工作空间内，依赖同空间的 `mower_msgs` 和 `util`（即 minibus_msg_util）：

```bash
cd ~/crawler_control
catkin_make -DCATKIN_WHITELIST_PACKAGES="mower_msgs;util;cloud_bridge"
source devel/setup.bash
```

注意：工作空间里 `pure_pursuit_syq` 与 `pure_pursuit` 是重名包（package.xml 的
`<name>` 都是 pure_pursuit），**不能整空间裸跑 `catkin_make`**，必须用上面的白名单
方式（要编译别的包时把包名加进白名单即可）。

## 启动

```bash
roslaunch cloud_bridge cloud_bridge.launch
```

所有参数（broker 地址、device_id、`srt_target` 推流地址、视频默认值、看门狗超时
等）都在 `launch/cloud_bridge.launch` 里，直接改 launch 文件即可。

视频源是 `/camera/image_rect`（可用 `image_topic` 参数改）。视频走 SRT：车端以
caller 模式向 `srt_target`（如 `srt://云平台IP:9000?mode=caller`）推 H.264 流，
云平台以 listener 模式监听收流。

## 验证清单（机载电脑 + mosquitto 模拟云平台）

1. `mosquitto_sub -t 'mower/mower_001/#' -v` 能看到 `state/location`（2Hz）和 `state/vehicle`（1Hz）上报
2. 发 `cmd/move {"linear":0.3,"angular":0}` → `rostopic echo /vehicle/cmd` 有非零 drive_value；停止发送 0.5s 后自动回零（看门狗）
3. 发 `cmd/blade {"state":1}` → `/vehicle/cmd` 的 mover_bool=1
4. 发 `cmd/task {"action":"start","map_name":"xxx"}` → `rostopic echo /signal` 依次看到 start_execution/reset/use_map/single_map/start_work
5. 发 `cmd/task {"action":"stop"}` → /signal 收到 stop 和 stop_execution
6. 视频：云平台侧先起监听 `ffplay -fflags nobuffer "srt://0.0.0.0:9000?mode=listener"`，车端发 `cmd/video {"enable":true}` 后应能看到画面；改 fps/width/height/bitrate 后流自动重启生效
