#include <stdint.h>
#include <esp_err.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/spi_master.h"
#include "amp_parameters.h"

#ifndef _spi_parameters_H_
#define _spi_parameters_H_
static const spi_host_device_t SENDER_HOST=HSPI_HOST;
static const uint8_t LOGE_SPI = false;
static const uint8_t LOGI_SPI = false;
static char* const SPI_TAG = "SPI";

static const gpio_num_t GPIO_MOSI = GPIO_NUM_13;
static const gpio_num_t GPIO_MISO= GPIO_NUM_12;
static const gpio_num_t GPIO_CLK = GPIO_NUM_14;
static const gpio_num_t GPIO_CS= GPIO_NUM_15;

esp_err_t SpiInit(void); // initialize spi
esp_err_t SpiInit2(void);
esp_err_t SpiBusRemoveNjw1194(void);
esp_err_t SpiBusAddNjw1194(void);
esp_err_t SpiWrite(size_t size, uint8_t *data);
#endif