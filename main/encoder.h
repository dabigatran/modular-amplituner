#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <esp_err.h>
#include "esp_log.h"
#include "i2c.h"
#include "mcp23017.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "amp_parameters.h"

#ifndef _encoder_parameters_H_
#define _encoder_parameters_H_
static const uint8_t LOGE_ENC = true;
static const uint8_t LOGI_ENC = false;

/* MCP1, GPIOB values (8 bits), PIN 2-7 */
static const uint8_t ENCODER_DATA = 2;
static const int8_t SNAP_SIZE = 4;
static const uint8_t ENC1_PINA = 2;
static const uint8_t ENC1_PINB = 3;
static const uint8_t ENC1_SW_PIN = 4;
static const uint8_t ENC2_PINA = 5;
static const uint8_t ENC2_PINB = 6;
static const uint8_t ENC2_SW_PIN = 7;

static const int8_t PINA = 0;
static const int8_t PINB = 1;
static const int8_t PINA_VAL = 2;
static const int8_t PINB_VAL = 3;

static const uint8_t MAIN_BUT_PIN = 0x14;
static const gpio_num_t INTR_PIN = GPIO_NUM_4;
static const uint8_t INTR_DEFAULT_FLAG = 0;
static const uint8_t TRUE = 1;
static const uint8_t FALSE = 0;

static const uint8_t ENC1_TOG = 0;
static const uint8_t ENC1_SW = 1;
static const uint8_t ENC2_TOG = 2;
static const uint8_t ENC2_SW = 3;

int8_t CreateSnapshot(int8_t (*tunerState)[VAR_NO], uint8_t *interruptData);
int8_t CheckInterrupt(int8_t (*tunerState)[VAR_NO]);
int8_t CheckOverflow(int8_t (*tunerState)[VAR_NO], int8_t select);
int8_t DecodePins(uint8_t *snapshot, int8_t (*tunerState)[VAR_NO]);
int8_t OptionControl(uint8_t *snapshot, int8_t (*tunerState)[VAR_NO], int8_t option);
int8_t ButtonControl(int8_t (*tunerState)[VAR_NO], int8_t option);
int8_t Mute(int8_t (*tunerState)[VAR_NO], int8_t option);
int8_t Toggle(int8_t (*tunerState)[VAR_NO], int8_t option);
#endif