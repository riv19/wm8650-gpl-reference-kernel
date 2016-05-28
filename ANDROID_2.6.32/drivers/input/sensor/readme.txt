wmt.io.gsensor
Configure G-Sensor for Enable/Disable,X,Y,Z axis mapping, sampling rate, and GPIO pin for interrupt. If driver not detect this variable, then G-sensor driver won¡¦t be loaded.
<op>:<gpio>:<freq>:<axis-X>:<X-dir>:<axis-Y>:<Y-dir>:<axis-Z>:<Z-dir>
<op>:= Operation mode for the g-sensor
0 : G-Sensor is disabled.
1 : G-Sensor is enabled.
<gpio>:= GPIO pin number. The valid values are 0~3 for GPIO0~GPIO3.
<freq>:= G-Sensor sampling rate. The valid values are 1,2,4,8,16,32,64,and 128.
<axis-X>:= G-Sensor axis-x will map to which axis of device.
0 : X
1 : Y
2 : Z
<X-dir>:= If G-Sensor axis-x direction is the same as device.
1 : Positive direction
-1 : Negative direction
<axis-Y>:= G-Sensor axis-y will map to which axis of device.
0 : X
1 : Y
2 : Z
<Y-dir>:= If G-Sensor axis-y direction is the same as device.
1 : Positive direction
-1 : Negative direction
<axis-Z>:= G-Sensor axis-z will map to which axis of device.
0 : X
1 : Y
2 : Z
<Z-dir>:= If G-Sensor axis-z direction is the same as device.
1 : Positive direction
-1 : Negative direction
Ex:
#G-sensor enabled, using GPIO2 as interrupt for G-Sensor, 16 sampling rate, X„³-Y, Y„³X, Z„³-Z
setenv wmt.io.gsensor 1:2:16:1:-1:0:1:2:-1
