#include "spi.h"

static const char *TAG = "SPI";
#define SENDER_HOST HSPI_HOST
spi_device_handle_t spi_handle;

esp_err_t spi_init(void)
{
   spi_bus_config_t bus_config={
      .miso_io_num=-1,
      .mosi_io_num=GPIO_MOSI,
      .sclk_io_num=GPIO_CLK,
      .quadwp_io_num=-1,
      .quadhd_io_num=-1,
      .max_transfer_sz=10
   };
   esp_err_t ret= spi_bus_initialize(SENDER_HOST, &bus_config, SPI_DMA_CH_AUTO);
   if (ret!=ESP_OK) {
		ESP_LOGE(TAG, "Unable to init spi.");
		ESP_LOGE(TAG, "Description: %s", esp_err_to_name(ret));
      return ret;
   }
   return ret;
}


esp_err_t spi_init2(void){
   spi_bus_config_t bus_config={
        .miso_io_num=-1,
        .mosi_io_num=GPIO_MOSI,
        .sclk_io_num=GPIO_CLK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=10
    };
    
    spi_device_interface_config_t njw1194_config={
      .command_bits=0,
        .address_bits=0,
        .dummy_bits=0,
        .clock_speed_hz=400000,
        .duty_cycle_pos=128,        //50% duty cycle
        .mode=2,
        .spics_io_num=GPIO_CS,
        .cs_ena_pretrans=1,        //Keep the CS low 3 cycles after transaction, to stop slave from missing the last bit when CS has less propagation delay than CLK
        .cs_ena_posttrans=1,        //Keep the CS low 3 cycles after transaction, to stop slave from missing the last bit when CS has less propagation delay than CLK
        .queue_size=10,  //Specify pre-transfer callback to handle D/C line
    };
    
   esp_err_t ret= spi_bus_initialize(SENDER_HOST, &bus_config, SPI_DMA_CH_AUTO);
   if (ret!=ESP_OK) {
		ESP_LOGE(TAG, "Unable to init spi.");
		ESP_LOGE(TAG, "Description: %s", esp_err_to_name(ret));
      return ret;
   }
   ret = spi_bus_add_device(SENDER_HOST, &njw1194_config, &spi_handle);
   if (ret!=ESP_OK) {
		ESP_LOGE(TAG, "Unable to add njw1194.");
		ESP_LOGE(TAG, "Description: %s", esp_err_to_name(ret));
   }
   else
      ESP_LOGI(TAG, "SPI initialized.");
   return ret;
}

esp_err_t spi_bus_add_njw1194 (void){ 
   spi_device_interface_config_t njw1194_config={
      .command_bits=0,
        .address_bits=0,
        .dummy_bits=0,
        .clock_speed_hz=400000,
        .duty_cycle_pos=128,        
        .mode=2,
        .spics_io_num=GPIO_CS,
        .cs_ena_pretrans=1,        
        .cs_ena_posttrans=1,       
        .queue_size=10,
   };
   esp_err_t ret = spi_bus_add_device(SENDER_HOST, &njw1194_config, &spi_handle);
   if (ret!=ESP_OK) {
		ESP_LOGE(TAG, "Unable to add njw1194 to spi bus.");
		ESP_LOGE(TAG, "Description: %s", esp_err_to_name(ret));
   }
   else
      ESP_LOGI(TAG, "njw1194 added to spi bus.");
   return ret;
}

esp_err_t spi_bus_remove_njw1194 (void){ 
   esp_err_t ret = spi_bus_remove_device(spi_handle);
   if (ret!=ESP_OK) {
		ESP_LOGE(TAG, "Unable to remove njw1194 from spi bus.");
		ESP_LOGE(TAG, "Description: %s", esp_err_to_name(ret));
   }
   else
      ESP_LOGI(TAG, "njw1194 removed from spi bus.");
   return ret;
}

esp_err_t spi_write(size_t size, uint8_t* data){
   spi_transaction_t t={0};
      t.addr=0;
      t.length=size;
      t.rxlength=0;
      t.tx_buffer=data; 
   esp_err_t ret=spi_device_transmit(spi_handle, &t);
      if (ret == ESP_FAIL) {
      if(LOGE_SPI) ESP_LOGE(TAG,"Unable to write to spi.");
      if(LOGE_SPI) ESP_LOGE(TAG,"description: %s", esp_err_to_name(ret));
   }
   else
      if(LOGI_SPI) ESP_LOGI(TAG, "SPI write OK.");

   return ret;
}
