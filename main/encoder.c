#include "encoder.h"
static const char* TAG = "Encoder";


int8_t check_interrupt(int8_t (*tuner_state)[VAR_NO]){
  uint8_t snapshot[SNAP_SIZE] ;
  uint8_t last_interrupt;
  uint8_t captured_values;
  mcp_read(MCP1_ADDRESS, INTFB, &last_interrupt);
  mcp_read(MCP1_ADDRESS, INTCAPB, &captured_values);
  for (uint8_t i = 0; i < 8; i++) {
    if (last_interrupt & (1 << i)) {
      snapshot[PINA]=i;
    }
  }
  snapshot[PINA_VAL]=(captured_values>>snapshot[PINA])&0x01;
  if (snapshot[PINA]==ENC1_PINA || snapshot[PINA]==ENC2_PINA)
    snapshot[PINB]=snapshot[PINA]+1;
  else if (snapshot[PINA]==ENC1_SW_PIN || snapshot[PINA]==ENC2_SW_PIN)
    snapshot[PINB]=snapshot[PINA];
  snapshot[PINB_VAL]=(captured_values>>snapshot[PINB])&0x01;
    if(LOGI_ENC) ESP_LOGI(TAG, "Snapshot: PINA %d, PINB %d, PINA val %d, PINB val %d.", snapshot[PINA], snapshot[PINB], snapshot[PINA_VAL], snapshot[PINB_VAL]);
  return decode_pins(snapshot, tuner_state);
}

int8_t create_snapshot(int8_t (*tuner_state)[VAR_NO], uint8_t* interrupt_data){
  uint8_t snapshot[SNAP_SIZE] ;
  for (uint8_t i = 0; i < 8; i++) {
    if (interrupt_data[0] & (1 << i)) {
      snapshot[PINA]=i;
    }
  }
  snapshot[PINA_VAL]=(interrupt_data[1]>>snapshot[PINA])&0x01;
  if (snapshot[PINA]==ENC1_PINA || snapshot[PINA]==ENC2_PINA)
    snapshot[PINB]=snapshot[PINA]+1;
  else if (snapshot[PINA]==ENC1_SW_PIN || snapshot[PINA]==ENC2_SW_PIN)
    snapshot[PINB]=snapshot[PINA];
  snapshot[PINB_VAL]=(interrupt_data[1]>>snapshot[PINB])&0x01;
    if(LOGI_ENC) ESP_LOGI(TAG, "Snapshot: PINA %d, PINB %d, PINA val %d, PINB val %d.", snapshot[PINA], snapshot[PINB], snapshot[PINA_VAL], snapshot[PINB_VAL]);
  return decode_pins(snapshot, tuner_state);
}


int8_t decode_pins(uint8_t *snapshot, int8_t (*tuner_state)[VAR_NO]){
  if (snapshot[PINA]==ENC1_PINA && snapshot[PINB]==ENC1_PINB)
    return option_ctrl(snapshot, tuner_state, ENC1_TOG);
  else if (snapshot[PINA]==ENC1_SW_PIN && snapshot[PINA_VAL]==true)
    return button_ctrl(tuner_state, ENC1_SW);
  else if (snapshot[PINA]==ENC2_PINA && snapshot[PINB]==ENC2_PINB)
    return option_ctrl(snapshot, tuner_state, ENC2_TOG);
  else if (snapshot[PINA]==ENC2_SW_PIN && snapshot[PINA_VAL]==true)
    return button_ctrl(tuner_state, ENC2_SW);
  else{ 
        return NONE;
  }
}

