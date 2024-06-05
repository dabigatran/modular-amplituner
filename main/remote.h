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

bool rmt_rx_done_callback(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t *edata, void *user_data);
int8_t remote_parse(int8_t (*tuner_state)[VAR_NO], QueueHandle_t remote_queue);
void parse_frame(rmt_symbol_word_t *rmt_nec_symbols, size_t symbol_num, QueueHandle_t remote_queue);
bool nec_parse_frame(rmt_symbol_word_t *rmt_nec_symbols, uint16_t *remote_code);
bool nec_check_in_range(uint32_t signal_duration, uint32_t spec_duration);
bool nec_parse_logic0(rmt_symbol_word_t *rmt_nec_symbols);
bool nec_parse_logic1(rmt_symbol_word_t *rmt_nec_symbols);