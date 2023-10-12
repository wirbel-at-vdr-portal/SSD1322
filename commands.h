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

#define ENABLE_GRAY_SCALE_TABLE       0x00
#define SET_COLUMN_ADDRESS            0x15
#define WRITE_RAM                     0x5C
#define READ_RAM                      0x5D
#define SET_ROW_ADDRESS               0x75
#define SET_REMAP                     0xA0
#define SET_DISPLAY_START_LINE        0xA1
#define SET_DISPLAY_OFFSET            0xA2
#define SET_ENTIRE_DISPLAY_OFF        0xA4
#define SET_ENTIRE_DISPLAY_ON         0xA5
#define SET_DISPLAY_MODE_NORMAL       0xA6
#define SET_DISPLAY_MODE_INVERSE      0xA7
#define ENABLE_PARTIAL_DISPLAY        0xA8
#define EXIT_PARTIAL_DISPLAY          0xA9
#define FUNCTION_SELECTION            0xAB
#define SET_DISPLAY_OFF               0xAE
#define SET_DISPLAY_ON                0xAF
#define SET_PHASE_LENGTH              0xB1
#define SET_CLOCK_DIVIDER             0xB3
#define SET_DISPLAY_ENHANCE_A         0xB4
#define SET_GPIO                      0xB5
#define SET_SECOND_PRECHARGE_PERIOD   0xB6
#define SET_GRAY_SCALE_TABLE          0xB8
#define SELECT_DEFAULT_GRAY_SCALE_TBL 0xB9
#define SET_PRECHARGE_VOLTAGE         0xBB
#define SET_VCOMH                     0xBE
#define SET_CONTRAST_CURRENT          0xC1
#define MASTER_CURRENT_CONTROL        0xC7
#define SET_MULTIPLEX_RATIO           0xCA
#define SET_DISPLAY_ENHANCE_B         0xD1
#define SET_COMMAND_LOCK              0xFD
