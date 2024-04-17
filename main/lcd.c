#include "lcd.h"
static const char *TAG = "LCD";

esp_err_t lcd_init (void){	
	ESP_LOGI(TAG, "Starting initialization (address %02x).", LCD_ADDRESS);
	esp_err_t success=0;
	usleep(50000);
	success = success + lcd_cmd (FUNCTION_SET_0);
	usleep(5000); 
	success = success + lcd_cmd (FUNCTION_SET_0);
	usleep(200);  
	success = success + lcd_cmd (FUNCTION_SET_0);
	usleep(200); 
	success = success + lcd_cmd (FUNCTION_SET_1);
	usleep(200); 

	success = success + lcd_cmd (FUNCTION_SET_2);
	usleep(1000);
	success = success + lcd_cmd (DISPLAY_SWITCH_OFF);
	usleep(1000);
	success = success + lcd_cmd (SCREEN_CLEAR);
	usleep(2000);
	success = success + lcd_cmd (INPUT_SET);
	usleep(1000);
	success = success + lcd_cmd (DISPLAY_SWITCH_ON); 
	usleep(1000);
	if (success!=ESP_OK)
		ESP_LOGE(TAG, "Initialization failed.");
   	else
      	ESP_LOGI(TAG, "Initialization OK.");
   	return success;
}

esp_err_t lcd_cmd (char cmd){
	if(LOGI_LCD) ESP_LOGI(TAG, "Command: %02x", cmd);
	uint8_t i2c_data[4]={0};
	adapt_to_4bits (true, 0, &cmd, 0, i2c_data);
	esp_err_t ret =i2c_write(LCD_ADDRESS, 4, i2c_data);
	if (ret!=ESP_OK) {
		if(LOGE_LCD) ESP_LOGE(TAG, "Command failed (%02x).", cmd);
   	}
   	else
      	if(LOGI_LCD) ESP_LOGI(TAG, "Command OK (%02x).", cmd);
   	return ret;
}

esp_err_t lcd_byte (bool is_cmd, char data){
	if(LOGI_LCD) ESP_LOGI(TAG, "Write: %02x", data);
	uint8_t i2c_data[4]={0};
	adapt_to_4bits (is_cmd, 0, &data, 0, i2c_data);
	esp_err_t ret = i2c_write(LCD_ADDRESS, 4, i2c_data);
	if (ret!=ESP_OK) {
		if(LOGE_LCD) ESP_LOGE(TAG, "Write failed (%02x).", data);
   	}
   	else
      	if(LOGI_LCD) ESP_LOGI(TAG, "Write OK (%02x).", data);
   	return ret;
}

esp_err_t lcd_text (uint8_t row, uint8_t col, char *data){	
	if(LOGI_LCD) ESP_LOGI(TAG, "Text write ('%s').", data);
	uint8_t data_length = strlen(data);
	uint8_t i2c_data_length = 4*(data_length+1) * sizeof(uint8_t);
	uint8_t *i2c_data = malloc(i2c_data_length);
	uint8_t d_index=0;
	uint8_t s_index=0;
	while (s_index<data_length){ 
		if(s_index==0){
    		char cmd= lcd_position (row, col);
			adapt_to_4bits (true, s_index, &cmd, d_index, i2c_data);
			d_index++;
		}
		adapt_to_4bits (false, s_index, data, d_index, i2c_data);
		d_index++;
		s_index++;
	};
	esp_err_t ret = i2c_write(LCD_ADDRESS, i2c_data_length, i2c_data);
	free(i2c_data);
	if (ret!=ESP_OK) {
		if(LOGE_LCD) ESP_LOGE(TAG, "Text write failed ('%s').", data);
   	}
   	else
      	if(LOGI_LCD) ESP_LOGI(TAG, "Text write OK ('%s').", data);
   	return ret;
}

esp_err_t lcd_backlight_on(){
	uint8_t data=BACKLIGHT_ON;
	esp_err_t ret = i2c_write(LCD_ADDRESS, 1, &data);
	if (ret!=ESP_OK){ 
		if(LOGE_LCD) ESP_LOGE(TAG, "Backlight on failed");
	}
	else
      	if(LOGI_LCD) ESP_LOGI(TAG, "Backlight on OK.");
   	return ret;
}

esp_err_t lcd_backlight_off(){
	uint8_t data=BACKLIGHT_OFF;
	esp_err_t ret = i2c_write(LCD_ADDRESS, 1, &data);
	if (ret!=ESP_OK){
		if(LOGE_LCD) ESP_LOGE(TAG, "Backlight off failed.");
	}
	else
      	if(LOGI_LCD) ESP_LOGI(TAG, "Backlight off OK.");
   	return ret;
}

esp_err_t lcd_clear_line (uint8_t line){
	esp_err_t ret = lcd_text (line, 0, "               ");
	if (ret!=ESP_OK) {
		if(LOGE_LCD) ESP_LOGE(TAG, "Clear line failed.");
   	}
   	else
      	if(LOGI_LCD) ESP_LOGI(TAG, "Clear line OK.");
   	return ret;
}

esp_err_t lcd_clear (void){
	esp_err_t ret = lcd_cmd (0x01);
	vTaskDelay(12 / (portTICK_PERIOD_MS));
	if (ret!=ESP_OK) {
		if(LOGE_LCD) ESP_LOGE(TAG, "Clear display failed.");
   	}
   	else
      	if(LOGI_LCD) ESP_LOGI(TAG, "Clear display OK.");
   	return ret;
}

esp_err_t lcd_cursor(uint8_t row, uint8_t col){
    uint16_t cmd= lcd_position (row, col);
    esp_err_t ret = lcd_cmd (cmd);
	if (ret!=ESP_OK) {
		if(LOGE_LCD) ESP_LOGI(TAG, "Starting cursor set.");
   	}
   	else
      	if(LOGI_LCD) ESP_LOGI(TAG, "Cursor set OK.");
   	return ret;
}

uint8_t lcd_position(uint8_t row, uint8_t col){
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

void adapt_to_4bits (bool is_cmd, uint8_t s_index, char *source_data, uint8_t d_index, uint8_t *dest_data){
	char en_rs[2];
	char high_nib, low_nib;
	if (is_cmd){
		en_rs[0] = EN_RS_CMD1;
		en_rs[1] = EN_RS_CMD2;
	}
	else{
		en_rs[0] = EN_RS_DATA1;
		en_rs[1] = EN_RS_DATA2;
	}
	high_nib = (source_data[s_index]&0xf0);
	low_nib = ((source_data[s_index]<<4)&0xf0);
	
	dest_data[d_index*4] = high_nib|en_rs[0];  
	dest_data[d_index*4+1] = high_nib|en_rs[1];
	dest_data[d_index*4+2]= low_nib|en_rs[0]; 
	dest_data[d_index*4+3] = low_nib|en_rs[1];
}

