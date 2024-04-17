#include "mcp23017.h"

static const char* TAG = "MCP23017";
esp_err_t mcp_init(uint8_t address) {
   ESP_LOGI(TAG, "Initialization (address %02x).", address);
   uint8_t value0=0x00; 
   uint8_t value1=0xff;
   uint8_t value3=0xfc; ; //value for IODIRB pins 0-1 as output, 2-7 as input-> 0b11111100
   uint8_t value4=0x04;//value for IOCONB - INTB - OPendrain
   uint8_t value5=0xb4;
   esp_err_t ret;
      //Interrupts bank B -OpenDrain (IOCONB - config register)
      ret = mcp_write(address, IOCONB, 1, &value4); 
   //Set (IODIRA register) pins 0-7 bank A as INPUT   
      ret = mcp_write(address, IODIRA, 1, &value1); 
   // Reverse polarization (IPOLA register) on pins 0-7 bank A   
      ret = mcp_write(address, IPOLA, 1, &value1); 
	if (ret!=ESP_OK)
		ESP_LOGE(TAG, "Intitialization failed (address %02x).", address);
   //Set (IODIRB register) pins:  0-1 as output, 2-7 as input-> 0b00111111   
      ret = mcp_write (address, IODIRB, 1, &value3); 
   if (ret!=ESP_OK)
		ESP_LOGE(TAG, "Initialization failed (address %02x).", address);
   else
      ESP_LOGI(TAG, "Initialization OK (address %02x).", address);
   //Turn off Pullup resistors (GPPUA register) on pins 0-7 bank A   
      ret = mcp_write(address, GPPUA, 1, &value0);
   //Turn on Pullup resistBFors (GPPUB register) on pins 2-7 bank B   
      ret = mcp_write(address, GPPUB, 1, &value3);
   //Reverse polarization (IPOLB register) on pins 2-7 bank B   
     ret = mcp_write(address, IPOLB, 1, &value3); 
   //Interrupt on change (GPINTENB register) on pins 2-7 bank B   
   ret = mcp_write(address, GPINTENB, 1, &value5);
   
   //DEFAULT COMPARE REGISTER FOR INTERRUPT-ON-CHANGE (DEFVALB register) on pins 2-7 bank B   
    ret = mcp_write(address, DEFVALB, 1, &value0);

   //INTERRUPT CONTROL REGISTER (INCONB), If a bit is set, the corresponding I/O pin is compared
   //against the associated bit in the DEFVAL register.
   ret = mcp_write(address, INTCONB, 1, &value0);
   return ret;
}

esp_err_t mcp_write(uint8_t address, uint8_t reg, uint8_t size, uint8_t *data) {
   
   //concancate register and data to send those two values in i2c_data
   uint8_t i2c_size=size+1;
   uint8_t *i2c_data = malloc((i2c_size) * sizeof(uint8_t));
   i2c_data[0]=reg;
   memcpy(i2c_data+(uint8_t)1, data, size);
   esp_err_t ret= i2c_write(address, i2c_size, i2c_data); 
   if (ret!=ESP_OK){
		if(LOGE_MCP) ESP_LOGE(TAG, "Write failed (address %02x, reg %02x, value %02x).", address, reg, *data);
   }
   else
      if(LOGI_MCP) ESP_LOGI(TAG, "Write OK (address %02x, reg %02x, value %02x).", address, reg, *data);
   free(i2c_data);
   return ret;
}

esp_err_t mcp_read(uint8_t address, uint8_t reg, uint8_t *data) {
   esp_err_t ret= i2c_read(address, reg, 1, data); 
   if (ret!=ESP_OK){
		if(LOGE_MCP) ESP_LOGE(TAG, "Read failed (address %02x, reg %02x).", address, reg);
   }
   else
      if(LOGI_MCP) ESP_LOGI(TAG, "Read OK (address %02x, reg %02x, data %02x).", address, reg, *data);
   return ret;
}

esp_err_t mcp_set_pin(uint8_t address,  uint8_t reg, uint8_t pin){
   uint8_t value;
   uint8_t size =1;
   if(mcp_read(address, reg, &value) != ESP_OK) {
      if(LOGE_MCP) ESP_LOGE(TAG, "Unable to read pin %d value (address %02x, reg %02x).",pin, address, reg);
      return ESP_FAIL;
   }
   value |= 1 << pin;
   if(mcp_write(address, reg, size, &value) != ESP_OK) {
      if(LOGE_MCP) ESP_LOGE(TAG, "Unable to set pin %d value (address %02x, reg %02x).", pin, address, reg);
      return ESP_FAIL;
   }
   else
      if(LOGI_MCP) ESP_LOGI(TAG, "Set pin OK (address %02x, reg %02x, pin %d).", address, reg, pin);
   return ESP_OK;
}

esp_err_t mcp_clear_pin(uint8_t address,  uint8_t reg, uint8_t pin){
   uint8_t value;
   uint8_t size =1;
   if(mcp_read(address, reg, &value) != ESP_OK) {
      if(LOGE_MCP) ESP_LOGE(TAG, "Unable to read bit %d value (address %02x, reg %02x).",pin, address, reg);
      return ESP_FAIL;
   }
   value &= ~(1 << pin);
   if(mcp_write(address, reg, size, &value) != ESP_OK) {
      if(LOGE_MCP) ESP_LOGE(TAG, "Unable to clear pin %02X on peripheral (address %02x, reg %02x).", pin, address, reg);
      return ESP_FAIL;
   }
   else
      if(LOGI_MCP) ESP_LOGI(TAG, "Cleared pin OK (address %02x, reg %02x, bit %d).", address, reg, pin);

   return ESP_OK;
};

