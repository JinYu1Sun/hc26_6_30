uint32 point_id
float64 x # m
float64 y # m
float64 s # m
float64 speed # km/h
float64 heading # deg, east=0, north=90
float64 upper_boundary # left boundary, m, left > 0
float64 lower_boundary # right boundary, m, right < 0

uint8 isStoppingStation
# 0: not a stopping station, don't open door
# 1: is a stopping station, should open door

uint8 station_index
uint8 station_num

float64 ndt_x # m
float64 ndt_y # m
float64 ndt_heading # deg, east=0, north=90

float64 planning_upper_limit # left planning space, left > 0
float64 planning_lower_limit # right planning space, right < 0
