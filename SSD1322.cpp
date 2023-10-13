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
#include <algorithm> // std::swap
#include "SSD1322.h"
#include <SPI.h>
#include "commands.h"
#if (!defined(ESP8266) && !defined(ESP32))
  #include <avr/pgmspace.h>
#endif


/*******************************************************************************
 * constructor
 ******************************************************************************/
SSD1322::SSD1322(int8_t MOSI, int8_t SCK, int8_t DC, int8_t RST, int8_t CS) :
   Adafruit_GFX(LCD_WIDTH, LCD_HEIGHT), mosi(MOSI), sck(SCK), dc(DC), rst(RST),
   cs(CS)
{}

SSD1322::SSD1322(int8_t DC, int8_t RST, int8_t CS) :
   Adafruit_GFX(LCD_WIDTH, LCD_HEIGHT), mosi(-1), sck(-1), dc(DC), rst(RST),
   cs(CS)
{}


/*******************************************************************************
 * SPI low level functions.
 ******************************************************************************/
void SSD1322::Command(uint8_t cmd) {
  digitalWriteFast(cs, HIGH);
  digitalWriteFast(dc, LOW);
  digitalWriteFast(cs, LOW);
  SPI.transfer(cmd);
  digitalWriteFast(cs, HIGH);
}

void SSD1322::Data(uint8_t data) {
  digitalWriteFast(cs, HIGH);
  digitalWriteFast(dc, HIGH);
  digitalWriteFast(cs, LOW);
  SPI.transfer(data);
  digitalWriteFast(cs, HIGH);
}

void SSD1322::DataBytes(uint8_t* buf, uint32_t size) {
  digitalWriteFast(cs, HIGH);
  digitalWriteFast(dc, HIGH);
  digitalWriteFast(cs, LOW);
  #if (defined(ESP8266) || defined(ESP32))
     SPI.writeBytes(buf, size);
  #else
     SPI.transfer(buf, size);
  #endif
  digitalWriteFast(cs, HIGH);
}


/*******************************************************************************
 * Initalize the controller.
 ******************************************************************************/
void SSD1322::begin(bool reset) {
  pinMode(dc, OUTPUT);
  pinMode(cs, OUTPUT);

  #if defined(ESP32) || defined(ESP8266)
     // this doesn't make sense on other µCs than ESP
     SPI.begin(sck, MISO, mosi);
     SPI.setClockDivider(SPI_CLOCK_DIV2);
  #else
     SPI.begin();
  #endif

  if (reset and (rst > 0)) {
     pinMode(rst, OUTPUT);
     digitalWriteFast(rst, HIGH);
     delay(100);
     digitalWriteFast(rst, LOW);
     delay(400);
     digitalWriteFast(rst, HIGH);
     }


  Command(SET_COMMAND_LOCK);                      // 10.1.25 Set Command Lock
  Data(0x12);                                     // Unlock OLED driver IC

  Command(SET_DISPLAY_OFF);                       // 10.1.13 Set Display ON/OFF

  {
  Command(SET_CLOCK_DIVIDER);                     // 10.1.15 Set Clock Divider / Oscillator Frequency
  uint8_t data = 0;
  data |= (1 << 0);                               // Front Clock Divide Ratio [0..15 -> 1..16] -> divide by 2
  data |= (9 << 4);                               // Oscillator Frequency     [0..15 -> 1..16] -> 10 (default value)
  Data(data);
  }

  Command(SET_MULTIPLEX_RATIO);                   // 10.1.24 Set Multiplex Ratio
  Data(0x3F);                                     //    duty = 1/64

  Command(SET_DISPLAY_OFFSET);                    // 10.1.8 Set Display Offset
  Data(0x00);                                     //    no offset

  Command(SET_DISPLAY_START_LINE);                // 10.1.7 Set Display Start Line
  Data(0x00);                                     //    display ram begin without offset

  {
  Command(SET_REMAP);                             // 10.1.6 Set Re-map & Dual COM Line Mode
  uint8_t data = 0;
  data |= (0 << 0);                               // Horizontal address increment
  data |= (0 << 1);                               // Column Address Remap OFF, segments from left to right
  data |= (1 << 2);                               // Nibble Remap = ON. Sets bit order in ram
  data |= (1 << 4);                               // COM Scan remap: Scan from bottom to up
  data |= (0 << 5);                               // Disable COM split odd even
  Data(data);

  data = 0x01;                                    // Table 9-1 : Command table, p.32
  data |= (1 << 4);                               // Set Dual COM mode: Enabled, as shown in Figure 10-6
  Data(data);
  }

  Command(SET_GPIO);                              // 10.1.17 Set GPIO
  Data(0x00);                                     // Disable GPIO Pins

  Command(FUNCTION_SELECTION);                    // 10.1.12 Set Function selection
  Data(0x01);                                     // Internal VDD regulator is selected

  {
  Command(SET_DISPLAY_ENHANCE_A);                 // 10.1.16 Display Enhancement A
  uint8_t data = 0xA0;                            // Table 9-1 : Command table, p.34
  data |= (0 << 0);                               // Enable external VSL
  Data(data);

  data = 0x05;                                    // Table 9-1 : Command table, p.34
  data |= (0x1F << 3);                            // 0x1F = Enhanced low GS display quality; 0x16 = Normal [reset]
  Data(data);
  }

  Command(SET_CONTRAST_CURRENT);                  // 10.1.23 Set Contrast Current
  Data(0xFF);                                     // 256 steps. 0xFF - maximum brightness and contrast

  Command(MASTER_CURRENT_CONTROL);                // 10.1.24 Master Current Control
  Data(0x0F);                                     // 16 steps. 0x0F - maximum brightness and contrast

  Command(SELECT_DEFAULT_GRAY_SCALE_TBL);         // 10.1.20 Select Default Linear Gray Scale Table

  {
  Command(SET_PHASE_LENGTH);                      // 10.1.14 Set Phase Length
  uint8_t data = 0;
  data |= (0x2 << 0);                             // Phase 1 period (reset phase length)            =  5 DCLKs
  data |= (0xE << 4);                             // Phase 2 period (first pre-charge phase length) = 14 DCLKs
  Data(data);
  }

  {
  Command(SET_DISPLAY_ENHANCE_B);                 // 10.1.26 Display Enhancement B
  uint8_t data = 0x82;
  data |= (0x0 << 4);                             // User is recommended to set 0
  Data(data);
  Data(0x20);                                     // Table 9-1 : Command table, p.36
  }

  {
  Command(SET_PRECHARGE_VOLTAGE);                 // 10.1.21 Set Pre-charge voltage
  uint8_t data = 0x1F;
  data = min(data,0x1F);                          // 0..0x1F -> (0.2..0.6) * Vcc
  Data(data);
  }

  Command(SET_SECOND_PRECHARGE_PERIOD);           // 10.1.18 Set Second Pre-charge period
  Data(0x08);                                     // default

  Command(SET_VCOMH);                             // 10.1.22 Set VCOMH Voltage
  Data(0x07);                                     // 0..7 -> (0.72..0.86) * Vcc

  Command(SET_DISPLAY_MODE_NORMAL);               // 10.1.9 Set Display Mode (A4h ~ A7h)

  Command(EXIT_PARTIAL_DISPLAY);                  // 10.1.11 Exit Partial Display

  fillScreen(BLACK);                              // Clear image ram

  Command(SET_DISPLAY_ON);                        // 10.1.13 Set Display ON/OFF
}


