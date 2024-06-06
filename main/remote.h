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

#ifndef _remote_parameters_H_
#define _remote_parameters_H_
static const uint8_t LOGE_RMT = true;
static const uint8_t LOGI_RMT = false;


static const uint8_t REMOTE_DATA = 4; // remote data 4 bytes
static const int32_t IR_RESOLUTION_HZ = 1000000;
static const int32_t IR_RX_GPIO_NUM = 19;
static const int32_t IR_NEC_DECODE_MARGIN = 200;


static const int32_t NEC_LEADING_CODE_DURATION_0 = 9000;
static const int32_t NEC_LEADING_CODE_DURATION_1 = 4500;
static const int32_t NEC_PAYLOAD_ZERO_DURATION_0 = 560;
static const int32_t NEC_PAYLOAD_ZERO_DURATION_1 = 560;
static const int32_t NEC_PAYLOAD_ONE_DURATION_0 = 560;
static const int32_t NEC_PAYLOAD_ONE_DURATION_1 = 1690;
static const int32_t NEC_REPEAT_CODE_DURATION_0 = 9000;
static const int32_t NEC_REPEAT_CODE_DURATION_1 = 2250;

static const uint16_t CHROMECAST_ADDRESS = 0x857A;
static const uint16_t CHROMECAST_ON_OFF = 0xE11E;
static const uint16_t CHROMECAST_INPUT_TOGGLE = 0xE21D;
static const uint16_t CHROMECAST_VOLUME_UP = 0xE51A;
static const uint16_t CHROMECAST_VOLUME_DOWN = 0xE41B;
static const uint16_t CHROMECAST_MUTE_UNMUTE = 0xE31C;

bool RmtRxDoneCallback(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t *edata, void *userData);
int8_t RemoteParse(int8_t (*tunerState)[VAR_NO], QueueHandle_t remoteQueue);
void ParseFrame(rmt_symbol_word_t *rmtNecSymbols, size_t symbolNum, QueueHandle_t remoteQueue);
bool NecParseFrame(rmt_symbol_word_t *rmtNecSymbols, uint16_t *remoteCode);
bool NecCheckInRange(uint32_t signalDuration, uint32_t specDuration);
bool NecParseLogic0(rmt_symbol_word_t *rmtNecSymbols);
bool NecParseLogic1(rmt_symbol_word_t *rmtNecSymbols);
#endif