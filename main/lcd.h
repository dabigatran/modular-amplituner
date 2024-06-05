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

#define ROWS 2
#define COLS 16

#define SCREEN_CLEAR (char)0x01
#define CURSOR_RETURN (char)0x02
#define INPUT_SET (char)0x06          // Increment mode, no shift
#define DISPLAY_SWITCH_ON (char)0x0C  // Display on, cursor off, blink off
#define DISPLAY_SWITCH_OFF (char)0x08 // Display off, cursor off, blink off
#define SHIFT (char)0x10              // Cursor shift, left shift
#define FUNCTION_SET_0 (char)0x30     //(for 8bit init)b00110000 -> 8bit mode, 1 line, 5x8 characters
#define FUNCTION_SET_1 (char)0x20     //(for 4bit init)b00100000 -> 4bit mode, 1 line, 5x8 characters
#define FUNCTION_SET_2 (char)0x28     // b00101000 -> 4bit mode, 2 lines, 5x8 characters
#define BACKLIGHT_ON (char)0x08       // PCF8574 P3 (bit 4=1) - sent directly via i2c_write
#define BACKLIGHT_OFF (char)0x00      // PCF8574 P3 (bit 4=0) - sent directly via i2c_write

#define LINE1 (char)0x80
#define LINE2 (char)0xC0
#define EN_RS_CMD1 (char)0x0C
#define EN_RS_CMD2 (char)0x08
#define EN_RS_DATA1 (char)0x0D
#define EN_RS_DATA2 (char)0x09

#define LOGE_LCD (uint8_t)0x1
#define LOGI_LCD (uint8_t)0x0

esp_err_t LcdInit(void);
esp_err_t LcdCmd(char cmd);
esp_err_t LcdByte(bool is_cmd, char data);
esp_err_t LcdText(uint8_t row, uint8_t col, char *data);
esp_err_t LcdClear(void);
esp_err_t LcdClearLine(uint8_t line);
esp_err_t LcdCursor(uint8_t row, uint8_t col);
esp_err_t LcdBacklightOn();
esp_err_t LcdBacklightOff();
uint8_t LcdPosition(uint8_t row, uint8_t col);
void AdaptTo4bits(bool is_cmd, uint8_t char_index, char *source_data, uint8_t d_index, uint8_t *dest_data);
