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
#include "SSD1322.h"


/*******************************************************************************
 * using Teensy 3.2
 ******************************************************************************/
                 // 3.3V         <-> VDD
                 // GND          <-> GND, D3..D7, RD, WR
                 // PIN 11 (MOSI)<-> SDIN
                 // Pin 13 (SCK) <-> SCLK
#define CS   10  // Pin 10 (CS)  <-> /CS
#define DC   14  // Pin 9        <-> D/C
#define RST  15  // Pin 15       <-> /RST




// hardware SPI on teensy 3.x -> use default SPI pins
SSD1322 OLED(DC, RST, CS);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#define BITMAP_HEIGHT 16 
#define BITMAP_WIDTH  16 
static const unsigned char PROGMEM snowflake[] = {
  B00000000, B11000000, B00000001, B11000000, B00000001, B11000000,
  B00000011, B11100000, B11110011, B11100000, B11111110, B11111000,
  B01111110, B11111111, B00110011, B10011111, B00011111, B11111100,
  B00001101, B01110000, B00011011, B10100000, B00111111, B11100000,
  B00111111, B11110000, B01111100, B11110000, B01110000, B01110000,
  B00000000, B00110000
};


void setup() {
  OLED.begin(true);
}

void loop() { 
  // draw a single pixel
  OLED.fillScreen(BLACK);
  OLED.drawPixel(OLED.width()/2, OLED.height()/2, WHITE);
  OLED.update();
  delay(1500);

  // draw random pixels
  for(int i=0; i<64; i++) {
     OLED.drawPixel(random(OLED.width()), random(OLED.height()), WHITE);
     OLED.update();
     delay(20);
     }
  delay(1000);

  // draw many lines
  OLED.fillScreen(BLACK);
  testdrawline();
  OLED.update();
  delay(1500);

  // draw rectangles
  OLED.fillScreen(BLACK);
  testdrawrect();
  OLED.update();
  delay(1500);

  // draw multiple rectangles
  OLED.fillScreen(BLACK);
  testfillrect();
  OLED.update();
  delay(1500);

  // draw multiple circles
  OLED.fillScreen(BLACK);
  testdrawcircle();
  OLED.update();
  delay(1500);

  // draw a white circle, 10 pixel radius
  OLED.fillScreen(BLACK);
  OLED.fillCircle(OLED.width()/2, OLED.height()/2, 10, WHITE);
  OLED.update();
  delay(1500);

  OLED.fillScreen(BLACK);
  testdrawroundrect();
  delay(1500);

  OLED.fillScreen(BLACK);
  testfillroundrect();
  delay(1500);

  OLED.fillScreen(BLACK);
  testdrawtriangle();
  delay(1500);

  OLED.fillScreen(BLACK);
  testfilltriangle();
  delay(1500);

  // draw the first ~12 characters in the font
  OLED.fillScreen(BLACK);
  testdrawchar();
  OLED.update();
  delay(1500);

  // text display tests
  OLED.fillScreen(BLACK);
  OLED.setTextSize(1);
  OLED.setTextColor(WHITE);
  OLED.setCursor(0,0);
  OLED.println("Hello, world!");
  OLED.setTextColor(BLACK, WHITE); // 'inverted' text
  OLED.println(3.141592);
  OLED.setTextSize(2);
  OLED.setTextColor(WHITE);
  OLED.print("0x"); OLED.println(0xDEADBEEF, HEX);
  OLED.update();
  delay(2000);

  // Brightness tests
  OLED.fillScreen(BLACK);
  for(int cnt=0;cnt<2;cnt++) {
     OLED.setTextSize(2);
     OLED.setTextColor(BLACK, WHITE); // 'inverted' text
     for(int i=255; i>=0;) {
        OLED.fillScreen(BLACK);
        OLED.setCursor(2,2);
        OLED.print("setBrightness ");
        OLED.println(i);
        delay(300);
        OLED.update();
        OLED.setBrightness(i);
        if (i > 30)
           i -= 20;
        else
           i--;
        }
     OLED.setBrightness(255);
     }

  // miniature bitmap display
  OLED.fillScreen(BLACK);
  OLED.drawBitmap(30, 16, snowflake, BITMAP_WIDTH, BITMAP_HEIGHT, WHITE);
  OLED.update();
  delay(500);

  // invert the display
  OLED.invertDisplay(true);
  delay(1000);
  OLED.invertDisplay(false);
  delay(1000);

  // draw a bitmap icon and 'animate' movement
  testdrawbitmap(snowflake, BITMAP_WIDTH, BITMAP_HEIGHT);
}


