Header header

uint32 seq_drive
float32 Drive_SpeedTarget
float32 Drive_Acc
float32 Drive_ThrottlePedalTarget
uint8 Drive_EnCtrl

uint32 seq_brake
float32 Brake_Dec
float32 Brake_Pedal_Target
uint8 AEB_EnCtrl
uint8 Brake_EnCtrl

uint32 seq_steer
float32 Steer_AngleTarget
uint8 Steer_AngleSpeed
uint8 Steer_EnCtrl

uint32 seq_gear
uint8 Gear_Target
uint8 Gear_EnCtrl

uint32 seq_park
uint8 Park_Target
uint8 Park_EnCtrl

uint32 seq_vehicle_mode
uint8 frame_to_send
uint8 Vehicle_DoorCtrl
uint8 TurnLight_Ctrl
uint8 Vehicle_VIN_Req
uint8 Drive_ModeCtrl
uint8 Steer_ModeCtrl

uint8 linux_algo_ready
uint8 linux_algo_auto_en_req
uint8 linux_algo_js_en_req