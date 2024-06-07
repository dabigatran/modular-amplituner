#include "remote.h"

bool RmtRxDoneCallback(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t *edata, void *userData)
{
    BaseType_t highTaskWakeup = pdFALSE;
    QueueHandle_t receiveQueue = (QueueHandle_t)userData;
    xQueueSendFromISR(receiveQueue, edata, &highTaskWakeup);
    return highTaskWakeup == pdTRUE;
}

int8_t RemoteParse(int8_t (*tunerState)[VAR_NO], uint16_t remoteCode[2], bool standbyMode)
{
    if (LOGI_RMT)
    {
        ESP_LOGI(RMT_TAG, "Equipment code: %02x, function code: %02x", remoteCode[0], remoteCode[1]);
    }
    
    if (standbyMode == OFF && remoteCode[0] == CHROMECAST_ADDRESS)
    {
        switch (remoteCode[1])
        {
        case CHROMECAST_ON_OFF:
            return STANDBY;
        case CHROMECAST_INPUT_TOGGLE:
            tunerState[SELECTED][ACT_VAL] = SOURCE;
            tunerState[SOURCE][ACT_VAL]++;
            CheckOverflow(tunerState, SOURCE);
            return SOURCE;
        case CHROMECAST_VOLUME_UP:
            tunerState[VOLUME][ACT_VAL]++;
            CheckOverflow(tunerState, VOLUME);
            return VOLUME;
        case CHROMECAST_VOLUME_DOWN:
            tunerState[VOLUME][ACT_VAL]--;
            CheckOverflow(tunerState, VOLUME);
            return VOLUME;
        case CHROMECAST_MUTE_UNMUTE:
            Mute(tunerState, VOLUME);
            return VOLUME;
        }
    }
    else if (standbyMode == ON && remoteCode[0] == CHROMECAST_ADDRESS)
    {
        if (remoteCode[1] == CHROMECAST_ON_OFF)
        {
            return STANDBY;
        }
    }
    return NONE;
}

void ParseFrame(rmt_symbol_word_t *rmtNecSymbols, size_t symbolNum, QueueHandle_t remoteQueue)
{
    uint16_t remoteCode[2];
    switch (symbolNum)
    {
    case 34: // NEC normal frame
        if (NecParseFrame(rmtNecSymbols, remoteCode))
        {
            xQueueSend(remoteQueue, &remoteCode, PORT_DELAY);
        }
        break;
    default:
        break;
    }
}

bool NecParseFrame(rmt_symbol_word_t *rmtNecSymbols, uint16_t *remoteCode)
{
    rmt_symbol_word_t *cur = rmtNecSymbols;
    uint16_t address = 0;
    uint16_t command = 0;
    bool validLeadingCode = NecCheckInRange(cur->duration0, NEC_LEADING_CODE_DURATION_0) &&
                              NecCheckInRange(cur->duration1, NEC_LEADING_CODE_DURATION_1);
    if (!validLeadingCode)
    {
        return false;
    }
    cur++;
    for (int i = 0; i < 16; i++)
    {
        if (NecParseLogic1(cur))
        {
            address |= 1 << i;
        }
        else if (NecParseLogic0(cur))
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
        if (NecParseLogic1(cur))
        {
            command |= 1 << i;
        }
        else if (NecParseLogic0(cur))
        {
            command &= ~(1 << i);
        }
        else
        {
            return false;
        }
        cur++;
    }
    remoteCode[0] = address;
    remoteCode[1] = command;
    return true;
}

inline bool NecCheckInRange(uint32_t signalDuration, uint32_t specDuration)
{
    return (signalDuration < (specDuration + IR_NEC_DECODE_MARGIN)) &&
           (signalDuration > (specDuration - IR_NEC_DECODE_MARGIN));
}

bool NecParseLogic0(rmt_symbol_word_t *rmtNecSymbols)
{
    return NecCheckInRange(rmtNecSymbols->duration0, NEC_PAYLOAD_ZERO_DURATION_0) &&
           NecCheckInRange(rmtNecSymbols->duration1, NEC_PAYLOAD_ZERO_DURATION_1);
}

bool NecParseLogic1(rmt_symbol_word_t *rmtNecSymbols)
{
    return NecCheckInRange(rmtNecSymbols->duration0, NEC_PAYLOAD_ONE_DURATION_0) &&
           NecCheckInRange(rmtNecSymbols->duration1, NEC_PAYLOAD_ONE_DURATION_1);
}