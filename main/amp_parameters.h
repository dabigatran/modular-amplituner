#ifndef _amp_parameters_H_
#define _amp_parameters_H_
#define OPTION_NO (uint8_t)7
#define VAR_NO (uint8_t)5
#define MAX_SRC_ALIAS_LENGTH (uint8_t)16
#define MAX_OPT_ALIAS_LENGTH (uint8_t)12
#define MAX_SOURCES (uint8_t)9

static const gpio_num_t LED_GPIO = GPIO_NUM_2;
static const gpio_num_t STANDBY_BUTTON_GPIO = GPIO_NUM_27;
static const gpio_num_t AUDIO_POWER_CONTROL_GPIO = GPIO_NUM_18;
static const gpio_num_t DIGITAL_POWER_CONTROL_GPIO = GPIO_NUM_26;
static const gpio_num_t AMP_AUDIO_CONTROL_GPIO = GPIO_NUM_25;
static const gpio_num_t AMP_POWER_CONTROL_GPIO = GPIO_NUM_33;

static const int8_t SRC_MAX = 7;
static const int8_t SRC_MIN = 0;
static const int8_t BAL_MAX = 20;
static const int8_t BAL_MIN = -20;
static const int8_t TRE_MAX = 10;
static const int8_t TRE_MIN = -10;
static const int8_t BAS_MAX = 10;
static const int8_t BAS_MIN = -10;
static const int8_t TON_MAX = 1;
static const int8_t TON_MIN = 0;
static const int8_t SEL_MIN = 2;
static const int8_t SEL_MAX = 6;
static const int8_t NONE = -1;
static const int8_t VOLUME = 0;
static const int8_t SELECTED = 1;
static const int8_t SOURCE = 2;
static const int8_t BALANCE = 3;
static const int8_t TREBLE = 4;
static const int8_t BASS = 5;
static const int8_t TONE_SW = 6;
static const int8_t STANDBY = 7;

static const int8_t VAR_MIN = 0;
static const int8_t VAR_MAX = 3;
static const int8_t ACT_VAL = 0;
static const int8_t LAS_VAL = 1;
static const int8_t MIN_VAL = 2;
static const int8_t MAX_VAL = 3;
static const int8_t OVFL_VAL = 4;
static const int8_t OVFL_NO = 0;
static const int8_t OVFL_YES = 1;
static const int8_t PORT_DELAY = 2;
static const int8_t ENC_QUEUE_LENGTH = 40;
static const int8_t RMT_QUEUE_LENGTH = 20;
static const int8_t ON = 1;
static const int8_t OFF = 0;
static const int8_t MY_CONSTANT = 23;
static const int8_t VOL_MIN = 0;
static const int8_t VOL_MAX = 40;
#endif