Header header

uint32 seq_steer
float32 Steer_AngleActual
uint8 Steer_AngleSpeedActual
uint8 Steer_Flt2
uint8 Steer_Flt1
uint8 Steer_EnState

uint32 seq_drive
float32 Drive_ThrottlePedalActual
uint8 Drive_Flt2
uint8 Drive_Flt1
uint8 Drive_EnState

uint32 seq_brake
float32 Brake_PedalActual
uint8 Brake_Flt2
uint8 Brake_Flt1
uint8 Brake_EnState

uint32 seq_gear
uint8 Gear_Flt
uint8 Gear_Actual

uint32 seq_park
uint8 Park_Flt
uint8 Parking_Actual

uint32 seq_vcu
float32 Vehicle_Acc
float32 Vehicle_Speed
uint8 Vehicle_Errcode
uint8 CarWork_State
uint8 CarPower_State
uint8 AEB_Trigger_State
uint8 Brake_LightActual
uint8 TurnLight_Actual
uint8 Drive_ModeStatus
uint8 Steer_ModeStatus
uint8 Vehicle_ModeState
uint8 Vehicle_FrontCrashState
uint8 BackCrash_State
uint8 AEB_State

uint32 seq_wheelspeed
float32 WheelSpeedRR
float32 WheelSpeedRL
float32 WheelSpeedFR
float32 WheelSpeedFL

uint32 seq_bms
float32 Battery_Current
uint16 Battery_Voltage
uint8 BatteryLeadacid_Voltage
uint8 Battery_Soc

uint32 seq_door
uint8 DoorReportSta
uint8 DoorOpenTimeout
uint8 DoorOpenSta
uint8 DoorOpenInplace
uint8 DoorCloseTimeout
uint8 DoorCloseSta
uint8 DoorCloseInplace
uint8 DoorButtonEnable
