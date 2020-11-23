# smartMeterReader-esp32

## About

`smartMeterReader-esp32` is a serial to websocket bridge between your DSMR 5 smartmeter and the rest of your network.<br>
Websocket clients connected to `ws://ip-address/raw` get the raw dsmr telegram pushed every second.<br>
Also included is a http webserver so you can see your current energy use and the daily usage figures on your smartphone or pc browser.<br>
No app required!

![screenshot](screenshot.png)

Very convenient is that an esp32 flashed with `smartMeterReader-esp32` can be run from the 5V supplied by the smartmeter if equiped with a proper WiFi antenna.<br>The TTGO T7 is a nice board with an external antenna connector. 

![T7 pic](t7.jpg)

## How to use

1. Change your credentials in `wifisetup.h`.
1. (Optional) Change the i2c pin numbers for a connected ssd1306 oled screen in `smartMeterReader-esp32.ino`.
3. Flash the sketch to your esp32.
4. Connect your esp32 to the smart meter. [See here how](https://github.com/matthijskooijman/arduino-dsmr#connecting-the-p1-port).<br>Take note that to connect to the esp32 the `DATA` signal has to be inverted and level shifted.
5. If you added a ssd1306 oled screen, the ip address will be visible on the screen.<br>If there is no oled you can check the ip address on the serial port in the Arduino IDE.
6. Browse to the ip address of your esp32 to see your current energy use.

## Needed libraries

- [https://github.com/me-no-dev/AsyncTCP](https://github.com/me-no-dev/AsyncTCP)
- [https://github.com/me-no-dev/ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
- [https://github.com/matthijskooijman/arduino-dsmr](https://github.com/matthijskooijman/arduino-dsmr)

Download and install these in the Arduino libraries folder.

The driver library for the ssd1306 oled can be installed with the Arduino library manager.

## DSMR v5 P1 port standard specifications

[DSMR v5.0.2 P1 Companion Standard.pdf](https://github.com/matthijskooijman/arduino-dsmr/blob/master/specs/DSMR%20v5.0.2%20P1%20Companion%20Standard.pdf)
