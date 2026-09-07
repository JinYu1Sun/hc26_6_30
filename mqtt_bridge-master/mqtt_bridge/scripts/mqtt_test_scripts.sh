#!/usr/bin/env bash

set -u

WORKSPACE="${NANOCARGO_WORKSPACE:-/home/ubuntu/trj/catkin_nanocargo_J10A}"
ROS_SETUP="/opt/ros/humble/setup.bash"
WORKSPACE_SETUP="${WORKSPACE}/install/setup.bash"
SCRIPT_DIR="${WORKSPACE}/src/mqtt_bridge/scripts"
RUNTIME_DIR="${TMPDIR:-/tmp}/nanocargo_mqtt_test_${USER:-ubuntu}"
LOG_DIR="${RUNTIME_DIR}/logs"
LOG_FILE="${LOG_DIR}/path_mock.log"
PID_FILE="${RUNTIME_DIR}/path_mock.pid"

usage()
{
  cat <<'EOF'
MQTT联调ROS模拟脚本入口

用法：
  mqtt_test_scripts.sh start             启动路径规划模拟服务并查看日志
  mqtt_test_scripts.sh status            查看路径规划模拟服务状态
  mqtt_test_scripts.sh logs              持续查看路径规划模拟服务日志
  mqtt_test_scripts.sh stop              停止路径规划模拟服务
  mqtt_test_scripts.sh events [参数]     发布车辆事件
  mqtt_test_scripts.sh takeover [参数]   发布车辆主动接管请求
  mqtt_test_scripts.sh arrival [参数]    发布event_type=139到站事件

所有模拟脚本只收发ROS，VIN和MQTT协议字段由mqtt_bridge处理。
EOF
}

ensure_environment()
{
  [[ -f "${ROS_SETUP}" ]] || { echo "错误：找不到 ${ROS_SETUP}" >&2; return 1; }
  [[ -f "${WORKSPACE_SETUP}" ]] || { echo "错误：找不到 ${WORKSPACE_SETUP}" >&2; return 1; }
  set +u
  # shellcheck disable=SC1090
  source "${ROS_SETUP}"
  # shellcheck disable=SC1090
  source "${WORKSPACE_SETUP}"
  set -u
  mkdir -p "${RUNTIME_DIR}" "${LOG_DIR}"
}

run_logged()
{
  local name="$1"
  shift
  local timestamp log_file result
  timestamp="$(date '+%Y%m%d_%H%M%S')"
  log_file="${LOG_DIR}/${name}_${timestamp}.log"
  echo "日志保存到：${log_file}"
  "$@" 2>&1 | tee "${log_file}"
  result="${PIPESTATUS[0]}"
  return "${result}"
}

mock_pid()
{
  local pid
  [[ -f "${PID_FILE}" ]] || return 1
  pid="$(<"${PID_FILE}")"
  [[ "${pid}" =~ ^[0-9]+$ ]] || return 1
  kill -0 "${pid}" 2>/dev/null || return 1
  printf '%s' "${pid}"
}

start_mock()
{
  local pid
  if pid="$(mock_pid)"; then
    echo "路径规划模拟服务已运行，PID=${pid}"
  else
    : > "${LOG_FILE}"
    nohup setsid bash -lc \
      "source '${ROS_SETUP}'; source '${WORKSPACE_SETUP}'; exec python3 '${SCRIPT_DIR}/mock_bridge_services.py'" \
      > "${LOG_FILE}" 2>&1 &
    pid=$!
    printf '%s\n' "${pid}" > "${PID_FILE}"
    echo "已启动路径规划模拟服务，PID=${pid}，日志=${LOG_FILE}"
    sleep 2
    if ! mock_pid >/dev/null; then
      echo "启动失败，日志如下：" >&2
      tail -n 60 "${LOG_FILE}" >&2
      return 1
    fi
  fi
  echo "持续显示日志，按 Ctrl+C 退出查看，服务继续运行。"
  tail -n 100 -f "${LOG_FILE}"
}

show_status()
{
  local pid
  if pid="$(mock_pid)"; then
    if ros2 node list 2>/dev/null | grep -Fxq -- "/mqtt_bridge_service_mocks"; then
      echo "路径规划模拟服务：正常，PID=${pid}"
    else
      echo "路径规划模拟服务：进程存在，但ROS节点未发现，PID=${pid}"
    fi
  else
    echo "路径规划模拟服务：未运行"
  fi
  echo "日志：${LOG_FILE}"
}

stop_mock()
{
  local pid
  if pid="$(mock_pid)"; then
    kill -TERM -- "-${pid}" 2>/dev/null || kill -TERM "${pid}" 2>/dev/null || true
    echo "已停止路径规划模拟服务，PID=${pid}"
  fi
  rm -f "${PID_FILE}"
}

main()
{
  local action="${1:-help}"
  shift || true
  ensure_environment || return 1
  case "${action}" in
    start) start_mock ;;
    status) show_status ;;
    logs) [[ -f "${LOG_FILE}" ]] && tail -n 100 -f "${LOG_FILE}" || echo "暂无日志：${LOG_FILE}" ;;
    stop) stop_mock ;;
    events)
      run_logged events python3 "${SCRIPT_DIR}/publish_car_events.py" --count 2 "$@"
      ;;
    takeover)
      run_logged takeover python3 "${SCRIPT_DIR}/publish_takeover_request.py" \
        --request 0 --reason 1 --reason-description "车云联调主动接管测试" "$@"
      ;;
    arrival)
      run_logged arrival python3 "${SCRIPT_DIR}/publish_car_events.py" \
        --level 3 --event-type 139 --description "任务执行完成-当前任务成功完成" --count 1 "$@"
      ;;
    help|-h|--help) usage ;;
    *) echo "错误：未知命令 ${action}" >&2; usage; return 1 ;;
  esac
}

main "$@"
