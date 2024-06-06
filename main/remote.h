/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 * Modified by dabigatran
 */

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/rmt_rx.h"
#include "amp_parameters.h"
#include "encoder.h"

#define REMOTE_DATA (uint8_t)4 // remote data 4 bytes
#define IR_RESOLUTION_HZ 1000000
#define IR_RX_GPIO_NUM 19
#define IR_NEC_DECODE_MARGIN 200

#define NEC_LEADING_CODE_DURATION_0 9000
#define NEC_LEADING_CODE_DURATION_1 4500
#define NEC_PAYLOAD_ZERO_DURATION_0 560
#define NEC_PAYLOAD_ZERO_DURATION_1 560
#define NEC_PAYLOAD_ONE_DURATION_0 560
#define NEC_PAYLOAD_ONE_DURATION_1 1690
#define NEC_REPEAT_CODE_DURATION_0 9000
#define NEC_REPEAT_CODE_DURATION_1 2250

#define CHROMECAST_ADDRESS (uint16_t)0x857A
#define CHROMECAST_ON_OFF (uint16_t)0xE11E
#define CHROMECAST_INPUT_TOGGLE (uint16_t)0xE21D
#define CHROMECAST_VOLUME_UP (uint16_t)0xE51A
#define CHROMECAST_VOLUME_DOWN (uint16_t)0xE41B
#define CHROMECAST_MUTE_UNMUTE (uint16_t)0xE31C

bool RmtRxDoneCallback(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t *edata, void *userData);
int8_t RemoteParse(int8_t (*tunerState)[VAR_NO], QueueHandle_t remoteQueue);
void ParseFrame(rmt_symbol_word_t *rmtNecSymbols, size_t symbolNum, QueueHandle_t remoteQueue);
bool NecParseFrame(rmt_symbol_word_t *rmtNecSymbols, uint16_t *remoteCode);
bool NecCheckInRange(uint32_t signalDuration, uint32_t specDuration);
bool NecParseLogic0(rmt_symbol_word_t *rmtNecSymbols);
bool NecParseLogic1(rmt_symbol_word_t *rmtNecSymbols);