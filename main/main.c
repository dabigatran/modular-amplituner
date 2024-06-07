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


static SemaphoreHandle_t encoderInterruptSemaphore = NULL;
static SemaphoreHandle_t standbyInterruptSemaphore = NULL;
static TaskHandle_t encoderGetTask = NULL;
static TaskHandle_t encoderSetTask = NULL;
static TaskHandle_t encoderInterruptClearTask = NULL;
static TaskHandle_t remoteGetTask = NULL;
static TaskHandle_t remoteSetTask = NULL;
static QueueHandle_t encoderQueue = NULL;
static QueueHandle_t remoteQueue = NULL;
static bool standbyMode = ON;
static int8_t tunerState[OPTION_NO][VAR_NO] = {
    {0, 0, VOL_MIN, VOL_MAX, OVFL_NO},
    {SEL_MIN, 0, SEL_MIN, SEL_MAX, OVFL_YES},
    {0, 0, SRC_MIN, SRC_MAX, OVFL_YES},
    {0, 0, BAL_MIN, BAL_MAX, OVFL_NO},
    {0, 0, TRE_MIN, TRE_MAX, OVFL_NO},
    {0, 0, BAS_MIN, BAS_MAX, OVFL_NO},
    {0, 0, TON_MIN, TON_MAX, OVFL_NO}};
static char sourceAlias[MAX_SOURCES][MAX_SRC_ALIAS_LENGTH] = {
    {"Analog"}, {"HDMI1"}, {"HDMI2"}, {"HDMI3"}, {"HDMI4"}, {"\0"}, {"Spdif"}, {"Bluetooth"}, {"\0"}};
static char optionAlias[OPTION_NO][MAX_OPT_ALIAS_LENGTH] = {
    {"Volume"}, {"Selected"}, {"Source"}, {"Balance"}, {"Treble"}, {"Bass"}, {"Tone switch"}};

static void Initialize(void);
static void DisplayWelcomeMessage(void);
static void DisplayInterface(void);
static void EncoderIsrHandlerAdd(void);
static void EncoderIsrHandlerRemove(void);
static void GpioInit(void);
static void SetupAudioDefaults(void);
static void EncoderInterruptHandler(void *args);
static void BlinkLed(uint8_t state);
static void EncoderGetLoop();
static void EncoderSetLoop();
static void EncoderInterruptClearLoop();
static void RemoteGetLoop();
static void RemoteSetLoop();
static void SetChange(int8_t lastChange);
static void AudioPowerToggle(void);
static void DigitalPowerToggle(void);
static void AmpAudioControl(uint8_t state);
static void Standby(uint8_t state);
static void SetupEncoderTasks(void);
static void DeleteEncoderTasks(void);
static void SetupRemoteTasks(void);
static void StandbyIsrHandlerAdd(void);
static void IsrInit();
static void AmpPowerControl(uint8_t state);

void app_main(void)
{
  Initialize();
  while (1)
  {
    if (xSemaphoreTake(standbyInterruptSemaphore, STANDBY_SEMAPHORE_BLOCK_TIME == pdPASS))
    {
      uint8_t counter = 0;
      while (gpio_get_level(STANDBY_BUTTON_GPIO) == 0)
      {
        counter++;
        vTaskDelay(STANDBY_BUTTON_DELAY/STANDBY_BUTTON_COUNTER_MAX/portTICK_PERIOD_MS);
        if (counter >= STANDBY_BUTTON_COUNTER_MAX)
        {
          standbyMode = !standbyMode;
          Standby(standbyMode);
          break;
        }
      }
    }
  }
}

static void IsrInit()
{
  esp_err_t ret = gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
  if (LOGE_MAIN && ret != ESP_OK)
  {
    ESP_LOGE(AMP_TAG, "ISR install error.");
  }
}

static void Initialize(void)
{
  GpioInit();
  AmpAudioControl(ON);
  AudioPowerToggle();
  DigitalPowerToggle();
  IsrInit();
  SpiInit();
  I2CInit();
  StandbyIsrHandlerAdd();
  SetupAudioDefaults();
  SetupRemoteTasks();
  BlinkLed(ON);
  if(LOGI_MAIN){
    ESP_LOGI(AMP_TAG, "Init finished. standby: %d", standbyMode);
  }
  standbyInterruptSemaphore = xSemaphoreCreateBinary();
}

