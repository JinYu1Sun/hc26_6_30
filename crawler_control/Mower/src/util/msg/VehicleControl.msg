Header header
# VehicleCmd
# Gear_model
uint8 p_Gear = 0 # 0x00： P 档
uint8 R_Gear = 1 # 0x01： 无
uint8 N_Gear = 2 # 0x02： 转弯信号
uint8 D_Gear = 3 # 0x03： D 前进档

# ADCmd 0x310
uint8 mover_bool
uint8 ad_control_enable 
uint8 gear_model
uint16 turn_value
uint16 drive_value
uint8 mower_height

float64 left_wheel_speed  # 无
float64 right_wheel_speed # 无
float64 speed_cmd   # int8 [-128~127]





