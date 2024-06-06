/*
    njw1194 datasheet
    https://www.nisshinbo-microdevices.co.jp/en/pdf/datasheet/NJW1194_E.pdf
*/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "esp_log.h"
#include "unistd.h"
#include "string.h"
#include "spi.h"
#include "amp_parameters.h"
#ifndef _njw_parameters_H_
#define _njw_parameters_H_
static const uint8_t LOGE_NJW = true;
static const uint8_t LOGI_NJW = false;

// chip address (byte[0], low nibble)
static const uint8_t CHIP_ADDRESS = 0x00;


// select address (byte[0], high nibble)
static const uint8_t VOLUME_CTRL1 = 0x00; // byte[0], high nibble
static const uint8_t VOLUME_CTRL2 = 0x01; // byte[0], high nibble
static const uint8_t INPUT_SEL = 0x02; // byte[0], high nibble
static const uint8_t TREBLE_CTRL = 0x03; // byte[0], high nibble
static const uint8_t BASS_CTRL = 0x04; // byte[0], high nibble

// data input select(byte[1])
static const uint8_t INPUT_MUTE = 0x00; // byte[1]
static const uint8_t INPUT_1 = 0x04; // byte[1]
static const uint8_t INPUT_2 = 0x08; // byte[1]
static const uint8_t INPUT_3 = 0x0C; // byte[1]
static const uint8_t INPUT_4 = 0x10; // byte[1]

// data volume(byte[1])
static const uint8_t NJW_VOL_MIN = 0xfe; // byte[1]
static const uint8_t NJW_VOL_MAX = 0x40; // byte[1]
static const uint8_t NJW_VOL_MUTE = 0xff; // byte[1]

// data treble, boost (byte[1])
static const uint8_t TONE_CUT = 0x00; // byte[1]
static const uint8_t TONE_BOOST = 0x01; // byte[1]

//<TSW : Tone Control By-pass Switch > used with TREBLE_CTRL
static const uint8_t TONE_CTRL_OFF = 0x00; // byte[1]
static const uint8_t TONE_CTRL_ON = 0x01; // byte[1]

esp_err_t Njw1194Init(void);
esp_err_t NjwWrite(uint8_t chipAddress, uint8_t selectAddress, uint8_t data);
void SetAnalogInput(int8_t input);
void SetTone(int8_t state, uint8_t toneType, int8_t toneValue);
void SetToneCtrl(int8_t state, int8_t treble, int8_t bass);
void SetVolume(int8_t volume, int8_t balance);
#endif