static void SetupEncoderTasks(void)
{
  encoderInterruptSemaphore = xSemaphoreCreateBinary();
  encoderQueue = xQueueCreate(ENC_QUEUE_LENGTH, ENCODER_DATA);
  xTaskCreatePinnedToCore(EncoderGetLoop, "EncoderGetLoop", DEFAULT_STACK_DEPTH, NULL, PRIORITY_6, &encoderGetTask, CORE0);
  xTaskCreatePinnedToCore(EncoderSetLoop, "EncoderSetLoop", DEFAULT_STACK_DEPTH, NULL, PRIORITY_5, &encoderSetTask, CORE1);
  xTaskCreatePinnedToCore(EncoderInterruptClearLoop, "EncoderInterruptClearLoop", DEFAULT_STACK_DEPTH, NULL, PRIORITY_1, &encoderInterruptClearTask, CORE1);
}

static void SetupRemoteTasks(void)
{
  xTaskCreatePinnedToCore(RemoteGetLoop, "RemoteGetLoop", DEFAULT_STACK_DEPTH, NULL, PRIORITY_4, &remoteGetTask, CORE0);
  xTaskCreatePinnedToCore(RemoteSetLoop, "RemoteSetLoop", DEFAULT_STACK_DEPTH, NULL, PRIORITY_3, &remoteSetTask, CORE1);
}

static void DeleteEncoderTasks(void)
{
  encoderInterruptSemaphore = NULL;
  encoderQueue = NULL;
  vTaskDelete(encoderGetTask);
  vTaskDelete(encoderSetTask);
  vTaskDelete(encoderInterruptClearTask);
}

static void IRAM_ATTR EncoderGetLoop()
{
  while (1)
  {
    if (xSemaphoreTake(encoderInterruptSemaphore, PORT_DELAY) == pdPASS)
    {
      uint8_t interruptData[2] = {0};
      McpRead(MCP1_ADDRESS, INTFB, &interruptData[0]);
      McpRead(MCP1_ADDRESS, INTCAPB, &interruptData[1]);
      xQueueSend(encoderQueue, &interruptData, PORT_DELAY);
    }
  }
}

static void IRAM_ATTR EncoderSetLoop()
{
  uint8_t interruptData[2] = {0};
  int8_t lastChange=0;
  while (1)
  {
    if(xQueueReceive(encoderQueue, &interruptData, PORT_DELAY)!=pdFALSE)
    {
      lastChange = CreateSnapshot(tunerState, interruptData);
      SetChange(lastChange);
    }
  }
}

