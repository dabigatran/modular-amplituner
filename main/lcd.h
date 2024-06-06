/*
    Waveshare datasheet
    https://www.waveshare.com/datasheet/LCD_en_PDF/LCD1602.pdf
*/
#include <stdint.h>
#include <stdbool.h>
#include "esp_log.h"
#include "unistd.h"
#include "i2c.h"
#include "string.h"

#ifndef _lcd_parameters_H_
#define _lcd_parameters_H_

static const uint8_t LOGE_LCD = true;
static const uint8_t LOGI_LCD = false;
static char* const LCD_TAG = "LCD";

static const uint8_t ROWS = 2;
static const uint8_t COLS = 16;

static const char SCREEN_CLEAR = 0x01;
static const char CURSOR_RETURN = 0x02;
static const char INPUT_SET = 0x06; // Increment mode, no shift
static const char DISPLAY_SWITCH_ON = 0x0C; // Display on, cursor off, blink off
static const char DISPLAY_SWITCH_OFF = 0x08; // Display off, cursor off, blink off
static const char SHIFT = 0x10; // Cursor shift, left shift
static const char FUNCTION_SET_0 = 0x30; //(for 8bit init)b00110000 -> 8bit mode, 1 line, 5x8 characters
static const char FUNCTION_SET_1 = 0x20; //(for 4bit init)b00100000 -> 4bit mode, 1 line, 5x8 characters
static const char FUNCTION_SET_2 = 0x28; // b00101000 -> 4bit mode, 2 lines, 5x8 characters
static const char BACKLIGHT_ON = 0x08; // PCF8574 P3 (bit 4=1) - sent directly via i2c_write
static const char BACKLIGHT_OFF = 0x00; // PCF8574 P3 (bit 4=0) - sent directly via i2c_write

static const char LINE1 = 0x80;
static const char LINE2 = 0xC0;
static const char EN_RS_CMD1 = 0x0C;
static const char EN_RS_CMD2 = 0x08;
static const char EN_RS_DATA1 = 0x0D;
static const char EN_RS_DATA2 = 0x09;

esp_err_t LcdInit(void);
esp_err_t LcdCmd(char cmd);
esp_err_t LcdByte(bool isCmd, char data);
esp_err_t LcdText(uint8_t row, uint8_t col, char *data);
esp_err_t LcdClear(void);
esp_err_t LcdClearLine(uint8_t line);
esp_err_t LcdCursor(uint8_t row, uint8_t col);
esp_err_t LcdBacklightOn();
esp_err_t LcdBacklightOff();
uint8_t LcdPosition(uint8_t row, uint8_t col);
void AdaptTo4bits(bool isCmd, uint8_t charIndex, char *sourceData, uint8_t dIndex, uint8_t *destData);
#endif