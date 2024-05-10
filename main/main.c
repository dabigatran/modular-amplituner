#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/idf_additions.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "driver/i2c.h"
#include "amp_parameters.h" 
#include "lcd.h"
#include "i2c.h"
#include "mcp23017.h"
#include "source.h"
#include "encoder.h"
#include "njw1194.h"
#include "spi.h"
#include "remote.h"

static const char *TAG = "AmpliTuner";
static SemaphoreHandle_t encoder_interrupt_semaphore=NULL;
static SemaphoreHandle_t standby_interrupt_semaphore=NULL;
static TaskHandle_t encoder_get_task=NULL;
static TaskHandle_t encoder_set_task=NULL;
static TaskHandle_t encoder_interrupt_clear_task=NULL;
static TaskHandle_t remote_get_task=NULL;
static TaskHandle_t remote_set_task=NULL;
static QueueHandle_t encoder_queue=NULL;
static QueueHandle_t remote_queue=NULL;

bool standby_mode= ON;
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

static void initialize(void);
static void display_welcome_message(void);
static void display_interface(void);
static void encoder_isr_handler_add(void);
static void encoder_isr_handler_remove(void);
static void gpio_init(void);
static void setup_audio_defaults(void);
static void encoder_interrupt_handler(void *args);
static void blink_led(uint8_t state);
static void encoder_get_loop();
static void encoder_set_loop();
static void encoder_interrupt_clear_loop();
static void remote_get_loop();
static void remote_set_loop();
static void set_change(int8_t last_change);
static void audio_power_toggle(void);
static void digital_power_toggle(void);
static void amp_audio_control(uint8_t state);
static void standby(uint8_t state);
static void setup_encoder_tasks(void);
static void delete_encoder_tasks(void);
static void setup_remote_tasks(void);
static void standby_isr_handler_add(void);
static void isr_init();


void app_main(void){
  initialize();
    while(1){
      if (xSemaphoreTake(standby_interrupt_semaphore, 3000) == pdPASS){
        uint8_t counter = 0;
        while(gpio_get_level(STANDBY_BUTTON_GPIO)==0){
          counter++;
          vTaskDelay(100 / portTICK_PERIOD_MS);
          if(counter>4){
            standby_mode = !standby_mode;
            standby(standby_mode);
            break;
          }
        }
      }
  }
}

void isr_init(){
  esp_err_t ret= gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
  if(ret!=ESP_OK)
    if(LOGE_ENC) ESP_LOGE(TAG, "ISR install error.");
}

static void initialize(void){
  gpio_init();
  amp_audio_control(ON);
  audio_power_toggle();    
  digital_power_toggle();
  
  isr_init();
  
  spi_init();
  i2c_init();
  
  standby_isr_handler_add();
  
  setup_audio_defaults();
  setup_remote_tasks();
  blink_led(ON);
  ESP_LOGI(TAG,"Init finished. standby: %d", standby_mode);
  standby_interrupt_semaphore = xSemaphoreCreateBinary();
}

static void setup_encoder_tasks(void){
  encoder_interrupt_semaphore = xSemaphoreCreateBinary();
  encoder_queue=xQueueCreate(ENC_QUEUE_LENGTH,ENCODER_DATA);
  xTaskCreatePinnedToCore(encoder_get_loop, "encoder_get_loop", 2048, NULL, 6, &encoder_get_task, 0); 
  xTaskCreatePinnedToCore(encoder_set_loop, "encoder_set_loop", 4096, NULL, 5, &encoder_set_task, 1); 
  xTaskCreatePinnedToCore(encoder_interrupt_clear_loop, "encoder_interrupt_clear_loop", 2048, NULL, 1, &encoder_interrupt_clear_task, 1); 
}

static void setup_remote_tasks(void){
  xTaskCreatePinnedToCore(remote_get_loop, "remote_get_loop", 4096, NULL, 4, &remote_get_task, 0); 
  xTaskCreatePinnedToCore(remote_set_loop, "remote_set_loop", 2048, NULL, 3, &remote_set_task, 1); 
}

static void delete_encoder_tasks(void){
  encoder_interrupt_semaphore = NULL;
  encoder_queue=NULL;
  vTaskDelete(encoder_get_task);
  vTaskDelete(encoder_set_task);
  vTaskDelete(encoder_interrupt_clear_task);
}

static void IRAM_ATTR encoder_get_loop(){
  while (1){
    if (xSemaphoreTake(encoder_interrupt_semaphore, PORT_DELAY) == pdPASS){
      uint8_t interrupt_data[2]={0};
      mcp_read(MCP1_ADDRESS, INTFB, &interrupt_data[0]);
      mcp_read(MCP1_ADDRESS, INTCAPB, &interrupt_data[1]);
      xQueueSend(encoder_queue,&interrupt_data, PORT_DELAY);
    }
  }
}

