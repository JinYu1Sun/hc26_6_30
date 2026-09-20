#!/bin/bash

source "$HOME/Mower/devel/setup.bash"
# ==============================
# ROS Bag 后台录制脚本
# ==============================

# Bag 保存目录
BAG_DIR="$HOME/Mower"

# PID 文件
PID_FILE="$HOME/Mower/rosbag_record.pid"

# 录制的话题
TOPICS=(
    "/lawn_mower/grid_map"
    "/lawn_mower/obstacle_polygon"
    "/lawn_mower/view_polygon"
    "/mower_perception/obstacles"
    "/debug/global_trajectory"
    "/debug/avoid_trajectory"
    "/debug/local_trajectory"
    "/tf"
    "/mower/stop_car"
    "/mower/stop_car2"
    "/lawn_mower/avoid_state"
)

# ==============================
# 创建保存目录
# ==============================
mkdir -p "$BAG_DIR"


# ==============================
# 检查是否正在录制
# ==============================
is_recording()
{
    if [ -f "$PID_FILE" ]; then
        PID=$(cat "$PID_FILE")

        if kill -0 "$PID" 2>/dev/null; then
            return 0
        else
            rm -f "$PID_FILE"
        fi
    fi

    return 1
}


# ==============================
# 开始录制
# ==============================
start_record()
{
    if is_recording; then
        PID=$(cat "$PID_FILE")
        echo "rosbag 已经在录制中"
        echo "PID: $PID"
        return 1
    fi

    TIME=$(date +"%Y%m%d_%H%M%S")

    BAG_NAME="${BAG_DIR}/wsy_${TIME}"

    echo "======================================"
    echo "开始录制 ROS Bag"
    echo "保存路径: $BAG_NAME"
    echo "======================================"

    echo "录制话题："

    for topic in "${TOPICS[@]}"
    do
        echo "  $topic"
    done

    echo ""

    # 后台启动 rosbag
    rosbag record \
        -O "$BAG_NAME" \
        "${TOPICS[@]}" \
        > "${BAG_NAME}.log" 2>&1 &

    PID=$!

    echo "$PID" > "$PID_FILE"

    echo "rosbag PID: $PID"
    echo "日志文件: ${BAG_NAME}.log"
    echo ""
    echo "录制已在后台运行"
    echo "停止录制:"
    echo "    $0 stop"
}


# ==============================
# 停止录制
# ==============================
stop_record()
{
    if ! is_recording; then
        echo "当前没有正在录制的 rosbag"
        return 0
    fi

    PID=$(cat "$PID_FILE")

    echo "正在停止 rosbag..."
    echo "PID: $PID"

    # 给 rosbag SIGINT，让 rosbag 正常关闭
    kill -SIGINT "$PID"

    # 等待 rosbag 正常退出
    for i in {1..20}
    do
        if ! kill -0 "$PID" 2>/dev/null; then
            break
        fi

        sleep 0.5
    done

    # 如果还没退出，强制结束
    if kill -0 "$PID" 2>/dev/null; then
        echo "rosbag 未正常退出，强制终止..."
        kill -SIGTERM "$PID"
    fi

    rm -f "$PID_FILE"

    echo "rosbag 录制已停止"
}


# ==============================
# 查看状态
# ==============================
status_record()
{
    if is_recording; then
        PID=$(cat "$PID_FILE")

        echo "======================================"
        echo "rosbag 正在录制"
        echo "PID: $PID"
        echo "======================================"

        ps -fp "$PID"
    else
        echo "当前没有录制 rosbag"
    fi
}

# ==============================
# 主程序
# ==============================
case "$1" in

    start)
        start_record
        ;;

    stop)
        stop_record
        ;;

    status)
        status_record
        ;;

    restart)
        stop_record
        sleep 1
        start_record
        ;;

    *)
        echo "用法:"
        echo ""
        echo "  $0 start      开始后台录制"
        echo "  $0 stop       停止录制"
        echo "  $0 status     查看录制状态"
        echo "  $0 restart    重启录制"
        echo ""
        exit 1
        ;;

esac
