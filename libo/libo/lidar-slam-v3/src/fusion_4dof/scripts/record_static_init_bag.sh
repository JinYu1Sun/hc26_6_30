#!/usr/bin/env bash
# Record the complete cold-start/static-initialization timing chain.
#
# Accurate use:
#   1. Start this script before restarting localization processes.
#   2. Wait until it prints "recording started".
#   3. Restart localization (or arrange for this script to start before the
#      localization launch during a full-machine boot).
#   4. Keep the mower completely stationary for the whole recording.
#
# Usage:
#   record_static_init_bag.sh [OUTPUT_DIR] [DURATION_SEC] [LABEL]
# Example:
#   record_static_init_bag.sh /home/nvidia/bags 180 cold_boot_static

set -u

output_dir="${1:-${PWD}/static_init_bags}"
duration_sec="${2:-180}"
label="${3:-static_init}"

if [[ ! "$duration_sec" =~ ^[1-9][0-9]*$ ]]; then
  printf 'DURATION_SEC must be a positive integer, got: %s\n' "$duration_sec" >&2
  exit 2
fi
if [[ ! "$label" =~ ^[A-Za-z0-9._-]+$ ]]; then
  printf 'LABEL may contain only letters, numbers, dot, underscore and dash.\n' >&2
  exit 2
fi
command -v rosbag >/dev/null 2>&1 || {
  printf 'rosbag is not available; source the ROS 1 workspace first.\n' >&2
  exit 127
}
command -v rosnode >/dev/null 2>&1 || {
  printf 'rosnode is not available; source the ROS 1 workspace first.\n' >&2
  exit 127
}

mkdir -p -- "$output_dir" || exit 1
output_dir="$(cd "$output_dir" && pwd -P)"
run_id="${label}_$(date '+%Y%m%d_%H%M%S')"
bag_prefix="${output_dir}/${run_id}"
metadata_dir="${bag_prefix}_metadata"
mkdir -p -- "$metadata_dir" || exit 1

{
  printf 'run_id=%s\n' "$run_id"
  printf 'script_start_iso=%s\n' "$(date --iso-8601=ns)"
  printf 'script_start_unix_ns=%s\n' "$(date +%s%N)"
  printf 'host_uptime_sec=%s\n' "$(cut -d' ' -f1 /proc/uptime 2>/dev/null || true)"
  printf 'duration_sec=%s\n' "$duration_sec"
  printf 'output_prefix=%s\n' "$bag_prefix"
  printf 'ros_master_uri=%s\n' "${ROS_MASTER_URI:-unset}"
  printf 'ros_ip=%s\n' "${ROS_IP:-unset}"
  printf 'ros_hostname=%s\n' "${ROS_HOSTNAME:-unset}"
  uname -a
} >"${metadata_dir}/run.txt"

printf 'Waiting for ROS master at %s ...\n' "${ROS_MASTER_URI:-http://localhost:11311}"
while ! rosnode list >/dev/null 2>&1; do
  sleep 0.2
done
printf 'ROS master is available. Keep the mower completely stationary.\n'

snapshot_ros_graph() {
  local suffix="$1"
  local node node_file
  date --iso-8601=ns >"${metadata_dir}/${suffix}_time.txt"
  rosnode list >"${metadata_dir}/${suffix}_nodes.txt" 2>&1 || true
  rostopic list -v >"${metadata_dir}/${suffix}_topics.txt" 2>&1 || true
  rosparam dump "${metadata_dir}/${suffix}_params.yaml" >/dev/null 2>&1 || true
  for node in /laserMapping /fusion_4dof /android_manager_4dof; do
    node_file="${node#/}"
    rosnode info "$node" >"${metadata_dir}/${suffix}_${node_file}.txt" 2>&1 || true
  done
}

snapshot_ros_graph before

# Raw LiDAR is intentionally included. Its first receive/header timestamp is
# needed to distinguish late sensor availability from repeated IMU-static
# window rejection. LZ4 limits disk bandwidth without changing timestamps.
topics=(
  /livox/imu
  /livox/lidar
  /Mower/lio_static_initialized
  /Mower/lio_static_calibration
  /Mower/lio_level_transform
  /Mower/lio_slam
  /Mower/lio_slam_level
  /Odometry
  /nanobot/gpsposition
  /Mower/position
  /Mower/localization_algorithm_status
  /Mower/localization_status
  /Mower/imu_odometry/status
  /Mower/imu_odometry/ready
  /Mower/debug/gps
  /Mower/debug/lio
  /Mower/debug/rtk_seed
  /mower/fusion_stop_car
  /mower/manager_stop_car
  /mower/stop_car
  /vehicle/cmd
  /vehicle/status
  /vehicle/left_wheel_speed
  /vehicle/right_wheel_speed
  /Mower/reboot
  /nanobot/reboot
  /signal
  /diagnostics
  /rosout
  /rosout_agg
  /tf
  /tf_static
)
printf '%s\n' "${topics[@]}" >"${metadata_dir}/requested_topics.txt"

compression_args=()
if rosbag record --help 2>&1 | grep -q -- '--lz4'; then
  compression_args=(--lz4)
fi

recorder_pid=''
cleanup() {
  local exit_code=$?
  trap - EXIT INT TERM
  if [[ -n "$recorder_pid" ]] && kill -0 "$recorder_pid" 2>/dev/null; then
    kill -INT "$recorder_pid" 2>/dev/null || true
    wait "$recorder_pid" || true
  fi
  snapshot_ros_graph after
  bag_file="${bag_prefix}.bag"
  if [[ -f "$bag_file" ]]; then
    rosbag info --yaml "$bag_file" >"${metadata_dir}/bag_info.yaml" 2>&1 ||
      rosbag info "$bag_file" >"${metadata_dir}/bag_info.txt" 2>&1 || true
  fi
  {
    printf 'script_end_iso=%s\n' "$(date --iso-8601=ns)"
    printf 'script_end_unix_ns=%s\n' "$(date +%s%N)"
    printf 'host_uptime_sec=%s\n' "$(cut -d' ' -f1 /proc/uptime 2>/dev/null || true)"
  } >>"${metadata_dir}/run.txt"
  printf '\nRecording finished. Send both of these to the analyst:\n'
  printf '  %s\n' "$bag_file"
  printf '  %s\n' "$metadata_dir"
  exit "$exit_code"
}
trap cleanup EXIT INT TERM

printf 'Starting %ss recording: %s.bag\n' "$duration_sec" "$bag_prefix"
rosbag record "${compression_args[@]}" --buffsize=2048 \
  --duration="${duration_sec}" -O "$bag_prefix" "${topics[@]}" &
recorder_pid=$!

# Give rosbag time to register all subscriptions before the operator restarts
# localization. A process-generation stamp still provides the authoritative
# start instant, but this delay prevents losing the first raw sensor packets.
sleep 2
if ! kill -0 "$recorder_pid" 2>/dev/null; then
  wait "$recorder_pid"
  exit $?
fi
printf '\n=== recording started ===\n'
printf 'Now restart localization if this is a restart test. Do not move the mower.\n'
printf 'The recording will stop automatically after %s seconds; Ctrl-C is also safe.\n\n' \
  "$duration_sec"
: >"${metadata_dir}/recording_ready"

wait "$recorder_pid"
recorder_status=$?
recorder_pid=''
exit "$recorder_status"
