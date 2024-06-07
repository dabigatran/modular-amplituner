#include "spi.h"
static spi_device_handle_t spiHandle;

esp_err_t SpiInit(void)
{
   spi_bus_config_t busConfig = {
       .miso_io_num = -1,
       .mosi_io_num = GPIO_MOSI,
       .sclk_io_num = GPIO_CLK,
       .quadwp_io_num = -1,
       .quadhd_io_num = -1,
       .max_transfer_sz = 10};
   esp_err_t ret = spi_bus_initialize(SENDER_HOST, &busConfig, SPI_DMA_CH_AUTO);
   if (LOGE_SPI && ret != ESP_OK)
   {
      ESP_LOGE(SPI_TAG, "Unable to init spi.");
   }
   return ret;
}

esp_err_t SpiBusAddNjw1194(void)
{
   spi_device_interface_config_t njw1194Config = {
       .command_bits = 0,
       .address_bits = 0,
       .dummy_bits = 0,
       .clock_speed_hz = 400000,
       .duty_cycle_pos = 128,
       .mode = 2,
       .spics_io_num = GPIO_CS,
       .cs_ena_pretrans = 1,
       .cs_ena_posttrans = 1,
       .queue_size = 10,
   };
   esp_err_t ret = spi_bus_add_device(SENDER_HOST, &njw1194Config, &spiHandle);
   if (LOGE_SPI && ret != ESP_OK)
   {
      ESP_LOGE(SPI_TAG, "Unable to add njw1194 to spi bus.");
   }
   else if (LOGI_SPI)
   {
      ESP_LOGI(SPI_TAG, "njw1194 added to spi bus.");
   }
   return ret;
}

esp_err_t SpiBusRemoveNjw1194(void)
{
   esp_err_t ret = spi_bus_remove_device(spiHandle);
   if (LOGE_SPI && ret != ESP_OK)
   {
      ESP_LOGE(SPI_TAG, "Unable to remove njw1194 from spi bus.");
   }
   else if (LOGI_SPI)
   {
      ESP_LOGI(SPI_TAG, "njw1194 removed from spi bus.");
   }
   return ret;
}

esp_err_t SpiWrite(size_t size, uint8_t *data)
{
   spi_transaction_t t = {0};
   t.addr = 0;
   t.length = size;
   t.rxlength = 0;
   t.tx_buffer = data;
   esp_err_t ret = spi_device_transmit(spiHandle, &t);
   if (LOGE_SPI && ret == ESP_FAIL)
   {
         ESP_LOGE(SPI_TAG, "Unable to write to spi.");
   }
   else if (LOGI_SPI)
   {
      ESP_LOGI(SPI_TAG, "SPI write OK.");
   }
   return ret;
}
