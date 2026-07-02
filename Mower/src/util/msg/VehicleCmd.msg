#HAD
uint8 	Ord_Brake		#EPB驻车状态使能
uint8 HADAutoDriveEnable	#VCU自动驾驶使能
uint8 VCUAutoDriveEnable	#VCU自动驾驶使能
uint8 ESCAutoDriveEnable
uint8 EPSAutoDriveEnable 	#VCU自动驾驶使能
uint8 EPBAutoDriveEnable	#VCU自动驾驶使能
uint8 BCMAutoDriveEnable	#VCU自动驾驶使能
float32 ElectricAcPedal		#模拟加速踏板开度
float32 WhellAngle		#模拟方向盘转角 
uint16 WhellAngleAccleration	#模拟方向盘转角加速度	
float32 BrakeAccleration	#模拟制动减速度
uint8 AutoDrive_Gear		#自动驾驶档位命令

uint8 FrontWiperCtrlCmd     #  前雨刮控制命令
uint8 RearWiperCtrlCmd      #  后雨刮控制命令
uint8 LowBeamCtrlCmd        #  近光灯控制命令
uint8 HighBeamCtrlCmd       #  远光灯控制命令
uint8 BackFogLightCtrlCmd   #  后雾灯控制命令
uint8 FrontFogLightCtrlCmd  #  前雾灯控制命令
uint8 PositionLampCtrlCmd   #  位置灯控制命令
uint8 LeftLightCtrlCmd      #  左转向控制命令
uint8 RightLightCtrlCmd     #  右转向控制命令
uint8 HarzadCtrlCmd         #  危险报警灯控制命令
uint8 BrakeLampCtrlCmd      #  刹车灯控制命令
uint8 ReversingLampCtrlCmd  #  倒车灯控制命令
uint8 AlarmHornCtrlCmd      #  喇叭控制命令
uint8 DoorLockCmd           #  门锁控制命令

