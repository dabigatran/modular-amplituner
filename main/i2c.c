#include "i2c.h"

static const char *TAG = "I2C";
static const size_t I2C_TX_BUF_DISABLE = 0;
static const size_t I2C_RX_BUF_DISABLE = 0;
static const int INTR_FLAGS = 0;

esp_err_t i2c_init(void)
{
   i2c_config_t config = {
       .mode = I2C_MODE_MASTER,
       .sda_io_num = GPIO_NUM_21,
       .scl_io_num = GPIO_NUM_22,
       .sda_pullup_en = GPIO_PULLUP_ENABLE,
       .scl_pullup_en = GPIO_PULLUP_ENABLE,
       .master.clk_speed = 400000,
   };

   esp_err_t ret = i2c_param_config(I2C_PORT, &config);
   if (ret != ESP_OK)
   {
      ESP_LOGE(TAG, "Unable to set parameters.");
      ESP_LOGE(TAG, "Description: %s", esp_err_to_name(ret));
      return ret;
   }
   ret = i2c_driver_install(I2C_PORT, config.mode, I2C_TX_BUF_DISABLE, I2C_RX_BUF_DISABLE, INTR_FLAGS);
   if (ret != ESP_OK)
   {
      ESP_LOGE(TAG, "Unable to install driver.");
      ESP_LOGE(TAG, "Description: %s", esp_err_to_name(ret));
   }
   else
      ESP_LOGI(TAG, "Driver initialized.");
   return ret;
}

esp_err_t i2c_write(uint8_t address, uint8_t size, uint8_t *data)
{
   i2c_cmd_handle_t cmd = i2c_cmd_link_create();
   i2c_master_start(cmd);
   i2c_master_write_byte(cmd, address << 1 | WRITE, ACK);
   i2c_master_write(cmd, data, size, ACK);
   i2c_master_stop(cmd);
   esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, cmd, 1000 / portTICK_PERIOD_MS);
   i2c_cmd_link_delete(cmd);
   if (ret == ESP_FAIL)
   {
      if (LOGE_I2C)
         ESP_LOGE(TAG, "Unable to write (address %02x).", address);
      if (LOGE_I2C)
         ESP_LOGE(TAG, "description: %s", esp_err_to_name(ret));
   }
   else if (LOGI_I2C)
      ESP_LOGI(TAG, "Write OK (address %02x).", address);

   return ret;
}

esp_err_t i2c_read(uint8_t address, uint8_t reg, uint8_t size, uint8_t *data)
{
   // writing to register before reading is a dummy as explained in datasheet to delect register
   i2c_cmd_handle_t cmd = i2c_cmd_link_create();
   i2c_master_start(cmd);
   i2c_master_write_byte(cmd, address << 1 | WRITE, ACK);
   i2c_master_write_byte(cmd, reg, ACK);
   i2c_master_stop(cmd);
   esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, cmd, 1000 / portTICK_PERIOD_MS);
   i2c_cmd_link_delete(cmd);
   if (ret == ESP_FAIL)
   {
      if (LOGE_I2C)
         ESP_LOGE(TAG, "Unable to write (address %02x, reg %02x", address, reg);
      if (LOGE_I2C)
         ESP_LOGE(TAG, "Description: %s", esp_err_to_name(ret));
   }
   // read byte
   cmd = i2c_cmd_link_create();
   i2c_master_start(cmd);
   i2c_master_write_byte(cmd, address << 1 | READ, ACK); // mcp->i2c_addr << 1 if address does not work
   i2c_master_read(cmd, data, size, ACK);
   i2c_master_stop(cmd);
   ret = i2c_master_cmd_begin(I2C_PORT, cmd, 1000 / portTICK_PERIOD_MS);
   i2c_cmd_link_delete(cmd);
   if (ret == ESP_FAIL)
   {
      if (LOGE_I2C)
         ESP_LOGE(TAG, "Unable to read (address %02x, reg %02x", address, reg);
      if (LOGE_I2C)
         ESP_LOGE(TAG, "Description: %s", esp_err_to_name(ret));
   }
   else if (LOGI_I2C)
      ESP_LOGI(TAG, "Read OK (address %02x, reg %02x).", address, reg);
   return ret;
}