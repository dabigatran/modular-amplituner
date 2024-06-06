#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <esp_err.h>
#include "esp_log.h"
#include "i2c.h"
#include "mcp23017.h"
#include "lcd.h"
#include "amp_parameters.h"
#include "njw1194.h"
#define MAX_SRC_ALIAS_LENGTH 16
#define MAX_OPT_ALIAS_LENGTH 12
#define MAX_SOURCES 9

#ifndef _source_parameters_H_
#define _source_parameters_H_
static const uint8_t LOGE_SRC = true;
static const uint8_t LOGI_SRC = false;
// Number of ports
static const uint8_t HDMI_NO = 4;
static const uint8_t I2S_NO = 4;
static const uint8_t SOURCE0 = 0;
static const uint8_t SOURCE1 = 1;
static const uint8_t SOURCE2 = 2;
static const uint8_t SOURCE3 = 3;
static const uint8_t SOURCE4 = 4;
static const uint8_t SOURCE5 = 5;
static const uint8_t SOURCE6 = 6;
static const uint8_t SOURCE7 = 7;
static const uint8_t SOURCE8 = 8;
static const uint8_t NOT_FOUND = 0;
static const uint8_t FOUND = 1;
static const uint8_t SEARCHED = 0;
static const uint8_t SET = 1;

/* MCP1, GPIOA values (8 bits), ALL INPUT */
static const uint8_t HDMI1 = 0x11;
static const uint8_t HDMI2 = 0x12;
static const uint8_t HDMI3 = 0x14;
static const uint8_t HDMI4 = 0x18;
static const uint8_t I2S1 = 0x10;
static const uint8_t I2S2 = 0x20;
static const uint8_t I2S3 = 0x40;
static const uint8_t I2S4 = 0x80;

/* MCP1, GPIOB */
static const uint8_t HDMI_SWITCH = PB0; // OUTPUT
static const uint8_t I2S_SWITCH = PB1; // OUTPUT

void SourcesInit();
void MapSources();
void SetAvailibility();
void SetSource(int8_t (*tunerState)[VAR_NO]);
int8_t ChangeSource(int8_t source);
int8_t IsHdmiAvailable();
int8_t ToggleHdmi(int8_t source);
int8_t ToggleI2S(int8_t source);
int8_t HdmiNibToSource(int8_t nibValue);
int8_t I2sNibToSource(int8_t nibValue);
void SwitchHdmi();
void SwitchI2s();
#endif
