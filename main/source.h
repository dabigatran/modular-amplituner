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

// Number of ports
#define HDMI_NO (uint8_t)0x04
#define I2S_NO (uint8_t)0x04
#define MAX_SRC_ALIAS_LENGTH 16
#define MAX_OPT_ALIAS_LENGTH 12
#define MAX_SOURCES 9
#define SOURCE0 0
#define SOURCE1 1
#define SOURCE2 2
#define SOURCE3 3
#define SOURCE4 4
#define SOURCE5 5
#define SOURCE6 6
#define SOURCE7 7
#define SOURCE8 8
#define NOT_FOUND 0
#define FOUND 1
#define SEARCHED 0
#define SET 1

/* MCP1, GPIOA values (8 bits), ALL INPUT */
#define HDMI1 (uint8_t)0x11
#define HDMI2 (uint8_t)0x12
#define HDMI3 (uint8_t)0x14
#define HDMI4 (uint8_t)0x18

#define I2S1 (uint8_t)0x10
#define I2S2 (uint8_t)0x20
#define I2S3 (uint8_t)0x40
#define I2S4 (uint8_t)0x80

/* MCP1, GPIOB */
#define HDMI_SWITCH PB0 // OUTPUT
#define I2S_SWITCH PB1  // OUTPUT

#define LOGE_SRC (uint8_t)0x1
#define LOGI_SRC (uint8_t)0x0
void SourcesInit();
void MapSources();
void SetAvailibility();
void SetSource(int8_t (*tuner_state)[VAR_NO]);
int8_t ChangeSource(int8_t source);
int8_t IsHdmiAvailable();
int8_t ToggleHdmi(int8_t source);
int8_t ToggleI2S(int8_t source);
int8_t HdmiNibToSource(int8_t nib_value);
int8_t I2sNibToSource(int8_t nib_value);
void SwitchHdmi();
void SwitchI2s();
