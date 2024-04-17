#include <stdint.h>
#include <esp_err.h>
#include "esp_log.h"
#include "driver/i2c.h"

#define I2C_PORT        I2C_NUM_0 //esp32 has 2 ports NUM_0/NUM_1
#define LCD_ADDRESS     (uint8_t)0x27 //lcd i2c adress
#define MCP1_ADDRESS    (uint8_t)0x26 //MCP23017 1 i2c address
#define MCP2_ADDRESS    (uint8_t)0x25 //MCP23017 2 i2c address
#define WRITE           I2C_MASTER_WRITE // write bit
#define READ            I2C_MASTER_READ //read bit 
#define ACK             (uint8_t)0x1 //ACK bit
#define NO_REG          (uint8_t)0x0
#define WITH_REG        (uint8_t)0x1
#define LOGE_I2C        (uint8_t)0x1
#define LOGI_I2C        (uint8_t)0x0

esp_err_t i2c_init (void);   // initialize i2c
esp_err_t i2c_write(uint8_t address, uint8_t size, uint8_t* data);
esp_err_t i2c_read(uint8_t address, uint8_t reg, uint8_t size, uint8_t *data);