int8_t option_ctrl(uint8_t *snapshot, int8_t (*tuner_state)[VAR_NO], int8_t option){
  int8_t select = VOLUME; 
  if(option==ENC2_TOG)
    select = tuner_state[SELECTED][ACT_VAL];
  if(tuner_state[select][ACT_VAL]!=tuner_state[select][MIN_VAL])
    tuner_state[select][LAS_VAL]=tuner_state[select][ACT_VAL]; //save last state of chosen option
  if(snapshot[PINA_VAL]==FALSE && snapshot[PINB_VAL]==TRUE){
    tuner_state[select][ACT_VAL]++;
    if(LOGI_ENC) ESP_LOGI(TAG, "Option %d. Value: %d.", select, tuner_state[select][ACT_VAL]);
    return check_overflow(tuner_state, select);
  }  
  else if (snapshot[PINA_VAL]==FALSE && snapshot[PINB_VAL]==FALSE) {
      tuner_state[select][ACT_VAL]--;
    if(LOGI_ENC) ESP_LOGI(TAG, "Option %d. Value: %d.", select, tuner_state[select][ACT_VAL]);
    return check_overflow(tuner_state, select);
  }
  else
    return NONE;  
}

int8_t check_overflow(int8_t (*tuner_state)[VAR_NO], int8_t select){
  if (tuner_state[select][OVFL_VAL]==OVFL_NO && tuner_state[select][ACT_VAL]>tuner_state[select][MAX_VAL]){
    tuner_state[select][LAS_VAL]=tuner_state[select][MAX_VAL];
    tuner_state[select][ACT_VAL]=tuner_state[select][MAX_VAL];
    return select;
  }
  else if (tuner_state[select][OVFL_VAL]==OVFL_NO && tuner_state[select][ACT_VAL]<tuner_state[select][MIN_VAL]){
    tuner_state[select][LAS_VAL]=tuner_state[select][MIN_VAL];
    tuner_state[select][ACT_VAL]=tuner_state[select][MIN_VAL];
    return select;
  }
  else if(tuner_state[select][OVFL_VAL]==OVFL_YES && tuner_state[select][ACT_VAL]>tuner_state[select][MAX_VAL]){
    tuner_state[select][LAS_VAL]=tuner_state[select][MIN_VAL];
    tuner_state[select][ACT_VAL]=tuner_state[select][MIN_VAL];
    return select;
  }
  else if (tuner_state[select][OVFL_VAL]==OVFL_YES && tuner_state[select][ACT_VAL]<tuner_state[select][MIN_VAL]){
    tuner_state[select][LAS_VAL]=tuner_state[select][MAX_VAL];
    tuner_state[select][ACT_VAL]=tuner_state[select][MAX_VAL];
    return select;
  }   
  else
    return select;
}


int8_t button_ctrl(int8_t (*tuner_state)[VAR_NO], int8_t option){
  if(option==ENC1_SW){
    return mute(tuner_state, VOLUME);
  }
  else if(option==ENC2_SW){
    return toggle(tuner_state, SELECTED);
  }
  else
    return NONE;
}

int8_t mute(int8_t (*tuner_state)[VAR_NO], int8_t select){
  if(tuner_state[select][ACT_VAL]>tuner_state[select][MIN_VAL]){
    tuner_state[select][LAS_VAL]=tuner_state[select][ACT_VAL];
    tuner_state[select][ACT_VAL]=tuner_state[select][MIN_VAL]; 
    if(LOGI_ENC) ESP_LOGI(TAG, "Option: %d. Value: %d.", select, tuner_state[select][ACT_VAL]);
    return select;
  }  
  else{
    tuner_state[select][ACT_VAL]=tuner_state[select][LAS_VAL];
    if(LOGI_ENC) ESP_LOGI(TAG, "Option: %d. Value: %d.", select, tuner_state[select][ACT_VAL]);
    return select;
  } 
}

int8_t toggle (int8_t (*tuner_state)[VAR_NO], int8_t select){
  tuner_state[select][LAS_VAL]=tuner_state[select][ACT_VAL]; 
  tuner_state[select][ACT_VAL]++;  
  if(LOGI_ENC) ESP_LOGI(TAG, "Option set to %d. Value: %d", select, tuner_state[select][ACT_VAL]);
  return check_overflow(tuner_state, select);
}
