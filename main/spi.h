#include <stdint.h>
#include <esp_err.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/spi_master.h"
#include "amp_parameters.h"

#define GPIO_MOSI GPIO_NUM_13
#define GPIO_MISO GPIO_NUM_12
#define GPIO_CLK GPIO_NUM_14
#define GPIO_CS GPIO_NUM_15
#define LOGE_SPI (uint8_t)0x1
#define LOGI_SPI (uint8_t)0x00

esp_err_t SpiInit(void); // initialize spi
esp_err_t SpiInit2(void);
esp_err_t SpiBusRemoveNjw1194(void);
esp_err_t SpiBusAddNjw1194(void);
esp_err_t SpiWrite(size_t size, uint8_t *data);