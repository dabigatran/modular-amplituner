#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/idf_additions.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "driver/i2c.h"
#include "lcd.h"
#include "i2c.h"
#include "mcp23017.h"
#include "source.h"
#include "encoder.h"
#include "ampstate.h" 
#include "njw1194.h"
#include "spi.h"
#include "remote.h"

static SemaphoreHandle_t interruptSemaphore=NULL;
TaskHandle_t task0=NULL;
TaskHandle_t task1=NULL;
TaskHandle_t task2=NULL;
TaskHandle_t task3=NULL;
TaskHandle_t task4=NULL;
static QueueHandle_t encoder_queue=NULL;


static const char *TAG = "AmpliTuner";
static uint8_t s_led_state = 0;
int8_t tuner_state[OPTION_NO][VAR_NO]={
    {0, 0, VOL_MIN, VOL_MAX, OVFL_NO}, 
    {SEL_MIN, 0, SEL_MIN, SEL_MAX, OVFL_YES}, 
    {0, 0, SRC_MIN, SRC_MAX, OVFL_YES},
    {0, 0, BAL_MIN, BAL_MAX, OVFL_NO}, 
    {0, 0, TRE_MIN, TRE_MAX, OVFL_NO},
    {0, 0, BAS_MIN, BAS_MAX, OVFL_NO}, 
    {0, 0, TON_MIN, TON_MAX, OVFL_NO}
    };

static char source_alias [MAX_SOURCES][MAX_SRC_ALIAS_LENGTH]  = {
            {"Analog"}, {"HDMI1"}, {"HDMI2"}, {"HDMI3"}, {"HDMI4"},{"\0"},{"Spdif"},
            {"Bluetooth"}, {"\0"}
            }; 

static char option_alias [OPTION_NO][MAX_OPT_ALIAS_LENGTH]  = {
            {"Volume"}, {"Selected"}, {"Source"}, {"Balance"}, {"Treble"},{"Bass"}, {"Tone switch"}
            };

static void encoder_interrupt_handler(void *args){
  BaseType_t  xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(interruptSemaphore, &xHigherPriorityTaskWoken);
}

void ISR_init(){
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_INTR_NEGEDGE;
  io_conf.pin_bit_mask = (1<<INTR_PIN);
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
  gpio_config(&io_conf);
  esp_err_t ret= gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
  if(ret!=ESP_OK)
    if(LOGE_ENC) ESP_LOGE(TAG, "ISR install error.");
  ret = gpio_isr_handler_add(INTR_PIN, encoder_interrupt_handler, NULL);
  if(ret!=ESP_OK){
    if(LOGE_ENC) ESP_LOGE(TAG, "ISR handler add error error.");
  }
  else{
    if(LOGI_ENC) ESP_LOGI(TAG, "ISR handler init OK.");
  }
}

static void blink_led(void){
  gpio_set_level(BLINK_GPIO, s_led_state);
}

void welcome_message(){
  lcd_text(0,2,"   Tuner");
  lcd_text(1,1,"version 0.1");
}

void IRAM_ATTR clearing_loop(){
  size_t count=0;
  while (1){
    if (!gpio_get_level(INTR_PIN)){
      count++;
      if (count==4){
        uint8_t interrupt_data[2]={0};
        mcp_read(MCP1_ADDRESS, INTFB, &interrupt_data[0]);
        mcp_read(MCP1_ADDRESS, INTCAPB, &interrupt_data[1]);
        xQueueSend(encoder_queue,&interrupt_data, portMAX_DELAY);
        count=0;
      }
      vTaskDelay(40 / portTICK_PERIOD_MS);
      continue;
    }
    else{
      count=0;
    }
    vTaskDelay(400 / portTICK_PERIOD_MS);
  }
}

void IRAM_ATTR getting_loop(){
  while (1){
    if (xSemaphoreTake(interruptSemaphore, portMAX_DELAY) == pdPASS){
      //interrupt_data[0]=last_interrupt; interrupt_data[1]=captured_values
      uint8_t interrupt_data[2]={0};
      uint8_t dummy;  
      mcp_read(MCP1_ADDRESS, INTFB, &interrupt_data[0]);
      mcp_read(MCP1_ADDRESS, INTCAPB, &interrupt_data[1]);
      xQueueSend(encoder_queue,&interrupt_data, portMAX_DELAY);
      mcp_read(MCP1_ADDRESS, INTCAPB, &dummy);
    }
  }
}