/*******************************************************************************
 * Draw to the screen/framebuffer/etc. Must be overridden in subclass.
 * Set a single pixel.
 *   x      X coordinate in pixels
 *   y      Y coordinate in pixels
 *   color  16-bit pixel color.
 ******************************************************************************/
void SSD1322::drawPixel(int16_t x, int16_t y, uint16_t color) {
  switch(getRotation()) {
     case 0: //  0 degree rotation
        break;
     case 1: // 90 degree rotation
        std::swap(x, y);
        x = WIDTH - x - 1;
        break;
     case 2:// 180 degree rotation
        x = WIDTH - x - 1;
        y = HEIGHT - y - 1;
        break;
     case 3:// 270 degree rotation
        std::swap(x, y);
        y = HEIGHT - y - 1;
        break;
     }

  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height()))
     return;

  #if (BITS_PER_PIXEL == 1)
     register uint8_t* p = &buffer[(x >> 3) + (y * (LCD_WIDTH / 8))];
     if (color > 7)
        *p |=  (0x80 >> (x%8));
     else
        *p &= ~(0x80 >> (x%8));
  #elif (BITS_PER_PIXEL == 4)
     color &= 0x0F;
     register uint8_t mask = ((x % 2) ? color : color << 4);
     register uint8_t* p = &buffer[(x >> 1) + (y * (LCD_WIDTH / 2))];
     register uint8_t b1 = *p;
     b1 &= (x % 2) ? 0xF0 : 0x0F;
     *p = b1 | mask;
  #endif
}

/*******************************************************************************
 * TRANSACTION API / CORE DRAW API
 * Most likely, we don't gain speed if implementing those.
 ******************************************************************************/
/* 
 * void SSD1322::startWrite(void);
 * void SSD1322::writePixel(int16_t x, int16_t y, uint16_t color);
 * void SSD1322::writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
 * void SSD1322::writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
 * void SSD1322::writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
 * void SSD1322::writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
 * void SSD1322::endWrite(void);
 */



