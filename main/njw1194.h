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

// chip address (byte[0], low nibble)
#define CHIP_ADDRESS (uint8_t)0x00 // byte[0],  low nibble

// select address (byte[0], high nibble)
#define VOLUME_CTRL1 (uint8_t)0x00 // byte[0], high nibble
#define VOLUME_CTRL2 (uint8_t)0x01 // byte[0], high nibble
#define INPUT_SEL (uint8_t)0x02    // byte[0], high nibble
#define TREBLE_CTRL (uint8_t)0x03  // byte[0], high nibble
#define BASS_CTRL (uint8_t)0x04    // byte[0], high nibble

// data input select(byte[1])
#define INPUT_MUTE (uint8_t)0x00 // byte[1]
#define INPUT_1 (uint8_t)0x04    // byte[1]
#define INPUT_2 (uint8_t)0x08    // byte[1]
#define INPUT_3 (uint8_t)0x0C    // byte[1]
#define INPUT_4 (uint8_t)0x10    // byte[1]

// data volume(byte[1])
#define NJW_VOL_MIN (uint8_t)0xfe  // byte[1]
#define NJW_VOL_MAX (uint8_t)0x40  // byte[1]
#define NJW_VOL_MUTE (uint8_t)0xff // byte[1]

// data treble, boost (byte[1])
#define TONE_CUT (uint8_t)0x00   // byte[1], bit 8
#define TONE_BOOST (uint8_t)0x01 // byte[1], bit 8

//<TSW : Tone Control By-pass Switch > used with TREBLE_CTRL
#define TONE_CTRL_OFF (uint8_t)0x00 // byte[1], bit 2
#define TONE_CTRL_ON (uint8_t)0x01  // byte[1], bit 2

#define LOGE_NJW 0x01
#define LOGI_NJW 0x00

esp_err_t Njw1194Init(void);
esp_err_t NjwWrite(uint8_t chip_address, uint8_t select_address, uint8_t data);
void SetAnalogInput(int8_t input);
void SetTone(int8_t state, uint8_t tone_type, int8_t tone_value);
void SetToneCtrl(int8_t state, int8_t treble, int8_t bass);
void SetVolume(int8_t volume, int8_t balance);
