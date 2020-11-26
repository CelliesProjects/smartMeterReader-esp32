# smartMeterReader-esp32

## About

`smartMeterReader-esp32` is a serial to websocket bridge between your DSMR 5 smartmeter and the rest of your network.<br>
Websocket clients connected to `ws://ip-address/raw` get the raw dsmr telegram pushed every second.<br>
Also included is a http webserver so you can see your current energy use and the daily usage figures on your smartphone or pc browser.<br>
No app required!

![screenshot](screenshot.png)

![screenshot](screenshot_android.png)

## How to use

1. Change your credentials in `wifisetup.h`.
1. (Optional) In `smartMeterReader-esp32.ino` uncomment `#define SH1106_OLED` if you compile for sh1106 instead of ssd1306 and set the i2c pins (and address) for your oled screen.
3. Save all files and flash the sketch to your esp32.
4. Connect your esp32 to the smart meter. [See here how](https://github.com/matthijskooijman/arduino-dsmr#connecting-the-p1-port).<br>Take note that to connect to the esp32 the `DATA` signal has to be inverted and level shifted.
5. If you added a ssd1306/sh1106 oled screen, the ip address will be visible on the screen.<br>If there is no oled you can check the ip address on the serial port in the Arduino IDE.
6. Browse to the ip address of your esp32 to see your current energy use.

If you have a garbled screen you most likely compiled for the wrong oled type.<br>Try to comment/uncomment `#define SH1106_OLED` to solve this.

## Needed libraries

- [https://github.com/me-no-dev/AsyncTCP](https://github.com/me-no-dev/AsyncTCP)
- [https://github.com/me-no-dev/ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
- [https://github.com/matthijskooijman/arduino-dsmr](https://github.com/matthijskooijman/arduino-dsmr)

Download and install these in the Arduino libraries folder.

The driver library for a ssd1306/sh1106 oled can be installed with the Arduino library manager. Use the ThingPulse driver.

## DSMR v5 P1 port standard specifications

[DSMR v5.0.2 P1 Companion Standard.pdf](https://github.com/matthijskooijman/arduino-dsmr/blob/master/specs/DSMR%20v5.0.2%20P1%20Companion%20Standard.pdf)

## Hardware

Very convenient is that an esp32 flashed with `smartMeterReader-esp32` can be run from the 5V supplied by the smartmeter if equiped with a proper WiFi antenna. (This reduces the required power)<br>The LilyGo TTGO T7 is a nice board with an external antenna connector and a decent 3.3v LDO. Without an external antenna the WiFi signal tends to be too poor to be of any use over longer distances and/or through several walls.

![T7 pic](t7.jpg)

Here you can see how to enable the external antenna. Move the zero ohm resistor from position 1-2 to position 3-4. Or remove the resistor and solder position 3-4 closed.

![external-config](https://user-images.githubusercontent.com/17033305/78676790-34fd1080-78e7-11ea-8bb0-aee88efe75a6.jpg)

See [this LilyGo issue](https://github.com/LilyGO/ESP32-MINI-32-V1.3/issues/4#issuecomment-610394847) about the external antenna.

To invert and level shift the signal you can use a bc547 transistor with some resistors. For example like this:

![invert-and-level-shift](https://willem.aandewiel.nl/wp-content/uploads/2019/04/DSMR_LevelShifter_Circuit-300x251.png)

See [willem.aandewiel.nl/dsmr-logger-v4-slimme-meter-uitlezer/](https://willem.aandewiel.nl/index.php/2019/04/09/dsmr-logger-v4-slimme-meter-uitlezer/)
