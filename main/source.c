#include "source.h"

static const char* TAG = "Source";
static uint8_t sources_map[MAX_SOURCES]={NOT_FOUND};
static uint8_t sources_available[MAX_SOURCES]={NOT_FOUND};

void sources_init(){
    map_sources();
    set_availibility();
}

void map_sources(){
    sources_map[SOURCE0]=   INPUT_1; 
    sources_map[SOURCE1]=   (HDMI1&0x0f);//value of HDMI_nib (lo_nib)
    sources_map[SOURCE2]=   (HDMI2&0x0f); //value of HDMI_nib (lo_nib)
    sources_map[SOURCE3]=   (HDMI3&0x0f); //value of HDMI_nib (lo_nib)
    sources_map[SOURCE4]=   (HDMI4&0x0f); //value of HDMI_nib (lo_nib)
    sources_map[SOURCE5]=   ((I2S1>>4)&0x0f); //value of i2s_nib (hi_nib)
    sources_map[SOURCE6]=   ((I2S2>>4)&0x0f); //value of i2s_nib (hi_nib)
    sources_map[SOURCE7]=   ((I2S3>>4)&0x0f); //value of i2s_nib (hi_nib)
    sources_map[SOURCE8]=   ((I2S4>>4)&0x0f); //value of i2s_nib (hi_nib)
}

void set_availibility(){
    sources_available[SOURCE0]=FOUND; 
    for(int8_t i=SOURCE1; i<=SOURCE4; i++){ //sources 1-4
        int8_t source = is_hdmi_available();
        sources_available[source]=FOUND;
    }
    for(int8_t i=SOURCE6; i<=SOURCE7; i++){ //sources 6-7, 5 and 8 are used for hdmi and radio output
        sources_available[i]=FOUND;
    }
}

int8_t is_hdmi_available(){
    uint8_t value=0;
    switch_hdmi();
    mcp_read(MCP1_ADDRESS, GPIOA, &value);
    int8_t hdmi_nib = (int8_t)value&0x0f; 
    return hdmi_nib_to_source(hdmi_nib);
}

int8_t hdmi_nib_to_source(int8_t nib_value){
    for(int8_t i =SOURCE1; i<=SOURCE4; i++){
        if(nib_value==sources_map[i]) return i;
    }
    return 0;
}

int8_t i2s_nib_to_source(int8_t nib_value){
    for(int8_t i =SOURCE5; i<=SOURCE8; i++){
        if(nib_value==sources_map[i]) return i;
    }
    return 0;
}

void set_source(int8_t (*tuner_state)[VAR_NO]){
    /*  
        Checking if source is available. If not-> select next  or previous depending
        on toogle direction.
    */
    while(!sources_available[tuner_state[SOURCE][ACT_VAL]]){
        if(tuner_state[SOURCE][LAS_VAL]<=tuner_state[SOURCE][ACT_VAL]){
            tuner_state[SOURCE][LAS_VAL]=tuner_state[SOURCE][ACT_VAL];
            tuner_state[SOURCE][ACT_VAL]++;
        }
        else{
            tuner_state[SOURCE][LAS_VAL]=tuner_state[SOURCE][ACT_VAL];
            tuner_state[SOURCE][ACT_VAL]--;
        }
        if (tuner_state[SOURCE][ACT_VAL]<SOURCE0){
            tuner_state[SOURCE][ACT_VAL]=MAX_SOURCES-1;
            tuner_state[SOURCE][LAS_VAL]=MAX_SOURCES-1;
        }
        if (tuner_state[SOURCE][ACT_VAL]>=MAX_SOURCES){
            tuner_state[SOURCE][ACT_VAL]=SOURCE0;
            tuner_state[SOURCE][LAS_VAL]=SOURCE0; 
        }
    }
    /*Change to available source*/
    int8_t change_response = change_source(tuner_state[SOURCE][ACT_VAL]);
    if(change_response!=tuner_state[SOURCE][ACT_VAL])
        tuner_state[SOURCE][ACT_VAL]=change_response;
}

