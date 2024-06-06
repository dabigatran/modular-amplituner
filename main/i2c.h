#include <stdint.h>
#include <esp_err.h>
#include "esp_log.h"
#include "driver/i2c.h"
#ifndef _i2c_parameters_H_
#define _i2c_parameters_H_
static const uint8_t LOGE_I2C = false; // log errors from I2C
static const uint8_t LOGI_I2C = false; // log info from I2C
static char* const I2C_TAG = "I2C";


static const size_t I2C_TX_BUF_DISABLE = 0;
static const size_t I2C_RX_BUF_DISABLE = 0;
static const int INTR_FLAGS = 0;

static const uint8_t I2C_PORT = I2C_NUM_0; // esp32 has 2 ports NUM_0/NUM_1
static const uint8_t LCD_ADDRESS = 0x27; // lcd i2c adress
static const uint8_t MCP1_ADDRESS = 0x26; // MCP23017 1 i2c address
static const uint8_t MCP2_ADDRESS = 0x25; // MCP23017 2 i2c address
static const uint8_t WRITE = I2C_MASTER_WRITE; // write bit
static const uint8_t READ = I2C_MASTER_READ ; // read bit
static const uint8_t ACK = 0x1; // ACK bit        


static const uint8_t NO_REG = false; // additionally send register address 
static const uint8_t WITH_REG = true; // do ot send reigister

esp_err_t I2CInit(void); 
esp_err_t I2CWrite(uint8_t address, uint8_t size, uint8_t *data);
esp_err_t I2CRead(uint8_t address, uint8_t reg, uint8_t size, uint8_t *data);
#endif