/*******************************************************************************
 * CONTROL API
 * These MAY be overridden by the subclass to provide device-specific
 * optimized code.  Otherwise 'generic' versions are used.
 ******************************************************************************/
/*
void SSD1322::setRotation(uint8_t r);
*/

void SSD1322::invertDisplay(bool i) {
  if (i)
     Command(SET_DISPLAY_MODE_INVERSE);
  else
     Command(SET_DISPLAY_MODE_NORMAL);
}


/*******************************************************************************
 * private internals.
 ******************************************************************************/
void SSD1322::drawFastHLineInternal(int16_t x, int16_t y, int16_t w, uint16_t color) {
  if (y < 0 || y >= HEIGHT)
     return;

  // make sure we don't try to draw below 0
  if (x < 0) {
     w += x;
     x = 0;
     }

  // make sure we don't go off the edge of the display
  if ((x + w) > WIDTH)
     w = (WIDTH - x);

  // if our width is now negative, punt
  if (w <= 0)
     return;

  // set up the pointer for  movement through the buffer
  #if (BITS_PER_PIXEL == 4)
     // adjust the buffer pointer for the current row
     register uint8_t* pBuf = buffer;
     pBuf += (x >> 1) + (y * (LCD_WIDTH / 2));

     color &= 0x0F;
     register uint8_t oddmask = color;
     register uint8_t evenmask = (color << 4);
     register uint8_t fullmask = (color << 4) + color;
     uint8_t byteLen = w / 2;

     if (((x % 2) == 0) && ((w % 2) == 0)) {
        // Start at even and length is even
        while (byteLen--)
           *pBuf++ = fullmask;
        return;
        }

     if (((x % 2) == 1) && ((w % 2) == 1)) {
        // Start at odd and length is odd
        register uint8_t b1 = *pBuf;
        b1 &= (x % 2) ? 0xF0 : 0x0F;
        *pBuf++ = b1 | oddmask;
        while (byteLen--)
           *pBuf++ = fullmask;
        return;
        }

     if (((x % 2) == 0) && ((w % 2) == 1)) {
        // Start at even and length is odd
        while (byteLen--)
           *pBuf++ = fullmask;
        register uint8_t b1 = *pBuf;
        b1 &= 0x0F; // cleardown nibble to be replaced
        *pBuf++ = b1 | evenmask;
        return;
        }

     if (((x % 2) == 1) && ((w % 2) == 0)) { // Start at odd and length is even
        register uint8_t b1 = *pBuf;
        b1 &= (x % 2) ? 0xF0 : 0x0F;
        *pBuf++ = b1 | oddmask;
        while (byteLen--)
           *pBuf++ = fullmask;
        b1 = *pBuf;
        b1 &= 0x0F;
        *pBuf++ = b1 | evenmask;
        return;
        }
  #elif (BITS_PER_PIXEL == 1)
     register uint8_t* pBuf = &buffer[(x >> 3) + (y * (LCD_WIDTH / 8))];
     register uint8_t mod = (x % 8);
     if (mod) {
        mod = 8-mod;
        static uint8_t premask[8] = {0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F };
        register uint8_t mask = premask[mod];
        if (w < mod) {
           mask &= (0XFF << (mod - w));
           }
        if (color > 7)
           *pBuf |=  mask;
        else
           *pBuf &= ~mask;
        if (w < mod)
           return;
        w -= mod;
        pBuf++;
        }

     // write solid bytes while we can - effectively doing 8 rows at a time
     if (w >= 8) {
        register uint8_t val = (color > 7) ? 255 : 0;
        do {
           *pBuf++ = val;
           w -= 8;
           }
        while(w >= 8);
        }
   if (w) {
      mod = w % 8;
      static uint8_t postmask[8] = {0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE };
      register uint8_t mask = postmask[mod];
      if (color > 7)
         *pBuf |=  mask;
      else
         *pBuf &= ~mask;
      }
  #endif
}

