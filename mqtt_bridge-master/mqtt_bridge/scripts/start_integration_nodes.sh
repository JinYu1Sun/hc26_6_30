#!/usr/bin/env bash

set -u

WORKSPACE="${NANOCARGO_WORKSPACE:-/home/ubuntu/trj/catkin_nanocargo_J10A}"
ROS_SETUP="/opt/ros/humble/setup.bash"
WORKSPACE_SETUP="${WORKSPACE}/install/setup.bash"
RUNTIME_DIR="${TMPDIR:-/tmp}/nanocargo_integration_nodes_${USER:-ubuntu}"
LOG_DIR="${RUNTIME_DIR}/logs"
PID_DIR="${RUNTIME_DIR}/pids"

COMPONENTS=(
  system_health
  remote_control
  core_status
  command_fusion
  cloud_client
  nanocargo_manager
  mqtt_bridge
)

usage()
{
  cat <<'EOF'
业务节点启动与监控脚本

已明确排除：object_perception_simple、pbox_node_driver、ros_can_bridge、routing

用法：
  start_integration_nodes.sh start [秒]   启动全部节点并持续监控，默认3秒刷新
  start_integration_nodes.sh monitor [秒] 持续监控已启动节点
  start_integration_nodes.sh status       查看一次节点状态
  start_integration_nodes.sh logs         查看全部节点最近日志
  start_integration_nodes.sh logs 名称    持续查看指定节点日志
  start_integration_nodes.sh stop         停止本脚本启动的全部节点

节点名称：
  mqtt_bridge cloud_client remote_control core_status command_fusion
  system_health nanocargo_manager

环境变量：
  NANOCARGO_WORKSPACE  工作空间路径
EOF
}

component_command()
{
  case "$1" in
    mqtt_bridge) printf '%s' "ros2 launch mqtt_bridge mqtt_bridge.launch.py" ;;
    cloud_client) printf '%s' "ros2 launch angmen_cloud_client angmen_cloud_clientlaunch.py" ;;
    remote_control) printf '%s' "ros2 launch angmen_ad_remote_control remote_control.launch.py" ;;
    core_status) printf '%s' "ros2 launch angmen_ad_core_status core_status_launch.py" ;;
    command_fusion) printf '%s' "ros2 launch angmen_ad_command_fusion command_fusion_launch.py" ;;
    system_health) printf '%s' "ros2 launch system_health_monitor system_health_monitor.launch.py" ;;
    nanocargo_manager) printf '%s' "ros2 launch nanocargo_manager nanocargo_manager.launch" ;;
    *) return 1 ;;
  esac
}

expected_ros_node()
{
  case "$1" in
    mqtt_bridge) printf '%s' "/mqtt_bridge" ;;
    cloud_client) printf '%s' "/angmen_cloud_client_node" ;;
    remote_control) printf '%s' "/remote_control_node" ;;
    core_status) printf '%s' "/core_status_node" ;;
    command_fusion) printf '%s' "/command_fusion_node" ;;
    system_health) printf '%s' "/system_health_monitor" ;;
    nanocargo_manager) printf '%s' "/task_manager_node" ;;
    *) return 1 ;;
  esac
}

ensure_environment()
{
  if [[ ! -f "${ROS_SETUP}" ]]; then
    echo "错误：找不到ROS环境 ${ROS_SETUP}" >&2
    return 1
  fi
  if [[ ! -f "${WORKSPACE_SETUP}" ]]; then
    echo "错误：找不到工作空间环境 ${WORKSPACE_SETUP}，请先编译。" >&2
    return 1
  fi
  set +u
  # shellcheck disable=SC1090
  source "${ROS_SETUP}"
  # shellcheck disable=SC1090
  source "${WORKSPACE_SETUP}"
  set -u
  mkdir -p "${LOG_DIR}" "${PID_DIR}"
}

pid_file_for() { printf '%s/%s.pid' "${PID_DIR}" "$1"; }
log_file_for() { printf '%s/%s.log' "${LOG_DIR}" "$1"; }

component_pid()
{
  local pid_file pid
  pid_file="$(pid_file_for "$1")"
  [[ -f "${pid_file}" ]] || return 1
  pid="$(<"${pid_file}")"
  [[ "${pid}" =~ ^[0-9]+$ ]] || return 1
  kill -0 "${pid}" 2>/dev/null || return 1
  printf '%s' "${pid}"
}

start_component()
{
  local name="$1" command log_file pid_file pid
  if pid="$(component_pid "${name}")"; then
    echo "[跳过] ${name} 已运行，PID=${pid}"
    return 0
  fi
  command="$(component_command "${name}")" || return 1
  log_file="$(log_file_for "${name}")"
  pid_file="$(pid_file_for "${name}")"
  : > "${log_file}"
  nohup setsid bash -lc \
    "source '${ROS_SETUP}'; source '${WORKSPACE_SETUP}'; exec ${command}" \
    > "${log_file}" 2>&1 &
  pid=$!
  printf '%s\n' "${pid}" > "${pid_file}"
  echo "[启动] ${name} PID=${pid} 日志=${log_file}"
}

