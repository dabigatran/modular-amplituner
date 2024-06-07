#include "i2c.h"

esp_err_t I2CInit(void)
{
   i2c_config_t config = {0};
   config.mode = I2C_MODE_MASTER;
   config.sda_io_num = GPIO_NUM_21;
   config.scl_io_num = GPIO_NUM_22;
   config.sda_pullup_en = GPIO_PULLUP_ENABLE;
   config.scl_pullup_en = GPIO_PULLUP_ENABLE;
   config.master.clk_speed = 400000;
   
   esp_err_t ret = i2c_param_config(I2C_PORT, &config);
   if (LOGE_I2C && ret != ESP_OK)
   {
      ESP_LOGE(I2C_TAG, "Unable to set parameters.");
      return ret;
   }
   ret = i2c_driver_install(I2C_PORT, config.mode, I2C_TX_BUF_DISABLE, I2C_RX_BUF_DISABLE, INTR_FLAGS);
   if (LOGE_I2C && ret!= ESP_OK)
   {
      ESP_LOGE(I2C_TAG, "Unable to install driver.");
   }
   else if (LOGI_I2C)
   {
      ESP_LOGI(I2C_TAG, "Driver initialized.");
   }
   return ret;
}

esp_err_t I2CWrite(uint8_t address, uint8_t size, uint8_t *data)
{
   i2c_cmd_handle_t cmd = i2c_cmd_link_create();
   i2c_master_start(cmd);
   i2c_master_write_byte(cmd, address << 1 | WRITE, ACK);
   i2c_master_write(cmd, data, size, ACK);
   i2c_master_stop(cmd);
   esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, cmd, 1000 / portTICK_PERIOD_MS);
   i2c_cmd_link_delete(cmd);
   if (LOGE_I2C && ret == ESP_FAIL)
   {
         ESP_LOGE(I2C_TAG, "Unable to write (address %02x).", address);
   }
   else if (LOGI_I2C)
   {
      ESP_LOGI(I2C_TAG, "Write OK (address %02x).", address);
   }
   return ret;
}

esp_err_t I2CRead(uint8_t address, uint8_t reg, uint8_t size, uint8_t *data)
{
   // writing to register before reading is a dummy as explained in datasheet to delect register
   i2c_cmd_handle_t cmd = i2c_cmd_link_create();
   i2c_master_start(cmd);
   i2c_master_write_byte(cmd, address << 1 | WRITE, ACK);
   i2c_master_write_byte(cmd, reg, ACK);
   i2c_master_stop(cmd);
   esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, cmd, 1000 / portTICK_PERIOD_MS);
   i2c_cmd_link_delete(cmd);
   if (LOGE_I2C && ret == ESP_FAIL)
   {
         ESP_LOGE(I2C_TAG, "Unable to write (address %02x, reg %02x", address, reg);
   }
   // read byte
   cmd = i2c_cmd_link_create();
   i2c_master_start(cmd);
   i2c_master_write_byte(cmd, address << 1 | READ, ACK); // mcp->i2c_addr << 1 if address does not work
   i2c_master_read(cmd, data, size, ACK);
   i2c_master_stop(cmd);
   ret = i2c_master_cmd_begin(I2C_PORT, cmd, 1000 / portTICK_PERIOD_MS);
   i2c_cmd_link_delete(cmd);
   if (LOGE_I2C && ret == ESP_FAIL)
   {
         ESP_LOGE(I2C_TAG, "Unable to read (address %02x, reg %02x", address, reg);
   }
   else if (LOGI_I2C)
   {
      ESP_LOGI(I2C_TAG, "Read OK (address %02x, reg %02x).", address, reg);
   }
   return ret;
}