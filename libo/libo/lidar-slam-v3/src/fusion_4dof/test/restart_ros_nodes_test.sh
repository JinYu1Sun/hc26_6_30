#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
restart_script="${script_dir}/../scripts/restart_ros_nodes.sh"
[[ -x "$restart_script" ]] || {
  echo "restart_ros_nodes.sh must be executable so rosrun can launch it" >&2
  exit 1
}
test_root="$(mktemp -d)"
mock_bin="${test_root}/bin"
mkdir -p "$mock_bin"
export MOCK_NODE_STATE="${test_root}/nodes"
export MOCK_PIDS="${test_root}/pids"
export ROS_HOME="${test_root}/ros"
touch "$MOCK_NODE_STATE" "$MOCK_PIDS"

cleanup() {
  while IFS= read -r child_pid; do
    [[ "$child_pid" =~ ^[0-9]+$ ]] && kill "$child_pid" 2>/dev/null || true
  done <"$MOCK_PIDS"
  rm -rf -- "$test_root"
}
trap cleanup EXIT

cat >"${mock_bin}/rosnode" <<'MOCK'
#!/usr/bin/env bash
case "$1" in
  list) cat "$MOCK_NODE_STATE" ;;
  kill)
    old_pid="$(tail -n 1 "$MOCK_PIDS" 2>/dev/null || true)"
    [[ "$old_pid" =~ ^[0-9]+$ ]] && kill "$old_pid" 2>/dev/null || true
    : >"$MOCK_NODE_STATE"
    ;;
  ping) grep -Fqx -- "${@: -1}" "$MOCK_NODE_STATE" ;;
  *) exit 2 ;;
esac
MOCK
cat >"${mock_bin}/mock_launch" <<'MOCK'
#!/usr/bin/env bash
echo "$$" >>"$MOCK_PIDS"
printf '/fusion_4dof\n' >"$MOCK_NODE_STATE"
for i in $(seq 1 200); do printf 'bounded process output line %04d\n' "$i"; done
exec sleep 30
MOCK
cat >"${mock_bin}/alive_without_node" <<'MOCK'
#!/usr/bin/env bash
echo "$$" >>"$MOCK_PIDS"
exec sleep 30
MOCK
chmod +x "${mock_bin}/rosnode" "${mock_bin}/mock_launch" \
  "${mock_bin}/alive_without_node"
export PATH="${mock_bin}:${PATH}"

for run in 1 2 3; do
  "$restart_script" --delay 0 --startup-timeout 3 --max-summary-bytes 256 \
    --max-process-bytes 256 --max-transactions 2 /fusion_4dof -- mock_launch
done

transaction_count="$(find "${ROS_HOME}/fusion_4dof_restarts" -mindepth 1 \
  -maxdepth 1 -type d -name 'restart_*' | wc -l)"
[[ "$transaction_count" -eq 2 ]]
while IFS= read -r process_file; do
  [[ "$(stat -c %s "$process_file")" -le 256 ]]
done < <(find "${ROS_HOME}/fusion_4dof_restarts" -type f \
          -name 'process.log*')
[[ -f "${ROS_HOME}/fusion_4dof_restart.log.1" ]]

: >"$MOCK_NODE_STATE"
set +e
"$restart_script" --delay 0 --startup-timeout 1 /fusion_4dof -- \
  alive_without_node
partial_exit=$?
set -e
[[ "$partial_exit" -eq 22 ]]

set +e
"$restart_script" --delay 0 --startup-timeout 1 /fusion_4dof -- false
immediate_exit=$?
set -e
[[ "$immediate_exit" -eq 20 ]]

echo "restart_ros_nodes_test: PASS"
