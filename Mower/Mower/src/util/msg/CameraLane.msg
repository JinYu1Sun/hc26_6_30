Header      header

# fitted coefficient of lane 0, 1, 2, 3
# fitting equation: x = a * y ^ 2 + b * y + c
# (x, y in meters)

float32[4]  a
float32[4]  b
float32[4]  c
float32[4]  y_max
float32[4]  confidence # confidence of lane line detection, 0~100

#   0       1       2       3
#   0       1       2       3
#   0       1       2       3
#   0       1       2       3
#   0       1       2       3
#   0       1       2       3
#   0       1       2       3
#   0       1       2       3
#   0       1       2       3
#   0       1       2       3
#   0       1       2       3
#   0       1 ----- 2       3
#   0       1 |   | 2       3
#   0       1 |car| 2       3
#   0       1 |   | 2       3
#   0       1 ----- 2       3
#   0       1       2       3
