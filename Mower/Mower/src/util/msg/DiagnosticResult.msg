Header header

# Diagnostic status code
# 0: Normal operation
# 1: No position data (/Mower/position)
# 2: Stop car command received (/mower/stop_car=true)
# 3: Emergency stop for obstacle avoidance (/lawn_mower/avoid_state==3)
# 4: No global path (/lawn_mower/global_path)
# 5: PID controller node not running (pid_controller node)
# 6: CAN communication buffer full (fallback check when others are normal)
uint8 fault_code 