void SSD1322::drawFastVLineInternal(int16_t x, int16_t __y, int16_t __h, uint16_t color) {
  if (x < 0 || x >= WIDTH)
     return;

  if (__y < 0) {
     __h += __y;
     __y = 0;
     }

  if ((__y + __h) > HEIGHT)
     __h = (HEIGHT - __y);

  if (__h <= 0)
     return;

  // this display doesn't need ints for coordinates, use local byte registers for faster juggling
  register uint8_t y = __y;
  register uint8_t h = __h;

  #if (BITS_PER_PIXEL == 4)
     // set up the pointer for fast movement through the buffer
     register uint8_t* pBuf = buffer;
     pBuf += (x >> 1) + (y  * (LCD_WIDTH / 2));

     color &= 0x0F;
     register uint8_t mask = ((x % 2) ? color : color << 4);
     while(h--) {
        register uint8_t b1 = *pBuf;
        b1 &= (x % 2) ? 0xF0 : 0x0F; // cleardown nibble to be replaced
        *pBuf = b1 | mask;
        pBuf += LCD_WIDTH / 2;
        }
  #elif (BITS_PER_PIXEL == 1)
     register uint8_t* pBuf = &buffer[(x >> 3) + (y * (LCD_WIDTH / 8))];
     register uint8_t mod = (x % 8);

     static uint8_t postmask[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
     register uint8_t mask = postmask[mod];

     while(h--) {
        if (color > 7)
           *pBuf |=  mask;
        else
           *pBuf &= ~mask;
        pBuf += LCD_WIDTH / 8;
        }
  #endif
}

/*******************************************************************************
 * BASIC DRAW API
 * These MAY be overridden by the subclass to provide device-specific
 * optimized code.  Otherwise 'generic' versions are used.
 * It's good to implement those, even if using transaction API
 ******************************************************************************/
void SSD1322::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
  bool bSwap = false;
  switch(rotation) {
     case 0:
        break;
     case 1:
        /* 90 degree rotation
         * swap x & y for rotation,
         * then invert x and adjust x for h (now to become w)
         */
        bSwap = true;
        std::swap(x, y);
        x = WIDTH - x - 1;
        x -= (h - 1);
        break;
     case 2:
        /* 180 degree rotation
         * invert x and y,
         * then shift y around for height.
         */
        x = WIDTH - x - 1;
        y = HEIGHT - y - 1;
        y -= (h - 1);
        break;
     case 3:
        /* 270 degree rotation
         * swap x & y for rotation,
         * then invert y
         */
        bSwap = true;
        std::swap(x, y);
        y = HEIGHT - y - 1;
        break;
     }

  if (bSwap)
     drawFastHLineInternal(x, y, h, color);
  else
     drawFastVLineInternal(x, y, h, color);
}

void SSD1322::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
  boolean bSwap = false;
  switch (rotation) {
     case 0:
        break;
     case 1:
        /* 90 degree rotation,
         * swap x & y for rotation,
         * then invert x
         */
        bSwap = true;
        std::swap(x, y);
        x = WIDTH - x - 1;
        break;
     case 2:
        /* 180 degree rotation,
         * invert x and y
         * then shift y around for height.
         */
        x = WIDTH - x - 1;
        y = HEIGHT - y - 1;
        x -= (w - 1);
        break;
     case 3:
       /* 270 degree rotation,
        * swap x & y for rotation,
        * then invert y  and adjust y for w (not to become h)
        */
       bSwap = true;
       std::swap(x, y);
       y = HEIGHT - y - 1;
       y -= (w - 1);
       break;
     }

  if (bSwap)
     drawFastVLineInternal(x, y, w, color);
  else
     drawFastHLineInternal(x, y, w, color);
}

//void SSD1322::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

void SSD1322::fillScreen(uint16_t color) {
  #if (BITS_PER_PIXEL == 1)
     if (color > 7)
        memset(buffer, 0xFF, sizeof(buffer));
     else
        memset(buffer, 0x00, sizeof(buffer));
  #elif (BITS_PER_PIXEL == 4)
     uint8_t color16 = (color & 0x0F);
     memset(buffer, (color16 << 4) | color16, sizeof(buffer));
  #endif
}

// Optional and probably not necessary to change
//void SSD1322::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
//void SSD1322::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);



/*******************************************************************************
 * display specific stuff, not related to Adafruit GFX.
 ******************************************************************************/
void SSD1322::update(void) {

  #define MIN_SEG   0x1C
  #define MAX_SEG   0x5B

  Command(SET_COLUMN_ADDRESS);
  Data(MIN_SEG);
  Data(MAX_SEG);

  Command(SET_ROW_ADDRESS);
  Data(0);
  Data(63);

  Command(WRITE_RAM);

  register uint32_t bufSize = sizeof(buffer); // bytes
  register uint8_t* p = buffer;

  #if (BITS_PER_PIXEL == 1)
     uint16_t srcIndex = 0;
     while(srcIndex < bufSize) {
        uint8_t destIndex = 0;
        uint8_t destArray[64] = {0};

        while(destIndex < 64) {
           uint8_t mask = 0x80;
           while(mask > 0) {
              destArray[destIndex] |= (p[srcIndex] & mask) ? 0xF0 : 0x00;
              mask >>= 1;
              destArray[destIndex] |= (p[srcIndex] & mask) ? 0x0F : 0x00;
              destIndex++;
              mask >>= 1;
              }
           srcIndex++;
           }
        DataBytes(destArray, 64);
        }
  #elif (BITS_PER_PIXEL == 4)
     DataBytes(p, bufSize);
  #endif
}


// Set display brightness in 256 steps.
void SSD1322::setBrightness(uint8_t brightness) {
  Command(SET_CONTRAST_CURRENT);
  Data(brightness);
}

