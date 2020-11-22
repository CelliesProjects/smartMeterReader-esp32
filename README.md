# smartMeterReader-esp32

![screenshot](screenshot.png)

## How to use.

1. Change your credentials in `wifisetup.h`.
1. (Optional) Change the i2c pin numbers for a connected ssd1306 oled screen in `smartMeterReader-esp32.ino` and flash your esp32.
2. Connect your esp32 to the smart meter. [See here how](https://github.com/matthijskooijman/arduino-dsmr#connecting-the-p1-port).<br>Take note that to connect to the esp32 the `DATA` signal has to be inverted and level shifted.
3. If you added a ssd1306 oled screen, the ip address will be visible on the screen.<br>If there is no oled you can check the ip address on the serial port in the Arduino IDE.
4. Browse to the ip address of your esp32 to see your current energy use.

## Needed libraries.

- [https://github.com/me-no-dev/AsyncTCP](https://github.com/me-no-dev/AsyncTCP)
- [https://github.com/me-no-dev/ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
- [https://github.com/matthijskooijman/arduino-dsmr](https://github.com/matthijskooijman/arduino-dsmr)

Download and install these in the Arduino libraries folder.

The driver library for the ssd1306 oled can be installed with the Arduino library manager.

## DSMR v5 P1 port standard.

[DSMR v5.0.2 P1 Companion Standard.pdf](https://github.com/matthijskooijman/arduino-dsmr/blob/master/specs/DSMR%20v5.0.2%20P1%20Companion%20Standard.pdf)
