Header header

# AGVStatus 0x301  现履带车底控无反馈，均不成立
float64 speed_status  
float64 leftw_speed   
float64 rightw_speed
uint8 gear_model      
uint8 drive_mode
uint8 mow_height
uint8 battery_soc

#gear_model
uint8 P_Gear=0 # 0x00： P 档
uint8 R_Gear=1 # 0x01： 倒档R
uint8 N_Gear=2 # 0x02： N 档
uint8 D_Gear=3 # 0x03： D 档
#drive_model
uint8 Manualcontrol_mode=0 # 0x00：手动控制模式
uint8 Automaticcontrol_mode=1 # 0x01：自动控制模式
uint8 Remotecontrol_mode=2 # 0x02：遥控器调试模式
#mow_height
uint8 Low_mode=2 # 0x02：割草高度为低
uint8 Medium_mode=1 # 0x01：割草高度为中
uint8 High_mode=0 # 0x00：割草高度为高

