#!/usr/bin/env bash
# Restart ROS 1 nodes with generation-safe verification and bounded logs.
# Usage: restart_ros_nodes.sh [OPTIONS] NODE... -- COMMAND...

set -u

rotate_bounded_file() {
  local file="$1" max_bytes="$2" backups="$3" size i
  size="$(stat -c %s -- "$file" 2>/dev/null || printf '0')"
  (( size <= max_bytes )) && return 0
  for ((i = backups; i >= 2; --i)); do
    [[ -f "${file}.$((i - 1))" ]] &&
      mv -f -- "${file}.$((i - 1))" "${file}.${i}"
  done
  mv -f -- "$file" "${file}.1"
}

# Internal line-oriented sink used by process substitution below. One sink is
# created per roslaunch, so unrelated processes never share a file.
if [[ "${1:-}" == "--bounded-writer" ]]; then
  [[ $# -eq 4 ]] || exit 70
  writer_file="$2"
  writer_max_bytes="$3"
  writer_backups="$4"
  LC_ALL=C
  writer_size="$(stat -c %s -- "$writer_file" 2>/dev/null || printf '0')"
  mkdir -p -- "$(dirname -- "$writer_file")" || exit 70
  writer_active="$(dirname -- "$writer_file")/.active"
  : >"$writer_active"
  trap 'rm -f -- "$writer_active"' EXIT
  while IFS= read -r writer_line || [[ -n "${writer_line:-}" ]]; do
    writer_payload="${writer_line}"$'\n'
    while [[ -n "$writer_payload" ]]; do
      (( writer_size >= writer_max_bytes )) && {
        rotate_bounded_file "$writer_file" 0 "$writer_backups"
        writer_size=0
      }
      writer_available=$((writer_max_bytes - writer_size))
      writer_chunk="${writer_payload:0:writer_available}"
      printf '%s' "$writer_chunk" >>"$writer_file"
      writer_chunk_bytes=${#writer_chunk}
      ((writer_size += writer_chunk_bytes))
      writer_payload="${writer_payload:writer_chunk_bytes}"
    done
  done
  rotate_bounded_file "$writer_file" "$writer_max_bytes" "$writer_backups"
  exit 0
fi

delay_sec=2
ros_log_root="${ROS_HOME:-${HOME:-/tmp}/.ros}"
log_dir="${ros_log_root}/fusion_4dof_restarts"
summary_log="${ros_log_root}/fusion_4dof_restart.log"
wait_timeout_sec=5
startup_timeout_sec=15
max_summary_bytes=$((1024 * 1024))
max_process_bytes=$((8 * 1024 * 1024))
log_backups=2
max_transactions=12

while [[ $# -gt 0 ]]; do
  case "$1" in
    -d|--delay) [[ $# -ge 2 ]] || exit 2; delay_sec="$2"; shift 2 ;;
    -l|--log) [[ $# -ge 2 ]] || exit 2; summary_log="$2"; shift 2 ;;
    --log-dir) [[ $# -ge 2 ]] || exit 2; log_dir="$2"; shift 2 ;;
    --wait-timeout) [[ $# -ge 2 ]] || exit 2; wait_timeout_sec="$2"; shift 2 ;;
    --startup-timeout) [[ $# -ge 2 ]] || exit 2; startup_timeout_sec="$2"; shift 2 ;;
    --max-summary-bytes) [[ $# -ge 2 ]] || exit 2; max_summary_bytes="$2"; shift 2 ;;
    --max-process-bytes) [[ $# -ge 2 ]] || exit 2; max_process_bytes="$2"; shift 2 ;;
    --max-transactions) [[ $# -ge 2 ]] || exit 2; max_transactions="$2"; shift 2 ;;
    *) break ;;
  esac
done

for numeric_value in "$delay_sec" "$wait_timeout_sec" "$startup_timeout_sec" \
  "$max_summary_bytes" "$max_process_bytes" "$max_transactions"; do
  [[ "$numeric_value" =~ ^[0-9]+([.][0-9]+)?$ ]] || {
    echo "invalid numeric option: $numeric_value" >&2
    exit 2
  }
done
[[ "$max_summary_bytes" =~ ^[0-9]+$ && "$max_process_bytes" =~ ^[0-9]+$ &&
   "$max_transactions" =~ ^[0-9]+$ && "$wait_timeout_sec" =~ ^[0-9]+$ &&
   "$startup_timeout_sec" =~ ^[0-9]+$ ]] || {
  echo "timeout and log capacity options must be integers" >&2
  exit 2
}
(( max_summary_bytes > 0 && max_process_bytes > 0 &&
   max_transactions > 0 )) || {
  echo "log capacity options must be greater than zero" >&2
  exit 2
}

nodes=()
stale_nodes=()
while [[ $# -gt 0 && "$1" != "--" ]]; do
  [[ "$1" == /* ]] || { echo "ROS node name must be absolute: $1" >&2; exit 2; }
  nodes+=("$1")
  shift
done
[[ ${#nodes[@]} -gt 0 ]] || { echo "no ROS node specified" >&2; exit 2; }
[[ $# -gt 0 && "$1" == "--" ]] || { echo "missing -- separator" >&2; exit 2; }
shift
[[ $# -gt 0 ]] || { echo "missing restart command" >&2; exit 2; }

command -v rosnode >/dev/null 2>&1 || { echo "rosnode is not available" >&2; exit 127; }
command -v timeout >/dev/null 2>&1 || { echo "timeout is not available" >&2; exit 127; }

mkdir -p -- "$log_dir" "$(dirname -- "$summary_log")" || exit 70
rotate_bounded_file "$summary_log" "$max_summary_bytes" "$log_backups"

# Serialize the stop/start window. The child and bounded writer close fd 9, so
# they cannot retain this lock after this transaction exits.
exec 9>>"${log_dir}/.restart.lock"
if command -v flock >/dev/null 2>&1; then
  flock -x 9 || exit 70
fi

transaction_id="restart_$(date '+%Y%m%d_%H%M%S_%N')_$$"
transaction_dir="${log_dir}/${transaction_id}"
mkdir -p -- "$transaction_dir" || exit 70
transaction_log="${transaction_dir}/transaction.log"
process_log="${transaction_dir}/process.log"
: >"${transaction_dir}/.active"
writer_pid=""

prune_transactions() {
  local old_dir transaction_total
  local -a transaction_dirs=()
  mapfile -t transaction_dirs < <(
    find "$log_dir" -mindepth 1 -maxdepth 1 -type d \
      -name 'restart_*' -printf '%T@ %p\n' 2>/dev/null |
      sort -n | cut -d' ' -f2-)
  transaction_total=${#transaction_dirs[@]}
  for old_dir in "${transaction_dirs[@]}"; do
    (( transaction_total <= max_transactions )) && break
    [[ -f "${old_dir}/.active" ]] && continue
    if [[ "$old_dir" == "${log_dir}"/restart_* ]]; then
      rm -rf -- "$old_dir"
      ((transaction_total -= 1))
    fi
  done
}
prune_transactions

summary() {
  printf '[%s] transaction=%s %s\n' "$(date '+%F %T')" "$transaction_id" "$*" \
    >>"$summary_log"
}
detail() {
  printf '[%s] %s\n' "$(date '+%F %T')" "$*" >>"$transaction_log"
}
fail() {
  local code="$1" message="$2"
  [[ -n "${output_fifo:-}" && -p "${output_fifo:-}" ]] &&
    rm -f -- "$output_fifo"
  if [[ "$code" -eq 20 && -n "$writer_pid" ]]; then
    wait "$writer_pid" 2>/dev/null || true
  fi
  [[ -z "$writer_pid" ]] && rm -f -- "${transaction_dir}/.active"
  detail "FAILED exit=${code}: ${message}"
  summary "status=failed exit=${code} detail=${transaction_log} reason=${message}"
  prune_transactions
  echo "${message}; inspect ${transaction_log} and ${process_log}" >&2
  exit "$code"
}
node_reachable() { timeout 2s rosnode ping -c 1 "$1"; }
now_millis() { date +%s%3N; }
log_startup_diagnostics() {
  local current_nodes node registered reachable
  current_nodes="$(rosnode list 2>/dev/null || true)"
  for node in "${nodes[@]}"; do
    registered=0; reachable=0
    if grep -Fqx -- "$node" <<<"$current_nodes"; then
      registered=1
      node_reachable "$node" >/dev/null 2>&1 && reachable=1
    fi
    detail "startup node=${node} registered=${registered} reachable=${reachable}"
  done
}

summary "status=started detail=${transaction_log} process=${process_log}"
{
  printf '[%s] nodes:' "$(date '+%F %T')"
  printf ' %q' "${nodes[@]}"
  printf '; command:'
  printf ' %q' "$@"
  printf '\n'
} >>"$transaction_log"

for node in "${nodes[@]}"; do
  if rosnode list 2>/dev/null | grep -Fqx -- "$node"; then
    if ! rosnode kill "$node" >>"$transaction_log" 2>&1; then
      if ! node_reachable "$node" >>"$transaction_log" 2>&1; then
        stale_nodes+=("$node")
        detail "unreachable stale registration for ${node}; replacement allowed"
      else
        fail 21 "failed to request shutdown of live node ${node}"
      fi
    fi
  else
    detail "${node} is already absent"
  fi
done

deadline=$((SECONDS + wait_timeout_sec))
while (( SECONDS < deadline )); do
  remaining=0
  current_nodes="$(rosnode list 2>/dev/null || true)"
  for node in "${nodes[@]}"; do
    stale=0
    for stale_node in "${stale_nodes[@]}"; do
      [[ "$node" == "$stale_node" ]] && { stale=1; break; }
    done
    (( stale == 1 )) && continue
    grep -Fqx -- "$node" <<<"$current_nodes" && { remaining=1; break; }
  done
  (( remaining == 0 )) && break
  sleep 0.1
done

for node in "${nodes[@]}"; do
  stale=0
  for stale_node in "${stale_nodes[@]}"; do
    [[ "$node" == "$stale_node" ]] && { stale=1; break; }
  done
  (( stale == 1 )) && continue
  rosnode list 2>/dev/null | grep -Fqx -- "$node" &&
    fail 21 "node did not stop before timeout: ${node}"
done

detail "requested nodes stopped; restart delay=${delay_sec}s"
sleep "$delay_sec"
script_path="$(readlink -f -- "$0" 2>/dev/null || printf '%s' "$0")"
output_fifo="${transaction_dir}/process.pipe"
mkfifo -- "$output_fifo" || fail 70 "unable to create bounded-log pipe"
nohup "$script_path" --bounded-writer "$process_log" "$max_process_bytes" \
  "$log_backups" <"$output_fifo" >/dev/null 2>&1 9>&- &
writer_pid=$!
nohup "$@" </dev/null >"$output_fifo" 2>&1 9>&- &
launch_pid=$!
detail "restart command spawned pid=${launch_pid} writer_pid=${writer_pid}"

sleep 0.5
kill -0 "$launch_pid" 2>/dev/null ||
  fail 20 "restart command exited immediately"

deadline=$((SECONDS + startup_timeout_sec))
stable_since_ms=0
startup_verified=0
while (( SECONDS < deadline )); do
  missing=0
  current_nodes="$(rosnode list 2>/dev/null || true)"
  for node in "${nodes[@]}"; do
    grep -Fqx -- "$node" <<<"$current_nodes" || { missing=1; break; }
  done
  if (( missing == 0 )); then
    now_ms="$(now_millis)"
    if (( stable_since_ms == 0 )); then
      all_reachable=1
      for node in "${nodes[@]}"; do
        node_reachable "$node" >/dev/null 2>&1 || { all_reachable=0; break; }
      done
      (( all_reachable == 1 )) && stable_since_ms="$now_ms"
    elif (( now_ms - stable_since_ms >= 1000 )); then
      all_reachable=1
      for node in "${nodes[@]}"; do
        node_reachable "$node" >/dev/null 2>&1 || { all_reachable=0; break; }
      done
      if (( all_reachable == 1 )); then
        startup_verified=1
        break
      fi
      stable_since_ms=0
    fi
  else
    stable_since_ms=0
  fi
  kill -0 "$launch_pid" 2>/dev/null ||
    fail 20 "restart command exited before nodes registered"
  sleep 0.1
done

if (( startup_verified == 0 )); then
  log_startup_diagnostics
  fail 22 "new process is alive, but nodes were not stable within startup timeout"
fi
for node in "${nodes[@]}"; do
  if ! rosnode list 2>/dev/null | grep -Fqx -- "$node" ||
     ! node_reachable "$node" >/dev/null 2>&1; then
    log_startup_diagnostics
    fail 23 "node registered but final reachability failed: ${node}"
  fi
done

detail "restart verified pid=${launch_pid}"
rm -f -- "$output_fifo"
summary "status=verified pid=${launch_pid} detail=${transaction_log} process=${process_log}"
rotate_bounded_file "$summary_log" "$max_summary_bytes" "$log_backups"
prune_transactions
exit 0
