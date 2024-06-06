#include "encoder.h"

int8_t CheckInterrupt(int8_t (*tunerState)[VAR_NO])
{
  uint8_t snapshot[SNAP_SIZE];
  uint8_t lastInterrupt;
  uint8_t capturedValues;
  McpRead(MCP1_ADDRESS, INTFB, &lastInterrupt);
  McpRead(MCP1_ADDRESS, INTCAPB, &capturedValues);
  for (uint8_t i = 0; i < 8; i++)
  {
    if (lastInterrupt & (1 << i))
    {
      snapshot[PINA] = i;
    }
  }
  snapshot[PINA_VAL] = (capturedValues >> snapshot[PINA]) & 0x01;
  if (snapshot[PINA] == ENC1_PINA || snapshot[PINA] == ENC2_PINA)
    snapshot[PINB] = snapshot[PINA] + 1;
  else if (snapshot[PINA] == ENC1_SW_PIN || snapshot[PINA] == ENC2_SW_PIN)
    snapshot[PINB] = snapshot[PINA];
  snapshot[PINB_VAL] = (capturedValues >> snapshot[PINB]) & 0x01;
  if (LOGI_ENC)
    ESP_LOGI(ENC_TAG, "Snapshot: PINA %d, PINB %d, PINA val %d, PINB val %d.", snapshot[PINA], snapshot[PINB], snapshot[PINA_VAL], snapshot[PINB_VAL]);
  return DecodePins(snapshot, tunerState);
}

int8_t CreateSnapshot(int8_t (*tunerState)[VAR_NO], uint8_t *interruptData)
{
  uint8_t snapshot[SNAP_SIZE];
  for (uint8_t i = 0; i < 8; i++)
  {
    if (interruptData[0] & (1 << i))
    {
      snapshot[PINA] = i;
    }
  }
  snapshot[PINA_VAL] = (interruptData[1] >> snapshot[PINA]) & 0x01;
  if (snapshot[PINA] == ENC1_PINA || snapshot[PINA] == ENC2_PINA)
    snapshot[PINB] = snapshot[PINA] + 1;
  else if (snapshot[PINA] == ENC1_SW_PIN || snapshot[PINA] == ENC2_SW_PIN)
    snapshot[PINB] = snapshot[PINA];
  snapshot[PINB_VAL] = (interruptData[1] >> snapshot[PINB]) & 0x01;
  if (LOGI_ENC)
    ESP_LOGI(ENC_TAG, "Snapshot: PINA %d, PINB %d, PINA val %d, PINB val %d.", snapshot[PINA], snapshot[PINB], snapshot[PINA_VAL], snapshot[PINB_VAL]);
  return DecodePins(snapshot, tunerState);
}

int8_t DecodePins(uint8_t *snapshot, int8_t (*tunerState)[VAR_NO])
{
  if (snapshot[PINA] == ENC1_PINA && snapshot[PINB] == ENC1_PINB)
    return OptionControl(snapshot, tunerState, ENC1_TOG);
  else if (snapshot[PINA] == ENC1_SW_PIN && snapshot[PINA_VAL] == true)
    return ButtonControl(tunerState, ENC1_SW);
  else if (snapshot[PINA] == ENC2_PINA && snapshot[PINB] == ENC2_PINB)
    return OptionControl(snapshot, tunerState, ENC2_TOG);
  else if (snapshot[PINA] == ENC2_SW_PIN && snapshot[PINA_VAL] == true)
    return ButtonControl(tunerState, ENC2_SW);
  else
  {
    return NONE;
  }
}