static void IRAM_ATTR EncoderInterruptClearLoop()
{
  size_t count = 0;
  while (1)
  {
    if (!gpio_get_level(INTR_PIN))
    {
      count++;
      if (count == 3)
      {
        uint8_t interruptData[2] = {0};
        McpRead(MCP1_ADDRESS, INTFB, &interruptData[0]);
        McpRead(MCP1_ADDRESS, INTCAPB, &interruptData[1]);
        xQueueSend(encoderQueue, &interruptData, PORT_DELAY);
        count = 0;
      }
      vTaskDelay(10 / portTICK_PERIOD_MS);
      continue;
    }
    else
    {
      count = 0;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

static void IRAM_ATTR RemoteGetLoop()
{
  remoteQueue = xQueueCreate(RMT_QUEUE_LENGTH, REMOTE_DATA);
  if (LOGI_MAIN)
  {
    ESP_LOGI(AMP_TAG, "Create RMT RX channel.");
  }
  rmt_rx_channel_config_t rxChannelCfg = {
      .clk_src = RMT_CLK_SRC_DEFAULT,
      .resolution_hz = IR_RESOLUTION_HZ,
      .mem_block_symbols = 64,
      .gpio_num = IR_RX_GPIO_NUM,
  };
  rmt_channel_handle_t rxChannel = NULL;
  ESP_ERROR_CHECK(rmt_new_rx_channel(&rxChannelCfg, &rxChannel));
  
  if (LOGI_MAIN)
  {
    ESP_LOGI(AMP_TAG, "Register RX done callback.");
  }
  QueueHandle_t receiveQueue = xQueueCreate(1, sizeof(rmt_rx_done_event_data_t));
  assert(receiveQueue);
  rmt_rx_event_callbacks_t cbs = {
      .on_recv_done = RmtRxDoneCallback,
  };
  ESP_ERROR_CHECK(rmt_rx_register_event_callbacks(rxChannel, &cbs, receiveQueue));

  rmt_receive_config_t receiveConfig = {
      .signal_range_min_ns = 1250,
      .signal_range_max_ns = 12000000,
  };

  ESP_ERROR_CHECK(rmt_enable(rxChannel));
  rmt_symbol_word_t rawSymbols[64];
  rmt_rx_done_event_data_t rxData;
  ESP_ERROR_CHECK(rmt_receive(rxChannel, rawSymbols, sizeof(rawSymbols),&receiveConfig));
  while (1)
  {
    if (xQueueReceive(receiveQueue, &rxData, pdMS_TO_TICKS(1000)) == pdPASS)
    {
      ParseFrame(rxData.received_symbols, rxData.num_symbols, remoteQueue);
      ESP_ERROR_CHECK(rmt_receive(rxChannel, rawSymbols, sizeof(rawSymbols), &receiveConfig));
    }
  }
}

static void IRAM_ATTR RemoteSetLoop()
{
  uint16_t remoteCode[2] = {0};
  int8_t lastChange=0;
  while (1)
  {
    if(xQueueReceive(remoteQueue, &remoteCode, PORT_DELAY)!=pdFALSE)
    {
      lastChange = RemoteParse(tunerState, remoteCode, standbyMode);
      SetChange(lastChange);
    }
  }
}

static void EncoderInterruptHandler(void *args)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(encoderInterruptSemaphore, &xHigherPriorityTaskWoken);
}

static void StandbyInterruptHandler(void *args)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(standbyInterruptSemaphore, &xHigherPriorityTaskWoken);
}

static void EncoderIsrHandlerAdd(void)
{
  gpio_config_t ioConf;
  ioConf.intr_type = GPIO_INTR_NEGEDGE;
  ioConf.pin_bit_mask = (1 << INTR_PIN);
  ioConf.mode = GPIO_MODE_INPUT;
  ioConf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  ioConf.pull_up_en = GPIO_PULLUP_ENABLE;
  gpio_config(&ioConf);
  esp_err_t ret = gpio_isr_handler_add(INTR_PIN, EncoderInterruptHandler, NULL);
  if (LOGE_MAIN && ret != ESP_OK)
  {
    ESP_LOGE(AMP_TAG, "Encoder handler add error.");
  }
  else if (LOGI_MAIN)
  {
    ESP_LOGI(AMP_TAG, "Encoder handler add OK.");
  }
}

static void EncoderIsrHandlerRemove(void)
{
  esp_err_t ret = gpio_isr_handler_remove(INTR_PIN);
  if (LOGE_MAIN && ret != ESP_OK)
  {
    ESP_LOGE(AMP_TAG, "Encoder handler remove error.");
  }
  else if (LOGI_MAIN) 
  {
    ESP_LOGI(AMP_TAG, "Encoder handler remove OK.");
  }
}

static void StandbyIsrHandlerAdd(void)
{
  gpio_config_t ioConf;
  ioConf.intr_type = GPIO_INTR_NEGEDGE;
  ioConf.pin_bit_mask = (1ULL << STANDBY_BUTTON_GPIO);
  ioConf.mode = GPIO_MODE_INPUT;
  ioConf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  ioConf.pull_up_en = GPIO_PULLUP_ENABLE;
  gpio_config(&ioConf);
  esp_err_t ret = gpio_isr_handler_add(STANDBY_BUTTON_GPIO, StandbyInterruptHandler, NULL);
  if (LOGE_MAIN && ret != ESP_OK)
  {
    ESP_LOGE(AMP_TAG, "Standby handler add error.");
  }
  else if (LOGI_MAIN)
  {
    ESP_LOGI(AMP_TAG, "Standby handler add OK.");
  }
}

static void DisplayWelcomeMessage(void)
{
  LcdText(0, 2, "   Tuner");
  LcdText(1, 1, "version 0.5");
}

static void DisplayInterface(void)
{
  LcdClear();
  SetChange(SOURCE);
  LcdText(0, 0, optionAlias[VOLUME]);
  SetChange(VOLUME);
}

static void SetupAudioDefaults(void)
{
  tunerState[VOLUME][ACT_VAL] = VOL_DEFAULT;
  tunerState[SOURCE][ACT_VAL] = SRC_DEFAULT;
  tunerState[BALANCE][ACT_VAL] = BAL_DEFAULT;
  tunerState[TREBLE][ACT_VAL] = TRE_DEFAULT;
  tunerState[BASS][ACT_VAL] = BAS_DEFAULT;
  tunerState[TONE_SW][ACT_VAL] = TON_DEFAULT;
}

static void BlinkLed(uint8_t state)
{
  gpio_set_level(LED_GPIO, state);
}

static void GpioInit(void)
{
  gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
  gpio_set_direction(AMP_POWER_CONTROL_GPIO, GPIO_MODE_OUTPUT);
  gpio_set_direction(AMP_AUDIO_CONTROL_GPIO, GPIO_MODE_OUTPUT);
  gpio_set_direction(AUDIO_POWER_CONTROL_GPIO, GPIO_MODE_OUTPUT);
  gpio_set_direction(DIGITAL_POWER_CONTROL_GPIO, GPIO_MODE_OUTPUT);
}

static void AudioPowerToggle(void)
{
  // bistable module
  vTaskDelay(60 / portTICK_PERIOD_MS);
  gpio_set_level(AUDIO_POWER_CONTROL_GPIO, ON);
  vTaskDelay(60 / portTICK_PERIOD_MS);
  gpio_set_level(AUDIO_POWER_CONTROL_GPIO, OFF);
}

static void DigitalPowerToggle(void)
{
  // bistable module
  vTaskDelay(60 / portTICK_PERIOD_MS);
  gpio_set_level(DIGITAL_POWER_CONTROL_GPIO, ON);
  vTaskDelay(60 / portTICK_PERIOD_MS);
  gpio_set_level(DIGITAL_POWER_CONTROL_GPIO, OFF);
}

static void AmpAudioControl(uint8_t state)
{
  gpio_set_level(AMP_AUDIO_CONTROL_GPIO, state);
}

static void AmpPowerControl(uint8_t state)
{
  gpio_set_level(AMP_POWER_CONTROL_GPIO, state);
}

static void IRAM_ATTR SetChange(int8_t lastChange)
{
  char buffer[17];
  switch (lastChange)
  {
  case SOURCE:
    SetVolume(tunerState[VOLUME][MIN_VAL], tunerState[BALANCE][ACT_VAL]);
    SetSource(tunerState);
    sprintf(buffer, "%s", sourceAlias[tunerState[SOURCE][ACT_VAL]]);
    LcdClearLine(1);
    LcdText(1, 0, buffer);
    SetVolume(tunerState[VOLUME][ACT_VAL], tunerState[BALANCE][ACT_VAL]);
    break;
  case VOLUME:
    SetVolume(tunerState[VOLUME][ACT_VAL], tunerState[BALANCE][ACT_VAL]);
    sprintf(buffer, "%2d", tunerState[VOLUME][ACT_VAL]);
    LcdText(0, 7, buffer);
    break;
  case STANDBY:
    standbyMode = !standbyMode;
    Standby(standbyMode);
    break;
  case BALANCE:
    SetVolume(tunerState[VOLUME][ACT_VAL], tunerState[BALANCE][ACT_VAL]);
    sprintf(buffer, "%3d", tunerState[BALANCE][ACT_VAL]);
    LcdText(1, 8, buffer);
    break;
  case TREBLE:
    SetTone(tunerState[TONE_SW][ACT_VAL], TREBLE_CTRL, tunerState[TREBLE][ACT_VAL]);
    sprintf(buffer, "%3d", tunerState[TREBLE][ACT_VAL]);
    LcdText(1, 7, buffer);
    break;
  case BASS:
    SetTone(tunerState[TONE_SW][ACT_VAL], BASS_CTRL, tunerState[BASS][ACT_VAL]);
    sprintf(buffer, "%3d", tunerState[BASS][ACT_VAL]);
    LcdText(1, 5, buffer);
    break;
  case TONE_SW:
    SetTone(tunerState[TONE_SW][ACT_VAL], TREBLE_CTRL, tunerState[TREBLE][ACT_VAL]);
    if (tunerState[TONE_SW][ACT_VAL] == 1)
      sprintf(buffer, "%s", "ON ");
    else
      sprintf(buffer, "%s", "OFF");
    LcdText(1, 12, buffer);
    break;
  case SELECTED:
    LcdClearLine(1);
    if (tunerState[SELECTED][ACT_VAL] == SOURCE)
    {
      sprintf(buffer, "%s", sourceAlias[tunerState[SOURCE][ACT_VAL]]);
    }
    else if (tunerState[SELECTED][ACT_VAL] == TONE_SW && tunerState[TONE_SW][ACT_VAL] == 1)
    {
      sprintf(buffer, "%s %s", optionAlias[tunerState[SELECTED][ACT_VAL]], "ON");
    }
    else if (tunerState[SELECTED][ACT_VAL] == TONE_SW && tunerState[TONE_SW][ACT_VAL] == 0)
    {
      sprintf(buffer, "%s %s", optionAlias[tunerState[SELECTED][ACT_VAL]], "OFF");
    }
    else
    {
      sprintf(buffer, "%s %3d", optionAlias[tunerState[SELECTED][ACT_VAL]], tunerState[tunerState[SELECTED][ACT_VAL]][ACT_VAL]);
    }
    LcdText(1, 0, buffer);
  }
}

static void Standby(uint8_t state)
{
  if (state == ON)
  {
    if (LOGI_MAIN)
    {
      ESP_LOGI(AMP_TAG, "Going into standby mode.");
    }
    AmpAudioControl(ON);
    AudioPowerToggle();
    AmpPowerControl(ON);
    LcdClear();
    LcdText(0, 0, "   going into");
    LcdText(1, 0, "  Standby mode");
    SetVolume(tunerState[VOLUME][MIN_VAL], tunerState[BALANCE][ACT_VAL]);
    SetAnalogInput(INPUT_MUTE);
    EncoderIsrHandlerRemove();
    SpiBusRemoveNjw1194();
    DeleteEncoderTasks();
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    DigitalPowerToggle();
    if (LOGI_MAIN)
    {
      ESP_LOGI(AMP_TAG, "Stanby mode ON");
    }
    LcdClear();
  }
  else if (state == OFF)
  {
    if (LOGI_MAIN)
    {
      ESP_LOGI(AMP_TAG, "Waking up from Stanby mode");
    }
    AmpAudioControl(ON);
    AmpPowerControl(OFF);
    DigitalPowerToggle();
    AudioPowerToggle();
    McpInit(MCP1_ADDRESS);
    LcdInit();
    DisplayWelcomeMessage();
    SetupEncoderTasks();
    EncoderIsrHandlerAdd();
    SpiBusAddNjw1194();
    SourcesInit();
    SetTone(tunerState[TONE_SW][ACT_VAL], TREBLE_CTRL, tunerState[TREBLE][ACT_VAL]);
    SetTone(tunerState[TONE_SW][ACT_VAL], BASS_CTRL, tunerState[BASS][ACT_VAL]);
    SetSource(tunerState);
    if (tunerState[VOLUME][ACT_VAL]>VOL_MAX_ON_STARTUP){
      tunerState[VOLUME][ACT_VAL] = VOL_MAX_ON_STARTUP;
    }
    SetVolume(tunerState[VOLUME][ACT_VAL], tunerState[BALANCE][ACT_VAL]);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    DisplayInterface();
    AmpAudioControl(OFF);
    if (LOGI_MAIN)
    {
      ESP_LOGI(AMP_TAG, "Stanby mode off");
    }
  }
}
