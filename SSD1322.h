/*******************************************************************************
 * This library is for 3.2 inch SSD1322 256x64 16-color gray scale OLEDs.
 * It is based on Adafruit_GFX, so read the documentation there.
 *
 * The display needs to be configured in 4-wire SPI.
 *  R19, R21 soldered
 *  R18, R20 not soldered
 *
 * https://github.com/wirbel-at-vdr-portal/SSD1322
 ******************************************************************************/
#pragma once

#include "Arduino.h"
#include <Adafruit_GFX.h> // class Adafruit_GFX

#define BLACK        0x00
#define WHITE        0x0F

#define LCD_WIDTH    256
#define LCD_HEIGHT   64


/* You have the choice here.
 * - 4 bit per pixel takes more memory for the display buffer, but gives
 *   16 gray scale values.
 * - 1 bit per pixel is more light weight and the gray scale values
 *   doesnt make too much sense usually.
 */
//#define BITS_PER_PIXEL  4  /* 4 bits per pixel: 16 color */
#define BITS_PER_PIXEL  1    /* 1 bits per pixel:  2 color */


class SSD1322 : public Adafruit_GFX {
private:
  uint8_t buffer[(LCD_HEIGHT * LCD_WIDTH * BITS_PER_PIXEL) / 8];
  int8_t mosi, sck, dc, rst, cs;

  void Command(uint8_t cmd);
  void Data(uint8_t data);
  void DataBytes(uint8_t* buf, uint32_t size);

  inline void drawFastHLineInternal(int16_t x, int16_t y, int16_t w, uint16_t color) __attribute__((always_inline));
  inline void drawFastVLineInternal(int16_t x, int16_t y, int16_t h, uint16_t color) __attribute__((always_inline));

public:
  /*****************************************************************************
   * MOSI, SCK: on ESP boards, you can choose the SPI pins.
   * DC, RST, CS: Data/Command, Reset, Chip Select.
   * Reset may be connected to a dedicated reset pin. If so, put -1 here.
   ****************************************************************************/
  SSD1322(int8_t MOSI, int8_t SCK, int8_t DC, int8_t RST, int8_t CS); // hardware SPI on ESP
  SSD1322(int8_t DC, int8_t RST, int8_t CS);                          // hardware SPI using default SPI

  // call begin once in setup()
  void begin(bool reset = true);


  /*****************************************************************************
   * The only function, which needs to be implemented on any Adafruit_GFX. 
   ****************************************************************************/
  void drawPixel(int16_t x, int16_t y, uint16_t color);

  /*****************************************************************************
   * TRANSACTION API / CORE DRAW API
   *   -> Most likely, we don't gain any speed if implementing those,
   *      as we update the display only on request.
   * void startWrite(void);
   * void writePixel(int16_t x, int16_t y, uint16_t color);
   * void writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
   * void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
   * void writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
   * void writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
   * void endWrite(void);
   ****************************************************************************/

  /*****************************************************************************
   * CONTROL API
   *   -> No need to implementing setRotation(), everything our base class does
   *      is fine. Rotatation is 0..3 <-> 0 deg, +90deg .. +270 deg
   * void setRotation(uint8_t r);
   */
  void invertDisplay(bool i);

  /*****************************************************************************
   * BASIC DRAW API
   * These MAY be overridden by the subclass to provide device-specific
   * optimized code.  Otherwise 'generic' versions are used.
   * It's good to implement those, even if using transaction API
   ****************************************************************************/
  void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
  void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
//void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  void fillScreen(uint16_t color);

  /* Optional and probably not necessary to change */
  //void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
  //void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

  // send whole display buffer to OLED
  void update(void);

  // Set display brightness in 256 steps.
  // Lower brightness takes lower current, but not that much effect.
  void setBrightness(uint8_t brightness);
};
