#include "njw1194.h"

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
static const uint8_t volumeMap[] = {
	0xff, 0xdf, 0xd0, 0xca, 0xc4, 0xbe, 0xb8, 0xb2, 0xac, 0xa7, 0xa2, 0x9d, 0x98, 0x93, 0x8e, 0x8a, 0x86, 0x82, 0x7e, 0x7a, 0x76,
	0x72, 0x6e, 0x6b, 0x68, 0x65, 0x60, 0x5e, 0x5c, 0x5a, 0x58, 0x56, 0x54, 0x52, 0x50, 0x4f, 0x4e, 0x4d, 0x4c, 0x4b, 0x4a};

esp_err_t Njw1194Init(void)
{
	/* No need to init njw1194*/
	return ESP_OK;
}

esp_err_t NjwWrite(uint8_t chipAddress, uint8_t selectAddress, uint8_t data)
{
	uint8_t spiData[2] = {0, 0};
	spiData[1] = chipAddress | (selectAddress << 4);
	spiData[0] = data;
	esp_err_t ret = SpiWrite(2 * 8, spiData);
	if (LOGE_NJW && ret != ESP_OK)
	{
		ESP_LOGE(NJW_TAG, "NJW1194 write failed: address %02x, data %02x.", spiData[0], spiData[1]);
	}
	else if (LOGI_NJW)
	{
		ESP_LOGI(NJW_TAG, "NJW1194 write OK: address %02x, data %02x.", spiData[0], spiData[1]);
	}
	return ret;
}

void SetVolume(int8_t volume, int8_t balance)
{
	uint8_t njwVolumeL = volumeMap[VOL_MIN];
	uint8_t njwVolumeR = volumeMap[VOL_MIN];
	float njwBalance = VOL_MIN;
	switch (balance)
	{
	case 0:
		njwVolumeL = volumeMap[volume];
		njwVolumeR = njwVolumeL;
		break;
	case -19 ... - 1:
		njwBalance = -1 * ((float)balance / 40) * volumeMap[volume];
		njwVolumeL = volumeMap[volume];
		njwVolumeR = volumeMap[volume] + (uint8_t)njwBalance;
		if (njwVolumeR < volumeMap[VOL_MAX])
			njwVolumeR = volumeMap[VOL_MIN];
		break;
	case -20:
		njwVolumeL = volumeMap[volume];
		break;
	case 1 ... 19:
		njwBalance = ((float)balance / 40) * volumeMap[volume];
		njwVolumeR = volumeMap[volume];
		njwVolumeL = volumeMap[volume] + (uint8_t)njwBalance;
		if (njwVolumeL < volumeMap[VOL_MAX])
			njwVolumeL = volumeMap[VOL_MIN];
		break;
	case 20:
		njwVolumeR = volumeMap[volume];
		break;
	}
	NjwWrite(CHIP_ADDRESS, VOLUME_CTRL1, njwVolumeL);
	NjwWrite(CHIP_ADDRESS, VOLUME_CTRL2, njwVolumeR);
}

void SetAnalogInput(int8_t input)
{
	uint8_t inputValue = (uint8_t)input;
	NjwWrite(CHIP_ADDRESS, INPUT_SEL, inputValue);
}

void SetToneCtrl(int8_t state, int8_t treble, int8_t bass)
{
	switch (state)
	{
	case 0:
		SetTone(state, TREBLE_CTRL, treble);
		break;
	case 1:
		SetTone(state, TREBLE_CTRL, treble);
		SetTone(state, BASS_CTRL, bass);
	}
}

void SetTone(int8_t state, uint8_t toneType, int8_t toneValue)
{
	uint8_t tone = 0;
	uint8_t toneCB = TONE_BOOST;
	if (toneValue <= 0)
	{
		toneCB = TONE_CUT;
		toneValue *= -1;
	}
	tone = (toneCB << 7) | (toneValue << 3) | (state << 2);
	NjwWrite(CHIP_ADDRESS, toneType, tone);
}
