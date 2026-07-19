uint8 ID

float32 Range
	# m

float32 Speed
	# m/s

float32 Angle
	# deg

float32 LatVel
	# m/s

uint8   DynProp
	# 0.unknown
	# 1.coming
	# 2.leaving
	# 3.still

float32 SNR
	# dB

float32 Height
	# m

float32 PitchAngle
	# deg

float32 ExistProb
	# 0~2.55

float32 ACCLon
	# m/s2

float32 ACCLat
	# m/s2

uint16  ObjType
	# bit0: absolutely still
	# bit1: left guardrail
	# bit2: right guardrail
	# bit3: manhole cover
	# bit4: 
	# bit5: mid lane absolutely still
	# bit7:6: motion attribute
	#	0:unknown; 1:coming; 2:leaving; 3:still
	# bit10:8:
	#	0:invalid; 1:person; 2:bicycle; 3:sedan; 4:truck
	# bit11: moving object
	# bit12: object from behind

float32 ObjLength
	# m

float32 ObjWidth
	# m

float32 ObjXrms
	# m

float32 ObjYrms
	# m

float32 ObjVxRms
	# m/s

float32 ObjVyRms
	# m/s

float32 ObjAxRms
	# m/s2

float32 ObjAyRms
	# m/s2

float32 ObjAngleRms
	# deg

uint8 ObjMeasState
	# 0: not exist
	# 1: new
	# 2: measured
	# 3: predicted
	# 4: is going to disappear