void IRAM_ATTR set_change(int8_t last_change){
  char buffer [17];
  switch (last_change){
    case SOURCE:
      set_source(tuner_state);
        sprintf(buffer, "%s", source_alias[tuner_state[SOURCE][ACT_VAL]]);
      lcd_clear_line(1);
      lcd_text(1, 0, buffer);
      break;
    case VOLUME:
      set_volume(tuner_state[VOLUME][ACT_VAL], tuner_state[BALANCE][ACT_VAL]);
      sprintf(buffer, "%2d", tuner_state[VOLUME][ACT_VAL]);
      lcd_text(0, 7, buffer);
      break;
    case BALANCE:
      set_volume(tuner_state[VOLUME][ACT_VAL], tuner_state[BALANCE][ACT_VAL]);
      sprintf(buffer, "%3d", tuner_state[BALANCE][ACT_VAL]);
      lcd_text(1, 8, buffer);
      break;
    case TREBLE:
      set_tone(tuner_state[TONE_SW][ACT_VAL],TREBLE_CTRL, tuner_state[TREBLE][ACT_VAL]);
      sprintf(buffer, "%3d", tuner_state[TREBLE][ACT_VAL]);
      lcd_text(1, 7, buffer);
      break;
    case BASS:
      set_tone(tuner_state[TONE_SW][ACT_VAL],BASS_CTRL, tuner_state[BASS][ACT_VAL]);
      sprintf(buffer, "%3d", tuner_state[BASS][ACT_VAL]);
      lcd_text(1, 5, buffer);
      break;
    case TONE_SW:
      set_tone(tuner_state[TONE_SW][ACT_VAL],TREBLE_CTRL, tuner_state[TREBLE][ACT_VAL]);
      if (tuner_state[TONE_SW][ACT_VAL]==1)
        sprintf(buffer, "%s", "ON ");
      else
        sprintf(buffer, "%s", "OFF");
      lcd_text(1, 12, buffer);
      break;
    case SELECTED:
      lcd_clear_line(1);
      if(tuner_state[SELECTED][ACT_VAL]==SOURCE)
        sprintf(buffer, "%s", source_alias[tuner_state[SOURCE][ACT_VAL]]);
      else if (tuner_state[SELECTED][ACT_VAL]==TONE_SW && tuner_state[TONE_SW][ACT_VAL]==1)
        sprintf(buffer, "%s %s", option_alias[tuner_state[SELECTED][ACT_VAL]], "ON");
      else if (tuner_state[SELECTED][ACT_VAL]==TONE_SW && tuner_state[TONE_SW][ACT_VAL]==0)
        sprintf(buffer, "%s %s", option_alias[tuner_state[SELECTED][ACT_VAL]], "OFF");
      else
        sprintf(buffer, "%s %3d", option_alias[tuner_state[SELECTED][ACT_VAL]], tuner_state[tuner_state[SELECTED][ACT_VAL]][ACT_VAL]);
      lcd_text(1, 0, buffer );
    }
}

void IRAM_ATTR setting_loop(){
  while (1){
    uint8_t interrupt_data[2]={0};
    xQueueReceive(encoder_queue,&interrupt_data, portMAX_DELAY);
    int8_t last_change=create_snapshot(tuner_state, interrupt_data);
    set_change(last_change);
  }
}

void IRAM_ATTR remote_parse_loop(){
  while (1){
    int8_t last_change=remote_parse (tuner_state);
    set_change(last_change);
  }
}

void task_setup(void){
  interruptSemaphore = xSemaphoreCreateBinary();
  //data recieved from encoder through MCP23017 
  //byte[0]=last_interrupt; byte[1]=captured_values;
  encoder_queue=xQueueCreate(40,ENCODER_DATA);
  xTaskCreatePinnedToCore(getting_loop, "getting_loop", 2048, NULL, 4, &task0, 0); 
  xTaskCreatePinnedToCore(setting_loop, "setting_loop", 4096, NULL, 2, &task1, 1); 
  xTaskCreatePinnedToCore(clearing_loop, "clearing_loop", 2048, NULL, 1, &task2, 1); 
  xTaskCreatePinnedToCore(remote_ctrl_loop, "remote_ctrl_loop", 4096, NULL, 3, &task3, 0); 
  xTaskCreatePinnedToCore(remote_parse_loop, "remote_parse_loop", 4096, NULL, 1, &task4, 0); 
}


void init_audio(void){
  tuner_state[VOLUME][ACT_VAL]=15;
  tuner_state[SOURCE][ACT_VAL]=5;
  tuner_state[BALANCE][ACT_VAL]=0;
  tuner_state[TREBLE][ACT_VAL]=0;
  tuner_state[BASS][ACT_VAL]=0;
  tuner_state[TONE_SW][ACT_VAL]=0;
  set_tone(tuner_state[TONE_SW][ACT_VAL], TREBLE_CTRL, tuner_state[TREBLE][ACT_VAL]);
  set_tone(tuner_state[TONE_SW][ACT_VAL],BASS_CTRL, tuner_state[BASS][ACT_VAL]);
}

static void configure(void){
  ISR_init();
  gpio_reset_pin(BLINK_GPIO);
  gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
  i2c_init();
  lcd_init();
  spi_init();
  welcome_message();
  mcp_init(MCP1_ADDRESS);
  sources_init();
  s_led_state = !s_led_state;
  blink_led();
  init_audio();
  task_setup();
  ESP_LOGI(TAG,"Init finished");
}

void display_menu(){
  lcd_clear();
  set_change(SOURCE);
  lcd_text(0,0, option_alias[VOLUME]);
  set_change(VOLUME);
}

void app_main(void){
  configure();
  display_menu();
  
  while(1){
    vTaskDelete(NULL);
  }
}

