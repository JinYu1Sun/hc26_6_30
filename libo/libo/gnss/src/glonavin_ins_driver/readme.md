格纳微车载组合导航ROS驱动。

编译方法：
```
catkin_make -DCATKIN_WHITELIST_PACKAGES=gps_imu
```

已配置为hex2协议。
运行前记得在launch文件中确认串口设备路径`port`和波特率`baudrate`。

运行方法：
```
roslaunch gps_imu gnss_imu.launch
```