refresh_ros_nodes()
{
  mapfile -t ROS_NODES < <(ros2 node list 2>/dev/null | sort -u)
}

ros_node_exists()
{
  local expected="$1"
  printf '%s\n' "${ROS_NODES[@]:-}" | grep -Fxq -- "${expected}"
}

component_state()
{
  local name="$1" expected pid_file
  pid_file="$(pid_file_for "${name}")"
  if [[ ! -f "${pid_file}" ]]; then
    printf '%s' "未启动"
    return
  fi
  if ! component_pid "${name}" >/dev/null; then
    printf '%s' "启动失败/退出"
    return
  fi
  expected="$(expected_ros_node "${name}")"
  if ros_node_exists "${expected}"; then
    printf '%s' "正常"
  else
    printf '%s' "ROS节点缺失"
  fi
}

show_status()
{
  refresh_ros_nodes
  printf '%-20s %-14s %-9s %-30s %s\n' "模块" "状态" "PID" "ROS节点" "日志"
  printf '%-20s %-14s %-9s %-30s %s\n' "------------------" "------------" "-------" "----------------------------" "----"
  local name pid state expected
  for name in "${COMPONENTS[@]}"; do
    pid="$(component_pid "${name}" 2>/dev/null || true)"
    state="$(component_state "${name}")"
    expected="$(expected_ros_node "${name}")"
    printf '%-20s %-14s %-9s %-30s %s\n' \
      "${name}" "${state}" "${pid:--}" "${expected}" "$(log_file_for "${name}")"
  done
}

show_failed_log()
{
  local name="$1" log_file
  log_file="$(log_file_for "${name}")"
  echo
  echo "===== ${name} 异常，日志末尾 ====="
  tail -n 40 "${log_file}" 2>/dev/null || echo "暂无日志"
}

monitor_nodes()
{
  local interval="${1:-3}"
  if [[ ! "${interval}" =~ ^[1-9][0-9]*$ ]]; then
    echo "错误：刷新间隔必须是正整数秒。" >&2
    return 1
  fi
  local interrupted=0 name state
  declare -A previous_state=()
  trap 'interrupted=1' INT TERM
  echo "开始监控，刷新间隔 ${interval}s；按 Ctrl+C 退出监控，节点继续运行。"
  while ((interrupted == 0)); do
    refresh_ros_nodes
    printf '\n[%(%F %T)T]\n' -1
    show_status
    for name in "${COMPONENTS[@]}"; do
      state="$(component_state "${name}")"
      if [[ "${state}" != "正常" && "${previous_state[${name}]:-}" != "${state}" ]]; then
        show_failed_log "${name}"
      fi
      previous_state["${name}"]="${state}"
    done
    sleep "${interval}" || true
  done
  trap - INT TERM
}

start_all()
{
  local name
  for name in "${COMPONENTS[@]}"; do
    start_component "${name}"
  done
  echo "等待ROS节点注册..."
  sleep 4
  monitor_nodes "${1:-3}"
}

show_logs()
{
  local requested="${1:-all}" name log_file
  if [[ "${requested}" != "all" ]]; then
    component_command "${requested}" >/dev/null || {
      echo "错误：未知模块 ${requested}" >&2
      return 1
    }
    log_file="$(log_file_for "${requested}")"
    [[ -f "${log_file}" ]] || { echo "暂无日志：${log_file}" >&2; return 1; }
    tail -n 100 -f "${log_file}"
    return
  fi
  for name in "${COMPONENTS[@]}"; do
    log_file="$(log_file_for "${name}")"
    [[ -f "${log_file}" ]] || continue
    echo
    echo "===== ${name}: ${log_file} ====="
    tail -n 30 "${log_file}"
  done
}

stop_all()
{
  local name pid pid_file
  for name in "${COMPONENTS[@]}"; do
    pid_file="$(pid_file_for "${name}")"
    if pid="$(component_pid "${name}" 2>/dev/null)"; then
      echo "[停止] ${name} PID=${pid}"
      kill -TERM -- "-${pid}" 2>/dev/null || kill -TERM "${pid}" 2>/dev/null || true
    fi
  done
  sleep 2
  for name in "${COMPONENTS[@]}"; do
    pid_file="$(pid_file_for "${name}")"
    if pid="$(component_pid "${name}" 2>/dev/null)"; then
      echo "[强制停止] ${name} PID=${pid}"
      kill -KILL -- "-${pid}" 2>/dev/null || kill -KILL "${pid}" 2>/dev/null || true
    fi
    rm -f "${pid_file}"
  done
}

main()
{
  local action="${1:-help}"
  ensure_environment || return 1
  case "${action}" in
    start) start_all "${2:-3}" ;;
    monitor) monitor_nodes "${2:-3}" ;;
    status) show_status ;;
    logs) show_logs "${2:-all}" ;;
    stop) stop_all ;;
    help|-h|--help) usage ;;
    *) echo "错误：未知命令 ${action}" >&2; usage; return 1 ;;
  esac
}

main "$@"
