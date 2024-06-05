#include "njw1194.h"
static const char *TAG = "njw1194";
/*
	njw1194 volume range:
	mute = value 255 and 0
	minimum = -95dB = 254
	middle = 0dB = 64
	maximum = +31.5dB = 1
	values 255->64 = without preamp boost (with cut from inputs)
	values 64->1 = with preamp boost
	I decided not to use preamp boost in this implementation.
		My range 255->254->64 (mute->minimun->maximum);
*/

// logarithmic map of njw1194 volume control (volume 0->20 ==> -95dB->0dB)
static uint8_t volume_map[] = {
	0xff, 0xdf, 0xd0, 0xca, 0xc4, 0xbe, 0xb8, 0xb2, 0xac, 0xa7, 0xa2, 0x9d, 0x98, 0x93, 0x8e, 0x8a, 0x86, 0x82, 0x7e, 0x7a, 0x76,
	0x72, 0x6e, 0x6b, 0x68, 0x65, 0x60, 0x5e, 0x5c, 0x5a, 0x58, 0x56, 0x54, 0x52, 0x50, 0x4f, 0x4e, 0x4d, 0x4c, 0x4b, 0x4a};

esp_err_t njw1194_init(void)
{
	/* No need to init njw1194*/
	return ESP_OK;
}

esp_err_t njw_write(uint8_t chip_address, uint8_t select_address, uint8_t data)
{
	if (LOGI_NJW)
		ESP_LOGI(TAG, "NJW1194 write.");
	uint8_t spi_data[2] = {0, 0};
	spi_data[1] = chip_address | (select_address << 4);
	spi_data[0] = data;
	esp_err_t ret = spi_write(2 * 8, spi_data);
	if (ret != ESP_OK)
	{
		if (LOGE_NJW)
			ESP_LOGE(TAG, "NJW1194 write failed: address %02x, data %02x.", spi_data[0], spi_data[1]);
	}
	else if (LOGI_NJW)
		ESP_LOGI(TAG, "NJW1194 write OK: address %02x, data %02x.", spi_data[0], spi_data[1]);
	return ret;
}

void set_volume(int8_t volume, int8_t balance)
{
	uint8_t njw_volumeL = volume_map[VOL_MIN];
	uint8_t njw_volumeR = volume_map[VOL_MIN];
	float njw_balance = VOL_MIN;
	switch (balance)
	{
	case 0:
		njw_volumeL = volume_map[volume];
		njw_volumeR = njw_volumeL;
		break;
	case -19 ... - 1:
		njw_balance = -1 * ((float)balance / 40) * volume_map[volume];
		njw_volumeL = volume_map[volume];
		njw_volumeR = volume_map[volume] + (uint8_t)njw_balance;
		if (njw_volumeR < volume_map[VOL_MAX])
			njw_volumeR = volume_map[VOL_MIN];
		break;
	case -20:
		njw_volumeL = volume_map[volume];
		break;
	case 1 ... 19:
		njw_balance = ((float)balance / 40) * volume_map[volume];
		njw_volumeR = volume_map[volume];
		njw_volumeL = volume_map[volume] + (uint8_t)njw_balance;
		if (njw_volumeL < volume_map[VOL_MAX])
			njw_volumeL = volume_map[VOL_MIN];
		break;
	case 20:
		njw_volumeR = volume_map[volume];
		break;
	}
	njw_write(CHIP_ADDRESS, VOLUME_CTRL1, njw_volumeL);
	njw_write(CHIP_ADDRESS, VOLUME_CTRL2, njw_volumeR);
}

void set_analog_input(int8_t input)
{
	uint8_t input_value = (uint8_t)input;
	njw_write(CHIP_ADDRESS, INPUT_SEL, input_value);
}

void set_tone_ctrl(int8_t state, int8_t treble, int8_t bass)
{
	switch (state)
	{
	case 0:
		set_tone(state, TREBLE_CTRL, treble);
		break;
	case 1:
		set_tone(state, TREBLE_CTRL, treble);
		set_tone(state, BASS_CTRL, bass);
	}
}

void set_tone(int8_t state, uint8_t tone_type, int8_t tone_value)
{
	uint8_t tone = 0;
	uint8_t tone_c_b = TONE_BOOST;
	if (tone_value <= 0)
	{
		tone_c_b = TONE_CUT;
		tone_value *= -1;
	}
	tone = (tone_c_b << 7) | (tone_value << 3) | (state << 2);
	njw_write(CHIP_ADDRESS, tone_type, tone);
}