int8_t change_source(int8_t source){
   if(LOGI_SRC) ESP_LOGI(TAG, "Change source: %d", source);
   switch (source){
      case SOURCE0: //analog 
            set_analog_input(INPUT_1);
            return source;
      case SOURCE1 ... SOURCE4: //hdmi sources 
            int8_t response = toggle_hdmi(source);
            if (response==NOT_FOUND){
                //change to i2s source if no hdmi source found
                toggle_i2s(SOURCE6);
                set_analog_input(INPUT_2);
                return SOURCE6;
            }
            else{    
                toggle_i2s(SOURCE5);
                set_analog_input(INPUT_2);
                return response;
            }
      case SOURCE6 ... SOURCE7: //i2s sources, remark: SOURCE5 and 8 are used for hdmi and radio output
            toggle_i2s(source);
            set_analog_input(INPUT_2);
            return source;
   }
   return source; 
}

int8_t toggle_hdmi(int8_t source){
    //nib[0]->searched(selected) source, nib[1]->set source (read from hdmi module) 
    int8_t nib[2]={sources_map[source],0}; 
    uint8_t value=0;
    int8_t success=false;
    for(int i=0; i<HDMI_NO; i++){
      mcp_read(MCP1_ADDRESS, GPIOA, &value); 
      nib[SET]=(int8_t)value&0x0f;
        if(LOGI_SRC) ESP_LOGI(TAG, "hdmi toggle->selected: %02x, set: %02x", nib[SEARCHED], nib[SET]);
        
        //new device plugged during operation (was not on available map)
        //add this device to availbility map and set source to the device
        if(!sources_available[hdmi_nib_to_source(nib[SET])]){ 
            sources_available[hdmi_nib_to_source(nib[SET])]=FOUND;
            success=true; 
            break;
        }
        if(nib[SEARCHED]!=nib[SET]){
             switch_hdmi();
        }
        else{
            success=true;
            break;
        }  
   }
   if(success) 
        return hdmi_nib_to_source(nib[SET]);
   else{
        //not found, although on available map (probably unplugged during operation)
        //remove from availibility map
        sources_available[hdmi_nib_to_source(nib[SEARCHED])]=NOT_FOUND;
        return NOT_FOUND;
    } 
}

int8_t toggle_i2s(int8_t source) {
   //nib[0]->searched (selected) source, nib[1]->set source (read from i2s module)
   int8_t nib[2]={sources_map[source],0};  
   uint8_t value=0;
   for(int i=0; i<I2S_NO; i++){
        mcp_read(MCP1_ADDRESS, GPIOA, &value); 
        nib[SET]=(int8_t)(value>>4)&0x0f;
        if(LOGI_SRC) ESP_LOGI(TAG, "i2s toggle->selected: %02x, set: %02x", nib[SEARCHED], nib[SET]);
        if(nib[SEARCHED]!=nib[SET]){
            switch_i2s();
        }
        else break;
   }
    return i2s_nib_to_source(nib[SET]);
}

void switch_hdmi(){
    mcp_set_pin(MCP1_ADDRESS, GPIOB, HDMI_SWITCH); 
    vTaskDelay(100 / (portTICK_PERIOD_MS));
    mcp_clear_pin(MCP1_ADDRESS, GPIOB, HDMI_SWITCH); 
    vTaskDelay(500 / (portTICK_PERIOD_MS));
}

void switch_i2s(){
    mcp_set_pin(MCP1_ADDRESS, GPIOB, I2S_SWITCH); 
    vTaskDelay(110 / portTICK_PERIOD_MS);
    mcp_clear_pin(MCP1_ADDRESS, GPIOB, I2S_SWITCH); 
    vTaskDelay(10 / (portTICK_PERIOD_MS));
}




