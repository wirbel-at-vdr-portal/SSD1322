# About this library

This library is for 3.2 inch SSD1322 256x64 16-color gray scale OLEDs.
It is based on the [Adafruit GFX Graphics Library](https://learn.adafruit.com/adafruit-gfx-graphics-library/overview), so read the documentation there.

The OLED need to be connected via 4-wire SPI (SCK,MOSI,CS,RST,D/C).
As the SSD1322 has different command interfaces, the display needs to be configured.

In my case, this is done by 4 resistors:
- R19, R21 soldered
- R18, R20 not soldered

The lib is loosely adopted from other Adafruit GFX based libraries and optimized to just contain as much code as necessary.

Feel free to use it.

Winfried
