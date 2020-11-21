# smartMeterReader-esp32

![alt](screenshot.png)

## How to use:

1. Change your credentials in `wifisetup.h` and flash your esp32.
2. Connect your esp32 to the smart meter. [See here how](https://github.com/matthijskooijman/arduino-dsmr).<br>Take note that to connect to the esp32 the `DATA` signal has to be inverted and level shifted.
3. If you added an ssd1306 oled screen, the ip address should be visible on the display.<br>If you did not add the oled you can check the ip address on the serial port in the Arduino IDE.
4. Browse to the ip address of your esp32 to see your current energy use.