void testdrawbitmap(const uint8_t* bitmap, uint8_t w, uint8_t h) {
  uint8_t FlakePositions[NUMFLAKES][3];
 
  // initialize
  for(uint8_t i=0; i< NUMFLAKES; i++) {
     FlakePositions[i][XPOS  ] = random(OLED.width());
     FlakePositions[i][YPOS  ] = 0;
     FlakePositions[i][DELTAY] = random(5) + 1;
     }

  OLED.fillScreen(BLACK);

  bool invert = false;
  int invertCount = 0;
  
  while(invertCount < 1000) {
     for(uint8_t i=0; i<NUMFLAKES; i++) {
        OLED.drawBitmap(FlakePositions[i][XPOS],
                        FlakePositions[i][YPOS],
                        snowflake, w, h, WHITE);
        }
     OLED.update();
    
     // then erase it + move it
     for(uint8_t i=0; i<NUMFLAKES; i++) {
        // clear flake, by drawing it in black.
        OLED.drawBitmap(FlakePositions[i][XPOS],
                        FlakePositions[i][YPOS],
                        snowflake, w, h, BLACK);
        // update flake position
        FlakePositions[i][YPOS] += FlakePositions[i][DELTAY];
        // if its gone, reinit
        if (FlakePositions[i][YPOS] > OLED.height()) {
           FlakePositions[i][XPOS  ] = random(OLED.width());
           FlakePositions[i][YPOS  ] = 0;
           FlakePositions[i][DELTAY] = random(5) + 1;
           }
        }
    if (++invertCount % 100 == 0) {
       invert = not invert;
       OLED.invertDisplay(invert);
       }
    }
}

void testdrawchar(void) {
  OLED.setTextSize(1);
  OLED.setTextColor(WHITE);
  OLED.setCursor(0,0);

  for(uint8_t i=0; i<168; i++) {
     if (i == '\n') continue;
     OLED.write(i);
     if ((i > 0) && (i % 21 == 0))
        OLED.println();
     }
  OLED.update();
}

void testdrawcircle(void) {
  for(int16_t i=0; i<OLED.height(); i+=2) {
     OLED.drawCircle(OLED.width()/2, OLED.height()/2, i, WHITE);
     OLED.update();
     }
}

void testfillrect(void) {
  uint8_t color = 1;
  for(int16_t i=0; i<OLED.height()/2; i+=3) {
     // alternate colors
     OLED.fillRect(i, i, OLED.width()-i*2, OLED.height()-i*2, color%2);
     OLED.update();
     color++;
     }
}

void testdrawtriangle(void) {
  for(int16_t i=0; i<min(OLED.width(),OLED.height())/2; i+=5) {
     OLED.drawTriangle(OLED.width()/2  , OLED.height()/2-i,
                       OLED.width()/2-i, OLED.height()/2+i,
                       OLED.width()/2+i, OLED.height()/2+i,
                       WHITE);
     OLED.update();
     }
}

void testfilltriangle(void) {
  uint8_t color = WHITE;
  for(int16_t i=min(OLED.width(),OLED.height())/2; i>0; i-=5) {
     OLED.fillTriangle(OLED.width()/2  , OLED.height()/2-i,
                       OLED.width()/2-i, OLED.height()/2+i,
                       OLED.width()/2+i, OLED.height()/2+i,
                       WHITE);
     if (color == WHITE)
        color = BLACK;
     else
        color = WHITE;
     OLED.update();
     }
}

void testdrawroundrect(void) {
  for(int16_t i=0; i<OLED.height()/2-2; i+=2) {
     OLED.drawRoundRect(i, i, OLED.width()-2*i, OLED.height()-2*i, OLED.height()/4, WHITE);
     OLED.update();
     }
}

void testfillroundrect(void) {
  uint8_t color = WHITE;
  for(int16_t i=0; i<OLED.height()/2-2; i+=2) {
     OLED.fillRoundRect(i, i, OLED.width()-2*i, OLED.height()-2*i, OLED.height()/4, color);
     if (color == WHITE)
        color = BLACK;
     else
        color = WHITE;
     OLED.update();
     }
}
   
void testdrawrect(void) {
  for(int16_t i=0; i<OLED.height()/2; i+=2) {
     OLED.drawRect(i, i, OLED.width()-2*i, OLED.height()-2*i, WHITE);
     OLED.update();
     }
}

void testdrawline() {  
  for(int16_t i=0; i<OLED.width(); i+=4) {
     OLED.drawLine(0, 0, i, OLED.height()-1, WHITE);
     OLED.update();
     }
  for(int16_t i=0; i<OLED.height(); i+=4) {
     OLED.drawLine(0, 0, OLED.width()-1, i, WHITE);
     OLED.update();
     }
  delay(250);
  
  OLED.fillScreen(BLACK);
  for(int16_t i=0; i<OLED.width(); i+=4) {
     OLED.drawLine(0, OLED.height()-1, i, 0, WHITE);
     OLED.update();
     }
  for(int16_t i=OLED.height()-1; i>=0; i-=4) {
     OLED.drawLine(0, OLED.height()-1, OLED.width()-1, i, WHITE);
     OLED.update();
     }
  delay(250);
  
  OLED.fillScreen(BLACK);
  for(int16_t i=OLED.width()-1; i>=0; i-=4) {
     OLED.drawLine(OLED.width()-1, OLED.height()-1, i, 0, WHITE);
     OLED.update();
     }
  for(int16_t i=OLED.height()-1; i>=0; i-=4) {
     OLED.drawLine(OLED.width()-1, OLED.height()-1, 0, i, WHITE);
     OLED.update();
     }
  delay(250);

  OLED.fillScreen(BLACK);
  for(int16_t i=0; i<OLED.height(); i+=4) {
     OLED.drawLine(OLED.width()-1, 0, 0, i, WHITE);
     OLED.update();
     }
  for(int16_t i=0; i<OLED.width(); i+=4) {
     OLED.drawLine(OLED.width()-1, 0, i, OLED.height()-1, WHITE); 
     OLED.update();
     }
  delay(250);
}