int8_t OptionControl(uint8_t *snapshot, int8_t (*tunerState)[VAR_NO], int8_t option)
{
  int8_t select = VOLUME;
  if (option == ENC2_TOG)
    select = tunerState[SELECTED][ACT_VAL];
  if (tunerState[select][ACT_VAL] != tunerState[select][MIN_VAL])
    tunerState[select][LAS_VAL] = tunerState[select][ACT_VAL]; // save last state of chosen option
  if (snapshot[PINA_VAL] == FALSE && snapshot[PINB_VAL] == TRUE)
  {
    tunerState[select][ACT_VAL]++;
    if (LOGI_ENC)
      ESP_LOGI(ENC_TAG, "Option %d. Value: %d.", select, tunerState[select][ACT_VAL]);
    return CheckOverflow(tunerState, select);
  }
  else if (snapshot[PINA_VAL] == FALSE && snapshot[PINB_VAL] == FALSE)
  {
    tunerState[select][ACT_VAL]--;
    if (LOGI_ENC)
      ESP_LOGI(ENC_TAG, "Option %d. Value: %d.", select, tunerState[select][ACT_VAL]);
    return CheckOverflow(tunerState, select);
  }
  else
    return NONE;
}

int8_t CheckOverflow(int8_t (*tunerState)[VAR_NO], int8_t select)
{
  if (tunerState[select][OVFL_VAL] == OVFL_NO && tunerState[select][ACT_VAL] > tunerState[select][MAX_VAL])
  {
    tunerState[select][LAS_VAL] = tunerState[select][MAX_VAL];
    tunerState[select][ACT_VAL] = tunerState[select][MAX_VAL];
    return select;
  }
  else if (tunerState[select][OVFL_VAL] == OVFL_NO && tunerState[select][ACT_VAL] < tunerState[select][MIN_VAL])
  {
    tunerState[select][LAS_VAL] = tunerState[select][MIN_VAL];
    tunerState[select][ACT_VAL] = tunerState[select][MIN_VAL];
    return select;
  }
  else if (tunerState[select][OVFL_VAL] == OVFL_YES && tunerState[select][ACT_VAL] > tunerState[select][MAX_VAL])
  {
    tunerState[select][LAS_VAL] = tunerState[select][MIN_VAL];
    tunerState[select][ACT_VAL] = tunerState[select][MIN_VAL];
    return select;
  }
  else if (tunerState[select][OVFL_VAL] == OVFL_YES && tunerState[select][ACT_VAL] < tunerState[select][MIN_VAL])
  {
    tunerState[select][LAS_VAL] = tunerState[select][MAX_VAL];
    tunerState[select][ACT_VAL] = tunerState[select][MAX_VAL];
    return select;
  }
  else
    return select;
}

int8_t ButtonControl(int8_t (*tunerState)[VAR_NO], int8_t option)
{
  if (option == ENC1_SW)
  {
    return Mute(tunerState, VOLUME);
  }
  else if (option == ENC2_SW)
  {
    return Toggle(tunerState, SELECTED);
  }
  else
    return NONE;
}

int8_t Mute(int8_t (*tunerState)[VAR_NO], int8_t select)
{
  if (tunerState[select][ACT_VAL] > tunerState[select][MIN_VAL])
  {
    tunerState[select][LAS_VAL] = tunerState[select][ACT_VAL];
    tunerState[select][ACT_VAL] = tunerState[select][MIN_VAL];
    if (LOGI_ENC)
      ESP_LOGI(ENC_TAG, "Option: %d. Value: %d.", select, tunerState[select][ACT_VAL]);
    return select;
  }
  else
  {
    tunerState[select][ACT_VAL] = tunerState[select][LAS_VAL];
    if (LOGI_ENC)
      ESP_LOGI(ENC_TAG, "Option: %d. Value: %d.", select, tunerState[select][ACT_VAL]);
    return select;
  }
}

int8_t Toggle(int8_t (*tunerState)[VAR_NO], int8_t select)
{
  tunerState[select][LAS_VAL] = tunerState[select][ACT_VAL];
  tunerState[select][ACT_VAL]++;
  if (LOGI_ENC)
    ESP_LOGI(ENC_TAG, "Option set to %d. Value: %d", select, tunerState[select][ACT_VAL]);
  return CheckOverflow(tunerState, select);
}
