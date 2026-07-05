#GpsPosition
Header header
int32 plat_id			#标识此消息的平台id
int32 error_code		#错误标识码
int32 gps_flag			#GPS状态
int32 positionStatus		#系统运行状态

uint32 gps_week			#GPS Week

uint32 gps_millisecond		#GPS millisecond in a week

float64 longitude		#经纬度，单位为度
float64 latitude
float64 height			#海拔,单位为m

float64 gaussX			#高斯投影位置,cm
float64 gaussY


float64 pitch			#俯仰角,x轴方向(正右方)右手定则四指方向为正,单位为0.01度
float64 roll			#翻滚角,y轴方向(正前方)右手定则四指方向为正,单位为0.01度
float64 azimuth			#航向角，单位为0.01度,向东为零度，逆时针0-360

float64 acc_x # m/s2
float64 acc_y
float64 acc_z

float64 rot_x # deg/s
float64 rot_y
float64 rot_z

float64 northVelocity		#north速度，单位为cm/s
float64 eastVelocity		#east速度,单位为cm/s
float64 upVelocity		#up速度,单位为cm/s

int32 gps_confidence	#gps定位精度置信度,根据卫星数量和INS,POS解算状态综合得出, 8-10为良好,6-7为一般,小于等于5为信号差,0为没有信号

uint8 INS_GpsFlag_Pos
uint8 INS_NumSV
uint8 INS_GpsFlag_Heading
uint8 INS_Gps_Age
uint8 INS_Car_Status
uint8 INS_Status
uint8 INS_VehicleAlign
float64 INS_Std_Lat
float64 INS_Std_Lon
float64 INS_Std_LocatHeight
float64 INS_Std_Heading