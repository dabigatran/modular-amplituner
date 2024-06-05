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

/* MCP1, GPIOB values (8 bits), PIN 2-7 */
#define ENCODER_DATA (uint8_t)2
#define SNAP_SIZE (int8_t)4
#define ENC1_PINA (uint8_t)2
#define ENC1_PINB (uint8_t)3
#define ENC1_SW_PIN (uint8_t)4
#define ENC2_PINA (uint8_t)5
#define ENC2_PINB (uint8_t)6
#define ENC2_SW_PIN (uint8_t)7

#define PINA (int8_t)0
#define PINB (int8_t)1
#define PINA_VAL (int8_t)2
#define PINB_VAL (int8_t)3

#define MAIN_BUT_PIN (uint8_t)0x14
#define INTR_PIN GPIO_NUM_4
#define INTR_DEFAULT_FLAG 0
#define TRUE 1
#define FALSE 0

#define ENC1_TOG 0
#define ENC1_SW 1
#define ENC2_TOG 2
#define ENC2_SW 3

#define FALSE 0

#define LOGE_ENC (uint8_t)0x1
#define LOGI_ENC (uint8_t)0x0

int8_t CreateSnapshot(int8_t (*tuner_state)[VAR_NO], uint8_t *interrupt_data);
int8_t CheckInterrupt(int8_t (*tuner_state)[VAR_NO]);
int8_t CheckOverflow(int8_t (*tuner_state)[VAR_NO], int8_t select);
int8_t DecodePins(uint8_t *snapshot, int8_t (*tuner_state)[VAR_NO]);
int8_t OptionControl(uint8_t *snapshot, int8_t (*tuner_state)[VAR_NO], int8_t option);
int8_t ButtonControl(int8_t (*tuner_state)[VAR_NO], int8_t option);
int8_t Mute(int8_t (*tuner_state)[VAR_NO], int8_t option);
int8_t Toggle(int8_t (*tuner_state)[VAR_NO], int8_t option);