static void IRAM_ATTR encoder_set_loop(){
  while (1){
    uint8_t interrupt_data[2]={0};
    xQueueReceive(encoder_queue,&interrupt_data, PORT_DELAY);
    int8_t last_change=create_snapshot(tuner_state, interrupt_data);
    set_change(last_change);
  }
}

static void IRAM_ATTR encoder_interrupt_clear_loop(){
  size_t count=0;
  while (1){
    if (!gpio_get_level(INTR_PIN)){
      count++;
      if (count==3){
        uint8_t interrupt_data[2]={0};
        mcp_read(MCP1_ADDRESS, INTFB, &interrupt_data[0]);
        mcp_read(MCP1_ADDRESS, INTCAPB, &interrupt_data[1]);
        xQueueSend(encoder_queue,&interrupt_data, PORT_DELAY);
        count=0;
      }
      vTaskDelay(10 / portTICK_PERIOD_MS);
      continue;
    }
    else{
      count=0;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

static void IRAM_ATTR remote_get_loop(){
  remote_queue=xQueueCreate(RMT_QUEUE_LENGTH,REMOTE_DATA);
  ESP_LOGI(TAG, "create RMT RX channel");
    rmt_rx_channel_config_t rx_channel_cfg = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = IR_RESOLUTION_HZ,
        .mem_block_symbols = 64, 
        .gpio_num = IR_RX_GPIO_NUM,
    };
    rmt_channel_handle_t rx_channel = NULL;
    ESP_ERROR_CHECK(rmt_new_rx_channel(&rx_channel_cfg, &rx_channel));

    ESP_LOGI(TAG, "register RX done callback");
    QueueHandle_t receive_queue = xQueueCreate(1, sizeof(rmt_rx_done_event_data_t));
    assert(receive_queue);
    rmt_rx_event_callbacks_t cbs = {
        .on_recv_done = rmt_rx_done_callback,
    };
    ESP_ERROR_CHECK(rmt_rx_register_event_callbacks(rx_channel, &cbs, receive_queue));

    rmt_receive_config_t receive_config = {
        .signal_range_min_ns = 1250,     
        .signal_range_max_ns = 12000000, 
    };

    ESP_ERROR_CHECK(rmt_enable(rx_channel));
    rmt_symbol_word_t raw_symbols[64]; 
    rmt_rx_done_event_data_t rx_data;
    ESP_ERROR_CHECK(rmt_receive(rx_channel, raw_symbols, sizeof(raw_symbols), &receive_config));
    while (1) {
        if (xQueueReceive(receive_queue, &rx_data, pdMS_TO_TICKS(1000)) == pdPASS) {
            parse_frame(rx_data.received_symbols, rx_data.num_symbols, remote_queue);
            ESP_ERROR_CHECK(rmt_receive(rx_channel, raw_symbols, sizeof(raw_symbols), &receive_config));
        } 
    }
}

static void IRAM_ATTR remote_set_loop(){
  while (1){
    int8_t last_change=remote_parse (tuner_state, remote_queue);
    set_change(last_change);
  }
}

static void encoder_interrupt_handler(void *args){
  BaseType_t  xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(encoder_interrupt_semaphore, &xHigherPriorityTaskWoken);
}

static void standby_interrupt_handler(void *args){
  BaseType_t  xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(standby_interrupt_semaphore, &xHigherPriorityTaskWoken);
}

static void encoder_isr_handler_add(void){
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_INTR_NEGEDGE;
  io_conf.pin_bit_mask = (1<<INTR_PIN);
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
  gpio_config(&io_conf);  
  esp_err_t ret=gpio_isr_handler_add(INTR_PIN, encoder_interrupt_handler, NULL);
  if(ret!=ESP_OK){
    if(LOGE_ENC) ESP_LOGE(TAG, "Encoder handler add error.");
  }
  else{
    if(LOGI_ENC) ESP_LOGI(TAG, "Encoder handler add OK.");
  }
}

static void encoder_isr_handler_remove(void){
  esp_err_t ret=gpio_isr_handler_remove(INTR_PIN);
  if(ret!=ESP_OK){
    if(LOGE_ENC) ESP_LOGE(TAG, "Encoder handler remove error.");
  }
  else{
    if(LOGI_ENC) ESP_LOGI(TAG, "Encoder handler remove OK.");
  }
}

static void standby_isr_handler_add(void){
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_INTR_NEGEDGE;
  io_conf.pin_bit_mask = (1ULL<<STANDBY_BUTTON_GPIO);
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
  gpio_config(&io_conf);
  esp_err_t  ret = gpio_isr_handler_add(STANDBY_BUTTON_GPIO, standby_interrupt_handler, NULL);
  if(ret!=ESP_OK){
    if(LOGE_ENC) ESP_LOGE(TAG, "Standby handler add error.");
  }
  else{
    if(LOGI_ENC) ESP_LOGI(TAG, "Standby handler add OK.");
  }
}

static void display_welcome_message(void){
  lcd_text(0,2,"   Tuner");
  lcd_text(1,1,"version 0.5");
}

static void display_interface(void){
  lcd_clear();
  set_change(SOURCE);
  lcd_text(0,0, option_alias[VOLUME]);
  set_change(VOLUME);
}

static void setup_audio_defaults(void){
  tuner_state[VOLUME][ACT_VAL]=17;
  tuner_state[SOURCE][ACT_VAL]=5;
  tuner_state[BALANCE][ACT_VAL]=0;
  tuner_state[TREBLE][ACT_VAL]=0;
  tuner_state[BASS][ACT_VAL]=0;
  tuner_state[TONE_SW][ACT_VAL]=0;
}

static void blink_led(uint8_t state){
  gpio_set_level(LED_GPIO, state);
}

static void gpio_init(void){
  gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
  gpio_set_direction(AMP_AUDIO_CONTROL_GPIO, GPIO_MODE_OUTPUT);
  gpio_set_direction(AUDIO_POWER_CONTROL_GPIO, GPIO_MODE_OUTPUT);
  gpio_set_direction(DIGITAL_POWER_CONTROL_GPIO, GPIO_MODE_OUTPUT);
}

static void audio_power_toggle(void){
  //bistable module
  vTaskDelay(60 / portTICK_PERIOD_MS);
  gpio_set_level(AUDIO_POWER_CONTROL_GPIO, ON);
  vTaskDelay(60 / portTICK_PERIOD_MS);
  gpio_set_level(AUDIO_POWER_CONTROL_GPIO, OFF);
}

static void digital_power_toggle(void){
  //bistable module
  vTaskDelay(60 / portTICK_PERIOD_MS);
  gpio_set_level(DIGITAL_POWER_CONTROL_GPIO, ON);
  vTaskDelay(60 / portTICK_PERIOD_MS);
  gpio_set_level(DIGITAL_POWER_CONTROL_GPIO, OFF);
}

static void amp_audio_control(uint8_t state){
  gpio_set_level(AMP_AUDIO_CONTROL_GPIO, state);
}

static void IRAM_ATTR set_change(int8_t last_change){
  char buffer [17];
  switch (last_change){
    case SOURCE:
      set_volume(tuner_state[VOLUME][MIN_VAL], tuner_state[BALANCE][ACT_VAL]);
      set_source(tuner_state);
        sprintf(buffer, "%s", source_alias[tuner_state[SOURCE][ACT_VAL]]);
      lcd_clear_line(1);
      lcd_text(1, 0, buffer);
      set_volume(tuner_state[VOLUME][ACT_VAL], tuner_state[BALANCE][ACT_VAL]);
      break;
    case VOLUME:
      set_volume(tuner_state[VOLUME][ACT_VAL], tuner_state[BALANCE][ACT_VAL]);
      sprintf(buffer, "%2d", tuner_state[VOLUME][ACT_VAL]);
      lcd_text(0, 7, buffer);
      break;
    case STANDBY:
      standby_mode = !standby_mode;
      standby(standby_mode);
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

static void standby(uint8_t state){
  if(state==ON){
    ESP_LOGI(TAG,"Going into standby mode.");
    amp_audio_control(ON);
    lcd_clear();
    lcd_text(0,0,"   going into");
    lcd_text(1,0,"  Standby mode");
    set_volume(tuner_state[VOLUME][MIN_VAL], tuner_state[BALANCE][ACT_VAL]);
    set_analog_input(INPUT_MUTE);
    encoder_isr_handler_remove();
    spi_bus_remove_njw1194();
    delete_encoder_tasks();    
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    audio_power_toggle();    
    digital_power_toggle();    
    ESP_LOGI(TAG,"Stanby mode ON");
    lcd_clear();
  }
  else if (state==OFF){
    ESP_LOGI(TAG,"Waking up from Stanby mode");
    amp_audio_control(ON);
    digital_power_toggle();
    audio_power_toggle();    
    mcp_init(MCP1_ADDRESS);
    lcd_init();
    display_welcome_message();
    setup_encoder_tasks();
    encoder_isr_handler_add();
    spi_bus_add_njw1194();
    sources_init();
    set_tone(tuner_state[TONE_SW][ACT_VAL], TREBLE_CTRL, tuner_state[TREBLE][ACT_VAL]);
    set_tone(tuner_state[TONE_SW][ACT_VAL],BASS_CTRL, tuner_state[BASS][ACT_VAL]);
    set_source(tuner_state);
    set_volume(tuner_state[VOLUME][ACT_VAL], tuner_state[BALANCE][ACT_VAL]);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    display_interface();
    amp_audio_control(OFF);
    ESP_LOGI(TAG,"Stanby mode off");
  }

}



