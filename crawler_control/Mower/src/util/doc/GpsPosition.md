#GpsPosition
Header header
int32 plat_id			#标识此消息的平台id，目前没有使用
int32 error_code		#错误标识码，目前没有使用
int32 gps_flag			#GPS状态，目前没有使用
int32 positionStatus		#系统运行状态，目前没有使用

uint32 gps_week			#GPS Week

uint32 gps_millisecond		#GPS millisecond in a week

float64 longitude		#经纬度，单位为度
float64 latitude        #经纬度，单位为度
float64 height			#海拔,单位为m

int32 gaussX			#高斯投影位置,北方向，cm
int32 gaussY            #高斯投影位置,东方向，cm


int32 pitch			#俯仰角,x轴方向(正右方)右手定则四指方向为正,单位为0.01度
int32 roll			#翻滚角,y轴方向(正前方)右手定则四指方向为正,单位为0.01度
int32 azimuth			#航向角，单位为0.01度,向东为零度，逆时针0-360

int32 northVelocity		#north速度，单位为cm/s
int32 eastVelocity		#east速度,单位为cm/s
int32 upVelocity		#up速度,单位为cm/s

int32 gps_confidence	#gps定位精度置信度,由惯导给出的标准差计算得到，一般认为9和10属于良好，低于9的认为精度不可用

