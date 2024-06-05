#include "remote.h"

static const char *TAG = "Remote Control";

bool rmt_rx_done_callback(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t *edata, void *user_data)
{
    BaseType_t high_task_wakeup = pdFALSE;
    QueueHandle_t receive_queue = (QueueHandle_t)user_data;
    xQueueSendFromISR(receive_queue, edata, &high_task_wakeup);
    return high_task_wakeup == pdTRUE;
}

int8_t remote_parse(int8_t (*tuner_state)[VAR_NO], QueueHandle_t remote_queue)
{
    uint16_t remote_code[2] = {0};
    xQueueReceive(remote_queue, &remote_code, PORT_DELAY);
    if (remote_code[0] == CHROMECAST_ADDRESS)
    {
        switch (remote_code[1])
        {
        case CHROMECAST_ON_OFF:
            return STANDBY;
        case CHROMECAST_INPUT_TOGGLE:
            tuner_state[SELECTED][ACT_VAL] = SOURCE;
            tuner_state[SOURCE][ACT_VAL]++;
            check_overflow(tuner_state, SOURCE);
            return SOURCE;
        case CHROMECAST_VOLUME_UP:
            tuner_state[VOLUME][ACT_VAL]++;
            check_overflow(tuner_state, VOLUME);
            return VOLUME;
        case CHROMECAST_VOLUME_DOWN:
            tuner_state[VOLUME][ACT_VAL]--;
            check_overflow(tuner_state, VOLUME);
            return VOLUME;
        case CHROMECAST_MUTE_UNMUTE:
            mute(tuner_state, VOLUME);
            return VOLUME;
        }
    }
    return NONE;
}

void parse_frame(rmt_symbol_word_t *rmt_nec_symbols, size_t symbol_num, QueueHandle_t remote_queue)
{
    uint16_t remote_code[2];
    switch (symbol_num)
    {
    case 34: // NEC normal frame
        if (nec_parse_frame(rmt_nec_symbols, remote_code))
        {
            xQueueSend(remote_queue, &remote_code, PORT_DELAY);
        }
        break;
    default:
        break;
    }
}

bool nec_parse_frame(rmt_symbol_word_t *rmt_nec_symbols, uint16_t *remote_code)
{
    rmt_symbol_word_t *cur = rmt_nec_symbols;
    uint16_t address = 0;
    uint16_t command = 0;
    bool valid_leading_code = nec_check_in_range(cur->duration0, NEC_LEADING_CODE_DURATION_0) &&
                              nec_check_in_range(cur->duration1, NEC_LEADING_CODE_DURATION_1);
    if (!valid_leading_code)
    {
        return false;
    }
    cur++;
    for (int i = 0; i < 16; i++)
    {
        if (nec_parse_logic1(cur))
        {
            address |= 1 << i;
        }
        else if (nec_parse_logic0(cur))
        {
            address &= ~(1 << i);
        }
        else
        {
            return false;
        }
        cur++;
    }
    for (int i = 0; i < 16; i++)
    {
        if (nec_parse_logic1(cur))
        {
            command |= 1 << i;
        }
        else if (nec_parse_logic0(cur))
        {
            command &= ~(1 << i);
        }
        else
        {
            return false;
        }
        cur++;
    }
    remote_code[0] = address;
    remote_code[1] = command;
    return true;
}

inline bool nec_check_in_range(uint32_t signal_duration, uint32_t spec_duration)
{
    return (signal_duration < (spec_duration + IR_NEC_DECODE_MARGIN)) &&
           (signal_duration > (spec_duration - IR_NEC_DECODE_MARGIN));
}

bool nec_parse_logic0(rmt_symbol_word_t *rmt_nec_symbols)
{
    return nec_check_in_range(rmt_nec_symbols->duration0, NEC_PAYLOAD_ZERO_DURATION_0) &&
           nec_check_in_range(rmt_nec_symbols->duration1, NEC_PAYLOAD_ZERO_DURATION_1);
}

bool nec_parse_logic1(rmt_symbol_word_t *rmt_nec_symbols)
{
    return nec_check_in_range(rmt_nec_symbols->duration0, NEC_PAYLOAD_ONE_DURATION_0) &&
           nec_check_in_range(rmt_nec_symbols->duration1, NEC_PAYLOAD_ONE_DURATION_1);
}