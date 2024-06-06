#include "lcd.h"


esp_err_t LcdInit(void)
{
	ESP_LOGI(LCD_TAG, "Starting initialization (address %02x).", LCD_ADDRESS);
	esp_err_t success = 0;
	usleep(50000);
	success = success + LcdCmd(FUNCTION_SET_0);
	usleep(5000);
	success = success + LcdCmd(FUNCTION_SET_0);
	usleep(200);
	success = success + LcdCmd(FUNCTION_SET_0);
	usleep(200);
	success = success + LcdCmd(FUNCTION_SET_1);
	usleep(200);

	success = success + LcdCmd(FUNCTION_SET_2);
	usleep(1000);
	success = success + LcdCmd(DISPLAY_SWITCH_OFF);
	usleep(1000);
	success = success + LcdCmd(SCREEN_CLEAR);
	usleep(2000);
	success = success + LcdCmd(INPUT_SET);
	usleep(1000);
	success = success + LcdCmd(DISPLAY_SWITCH_ON);
	usleep(1000);
	if (success != ESP_OK)
		ESP_LOGE(LCD_TAG, "Initialization failed.");
	else
		ESP_LOGI(LCD_TAG, "Initialization OK.");
	return success;
}

esp_err_t LcdCmd(char cmd)
{
	if (LOGI_LCD)
		ESP_LOGI(LCD_TAG, "Command: %02x", cmd);
	uint8_t i2c_data[4] = {0};
	AdaptTo4bits(true, 0, &cmd, 0, i2c_data);
	esp_err_t ret = I2CWrite(LCD_ADDRESS, 4, i2c_data);
	if (ret != ESP_OK)
	{
		if (LOGE_LCD)
			ESP_LOGE(LCD_TAG, "Command failed (%02x).", cmd);
	}
	else if (LOGI_LCD)
		ESP_LOGI(LCD_TAG, "Command OK (%02x).", cmd);
	return ret;
}

esp_err_t LcdByte(bool isCmd, char data)
{
	if (LOGI_LCD)
		ESP_LOGI(LCD_TAG, "Write: %02x", data);
	uint8_t i2cData[4] = {0};
	AdaptTo4bits(isCmd, 0, &data, 0, i2cData);
	esp_err_t ret = I2CWrite(LCD_ADDRESS, 4, i2cData);
	if (ret != ESP_OK)
	{
		if (LOGE_LCD)
			ESP_LOGE(LCD_TAG, "Write failed (%02x).", data);
	}
	else if (LOGI_LCD)
		ESP_LOGI(LCD_TAG, "Write OK (%02x).", data);
	return ret;
}

esp_err_t LcdText(uint8_t row, uint8_t col, char *data)
{
	if (LOGI_LCD)
		ESP_LOGI(LCD_TAG, "Text write ('%s').", data);
	uint8_t dataLength = strlen(data);
	uint8_t i2cDataLength = 4 * (dataLength + 1) * sizeof(uint8_t);
	uint8_t *i2cData = malloc(i2cDataLength);
	uint8_t dIndex = 0;
	uint8_t sIndex = 0;
	while (sIndex < dataLength)
	{
		if (sIndex == 0)
		{
			char cmd = LcdPosition(row, col);
			AdaptTo4bits(true, sIndex, &cmd, dIndex, i2cData);
			dIndex++;
		}
		AdaptTo4bits(false, sIndex, data, dIndex, i2cData);
		dIndex++;
		sIndex++;
	};
	esp_err_t ret = I2CWrite(LCD_ADDRESS, i2cDataLength, i2cData);
	free(i2cData);
	if (ret != ESP_OK)
	{
		if (LOGE_LCD)
			ESP_LOGE(LCD_TAG, "Text write failed ('%s').", data);
	}
	else if (LOGI_LCD)
		ESP_LOGI(LCD_TAG, "Text write OK ('%s').", data);
	return ret;
}

esp_err_t LcdBacklightOn()
{
	uint8_t data = BACKLIGHT_ON;
	esp_err_t ret = I2CWrite(LCD_ADDRESS, 1, &data);
	if (ret != ESP_OK)
	{
		if (LOGE_LCD)
			ESP_LOGE(LCD_TAG, "Backlight on failed");
	}
	else if (LOGI_LCD)
		ESP_LOGI(LCD_TAG, "Backlight on OK.");
	return ret;
}

esp_err_t LcdBacklightOff()
{
	uint8_t data = BACKLIGHT_OFF;
	esp_err_t ret = I2CWrite(LCD_ADDRESS, 1, &data);
	if (ret != ESP_OK)
	{
		if (LOGE_LCD)
			ESP_LOGE(LCD_TAG, "Backlight off failed.");
	}
	else if (LOGI_LCD)
		ESP_LOGI(LCD_TAG, "Backlight off OK.");
	return ret;
}

esp_err_t LcdClearLine(uint8_t line)
{
	esp_err_t ret = LcdText(line, 0, "               ");
	if (ret != ESP_OK)
	{
		if (LOGE_LCD)
			ESP_LOGE(LCD_TAG, "Clear line failed.");
	}
	else if (LOGI_LCD)
		ESP_LOGI(LCD_TAG, "Clear line OK.");
	return ret;
}

esp_err_t LcdClear(void)
{
	esp_err_t ret = LcdCmd(0x01);
	vTaskDelay(12 / (portTICK_PERIOD_MS));
	if (ret != ESP_OK)
	{
		if (LOGE_LCD)
			ESP_LOGE(LCD_TAG, "Clear display failed.");
	}
	else if (LOGI_LCD)
		ESP_LOGI(LCD_TAG, "Clear display OK.");
	return ret;
}

esp_err_t LcdCursor(uint8_t row, uint8_t col)
{
	uint16_t cmd = LcdPosition(row, col);
	esp_err_t ret = LcdCmd(cmd);
	if (ret != ESP_OK)
	{
		if (LOGE_LCD)
			ESP_LOGI(LCD_TAG, "Starting cursor set.");
	}
	else if (LOGI_LCD)
		ESP_LOGI(LCD_TAG, "Cursor set OK.");
	return ret;
}

uint8_t LcdPosition(uint8_t row, uint8_t col)
{
	switch (row)
	{
	case 0:
		col |= LINE1;
		break;
	case 1:
		col |= LINE2;
		break;
	}
	return col;
}

void AdaptTo4bits(bool isCmd, uint8_t sIndex, char *sourceData, uint8_t dIndex, uint8_t *destData)
{
	char enRs[2];
	char highNib, lowNib;
	if (isCmd)
	{
		enRs[0] = EN_RS_CMD1;
		enRs[1] = EN_RS_CMD2;
	}
	else
	{
		enRs[0] = EN_RS_DATA1;
		enRs[1] = EN_RS_DATA2;
	}
	highNib = (sourceData[sIndex] & 0xf0);
	lowNib = ((sourceData[sIndex] << 4) & 0xf0);

	destData[dIndex * 4] = highNib | enRs[0];
	destData[dIndex * 4 + 1] = highNib | enRs[1];
	destData[dIndex * 4 + 2] = lowNib | enRs[0];
	destData[dIndex * 4 + 3] = lowNib | enRs[1];
}
