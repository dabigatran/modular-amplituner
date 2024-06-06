#include "mcp23017.h"


esp_err_t McpInit(uint8_t address)
{
   if (LOGI_MCP)
   {
      ESP_LOGI(MCP_TAG, "Initialization (address %02x).", address);
   }
   esp_err_t success = 0;
   uint8_t value0 = 0x00;
   uint8_t value1 = 0xff;
   uint8_t value3 = 0xfc;// value for IODIRB pins 0-1 as output, 2-7 as input-> 0b11111100                 
   uint8_t value4 = 0x04; // value for IOCONB - INTB - OPendrain
   uint8_t value5 = 0xb4;

   // Interrupts bank B -OpenDrain (IOCONB - config register)
   success =success + McpWrite(address, IOCONB, 1, &value4);

   // Set (IODIRA register) pins 0-7 bank A as INPUT
   success =success + McpWrite(address, IODIRA, 1, &value1);
   
   // Reverse polarization (IPOLA register) on pins 0-7 bank A
   success =success + McpWrite(address, IPOLA, 1, &value1);
      
   // Set (IODIRB register) pins:  0-1 as output, 2-7 as input-> 0b00111111
   success =success + McpWrite(address, IODIRB, 1, &value3);

   // Turn off Pullup resistors (GPPUA register) on pins 0-7 bank A
   success =success + McpWrite(address, GPPUA, 1, &value0);

   // Turn on Pullup resistBFors (GPPUB register) on pins 2-7 bank B
   success =success + McpWrite(address, GPPUB, 1, &value3);

   // Reverse polarization (IPOLB register) on pins 2-7 bank B
   success =success + McpWrite(address, IPOLB, 1, &value3);

   // Interrupt on change (GPINTENB register) on pins 2-7 bank B
   success =success + McpWrite(address, GPINTENB, 1, &value5);

   // DEFAULT COMPARE REGISTER FOR INTERRUPT-ON-CHANGE (DEFVALB register) on pins 2-7 bank B
   success =success + McpWrite(address, DEFVALB, 1, &value0);

   // INTERRUPT CONTROL REGISTER (INCONB), If a bit is set, the corresponding I/O pin is compared
   // against the associated bit in the DEFVAL register.
   success =success + McpWrite(address, INTCONB, 1, &value0);
   
   if (LOGE_MCP && success != ESP_OK)
   {
      ESP_LOGE(MCP_TAG, "Intitialization failed (address %02x).", address);
   }
   else if (LOGI_MCP)
   {
      ESP_LOGI(MCP_TAG, "Initialization OK (address %02x).", address);
   }
   return success;
}

esp_err_t McpWrite(uint8_t address, uint8_t reg, uint8_t size, uint8_t *data)
{
   // concancate register and data to send those two values in i2cData
   uint8_t i2cSize = size + 1;
   uint8_t *i2cData = malloc((i2cSize) * sizeof(uint8_t));
   i2cData[0] = reg;
   memcpy(i2cData + (uint8_t)1, data, size);
   esp_err_t ret = I2CWrite(address, i2cSize, i2cData);
   if (LOGE_MCP && ret != ESP_OK)
   {
      ESP_LOGE(MCP_TAG, "Write failed (address %02x, reg %02x, value %02x).", address, reg, *data);
   }
   else if (LOGI_MCP)
   {
      ESP_LOGI(MCP_TAG, "Write OK (address %02x, reg %02x, value %02x).", address, reg, *data);
   }   
   free(i2cData);
   return ret;
}

esp_err_t McpRead(uint8_t address, uint8_t reg, uint8_t *data)
{
   esp_err_t ret = I2CRead(address, reg, 1, data);
   if (LOGE_MCP && ret != ESP_OK)
   {
      ESP_LOGE(MCP_TAG, "Read failed (address %02x, reg %02x).", address, reg);
   }
   else if (LOGI_MCP)
   {
      ESP_LOGI(MCP_TAG, "Read OK (address %02x, reg %02x, data %02x).", address, reg, *data);
   }
   return ret;
}

esp_err_t McpSetPin(uint8_t address, uint8_t reg, uint8_t pin)
{
   uint8_t value;
   uint8_t size = 1;
   esp_err_t ret = McpRead(address, reg, &value); 
   if (LOGE_MCP && ret!= ESP_OK)
   {
      return ESP_FAIL;
   }
   value |= 1 << pin;
   ret = McpWrite(address, reg, size, &value);
   if (LOGE_MCP && ret != ESP_OK)
   {
      return ESP_FAIL;
   }
   else if (LOGI_MCP)
   {
      ESP_LOGI(MCP_TAG, "Set pin OK (address %02x, reg %02x, pin %d).", address, reg, pin);
   }
   return ESP_OK;
}

esp_err_t McpClearPin(uint8_t address, uint8_t reg, uint8_t pin)
{
   uint8_t value;
   uint8_t size = 1;
   esp_err_t ret = McpRead(address, reg, &value); 
   if (LOGE_MCP && ret != ESP_OK)
   {
      return ESP_FAIL;
   }
   value &= ~(1 << pin);
   ret = McpWrite(address, reg, size, &value);
   if (LOGE_MCP && ret != ESP_OK)
   {
      return ESP_FAIL;
   }
   else if (LOGI_MCP)
   {
      ESP_LOGI(MCP_TAG, "Cleared pin OK (address %02x, reg %02x, bit %d).", address, reg, pin);
   }
   return ESP_OK;
};
