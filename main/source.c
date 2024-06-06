#include "source.h"

static const char *TAG = "Source";
static uint8_t sourcesMap[MAX_SOURCES] = {NOT_FOUND};
static uint8_t sourcesAvailable[MAX_SOURCES] = {NOT_FOUND};

void SourcesInit()
{
    MapSources();
    SetAvailibility();
}

void MapSources()
{
    sourcesMap[SOURCE0] = INPUT_1;
    sourcesMap[SOURCE1] = (HDMI1 & 0x0f);       // value of HDMI_nib (lo_nib)
    sourcesMap[SOURCE2] = (HDMI2 & 0x0f);       // value of HDMI_nib (lo_nib)
    sourcesMap[SOURCE3] = (HDMI3 & 0x0f);       // value of HDMI_nib (lo_nib)
    sourcesMap[SOURCE4] = (HDMI4 & 0x0f);       // value of HDMI_nib (lo_nib)
    sourcesMap[SOURCE5] = ((I2S1 >> 4) & 0x0f); // value of i2s_nib (hi_nib)
    sourcesMap[SOURCE6] = ((I2S2 >> 4) & 0x0f); // value of i2s_nib (hi_nib)
    sourcesMap[SOURCE7] = ((I2S3 >> 4) & 0x0f); // value of i2s_nib (hi_nib)
    sourcesMap[SOURCE8] = ((I2S4 >> 4) & 0x0f); // value of i2s_nib (hi_nib)
}

void SetAvailibility()
{
    sourcesAvailable[SOURCE0] = FOUND;
    for (int8_t i = SOURCE1; i <= SOURCE4; i++)
    { // sources 1-4
        int8_t source = IsHdmiAvailable();
        sourcesAvailable[source] = FOUND;
    }
    for (int8_t i = SOURCE6; i <= SOURCE7; i++)
    { // sources 6-7, 5 and 8 are used for hdmi and radio output
        sourcesAvailable[i] = FOUND;
    }
}

int8_t IsHdmiAvailable()
{
    uint8_t value = 0;
    SwitchHdmi();
    McpRead(MCP1_ADDRESS, GPIOA, &value);
    int8_t hdmiNib = (int8_t)value & 0x0f;
    return HdmiNibToSource(hdmiNib);
}

int8_t HdmiNibToSource(int8_t nibValue)
{
    for (int8_t i = SOURCE1; i <= SOURCE4; i++)
    {
        if (nibValue == sourcesMap[i])
            return i;
    }
    return 0;
}

int8_t I2sNibToSource(int8_t nibValue)
{
    for (int8_t i = SOURCE5; i <= SOURCE8; i++)
    {
        if (nibValue == sourcesMap[i])
            return i;
    }
    return 0;
}

void SetSource(int8_t (*tunerState)[VAR_NO])
{
    /*
        Checking if source is available. If not-> select next  or previous depending
        on toogle direction.
    */
    while (!sourcesAvailable[tunerState[SOURCE][ACT_VAL]])
    {
        if (tunerState[SOURCE][LAS_VAL] <= tunerState[SOURCE][ACT_VAL])
        {
            tunerState[SOURCE][LAS_VAL] = tunerState[SOURCE][ACT_VAL];
            tunerState[SOURCE][ACT_VAL]++;
        }
        else
        {
            tunerState[SOURCE][LAS_VAL] = tunerState[SOURCE][ACT_VAL];
            tunerState[SOURCE][ACT_VAL]--;
        }
        if (tunerState[SOURCE][ACT_VAL] < SOURCE0)
        {
            tunerState[SOURCE][ACT_VAL] = MAX_SOURCES - 1;
            tunerState[SOURCE][LAS_VAL] = MAX_SOURCES - 1;
        }
        if (tunerState[SOURCE][ACT_VAL] >= MAX_SOURCES)
        {
            tunerState[SOURCE][ACT_VAL] = SOURCE0;
            tunerState[SOURCE][LAS_VAL] = SOURCE0;
        }
    }
    /*Change to available source*/
    int8_t changeResponse = ChangeSource(tunerState[SOURCE][ACT_VAL]);
    if (changeResponse != tunerState[SOURCE][ACT_VAL])
        tunerState[SOURCE][ACT_VAL] = changeResponse;
}

int8_t ChangeSource(int8_t source)
{
    if (LOGI_SRC)
        ESP_LOGI(TAG, "Change source: %d", source);
    switch (source)
    {
    case SOURCE0: // analog
        SetAnalogInput(INPUT_1);
        return source;
    case SOURCE1 ... SOURCE4: // hdmi sources
        int8_t response = ToggleHdmi(source);
        if (response == NOT_FOUND)
        {
            // change to i2s source if no hdmi source found
            ToggleI2S(SOURCE6);
            SetAnalogInput(INPUT_2);
            return SOURCE6;
        }
        else
        {
            ToggleI2S(SOURCE5);
            SetAnalogInput(INPUT_2);
            return response;
        }
    case SOURCE6 ... SOURCE7: // i2s sources, remark: SOURCE5 and 8 are used for hdmi and radio output
        ToggleI2S(source);
        SetAnalogInput(INPUT_2);
        return source;
    }
    return source;
}

int8_t ToggleHdmi(int8_t source)
{
    // nib[0]->searched(selected) source, nib[1]->set source (read from hdmi module)
    int8_t nib[2] = {sourcesMap[source], 0};
    uint8_t value = 0;
    int8_t success = false;
    for (int i = 0; i < HDMI_NO; i++)
    {
        McpRead(MCP1_ADDRESS, GPIOA, &value);
        nib[SET] = (int8_t)value & 0x0f;
        if (LOGI_SRC)
            ESP_LOGI(TAG, "hdmi toggle->selected: %02x, set: %02x", nib[SEARCHED], nib[SET]);

        // new device plugged during operation (was not on available map)
        // add this device to availbility map and set source to the device
        if (!sourcesAvailable[HdmiNibToSource(nib[SET])])
        {
            sourcesAvailable[HdmiNibToSource(nib[SET])] = FOUND;
            success = true;
            break;
        }
        if (nib[SEARCHED] != nib[SET])
        {
            SwitchHdmi();
        }
        else
        {
            success = true;
            break;
        }
    }
    if (success)
        return HdmiNibToSource(nib[SET]);
    else
    {
        // not found, although on available map (probably unplugged during operation)
        // remove from availibility map
        sourcesAvailable[HdmiNibToSource(nib[SEARCHED])] = NOT_FOUND;
        return NOT_FOUND;
    }
}

int8_t ToggleI2S(int8_t source)
{
    // nib[0]->searched (selected) source, nib[1]->set source (read from i2s module)
    int8_t nib[2] = {sourcesMap[source], 0};
    uint8_t value = 0;
    for (int i = 0; i < I2S_NO; i++)
    {
        McpRead(MCP1_ADDRESS, GPIOA, &value);
        nib[SET] = (int8_t)(value >> 4) & 0x0f;
        if (LOGI_SRC)
            ESP_LOGI(TAG, "i2s toggle->selected: %02x, set: %02x", nib[SEARCHED], nib[SET]);
        if (nib[SEARCHED] != nib[SET])
        {
            SwitchI2s();
        }
        else
            break;
    }
    return I2sNibToSource(nib[SET]);
}

void SwitchHdmi()
{
    McpSetPin(MCP1_ADDRESS, GPIOB, HDMI_SWITCH);
    vTaskDelay(100 / (portTICK_PERIOD_MS));
    McpClearPin(MCP1_ADDRESS, GPIOB, HDMI_SWITCH);
    vTaskDelay(500 / (portTICK_PERIOD_MS));
}

void SwitchI2s()
{
    McpSetPin(MCP1_ADDRESS, GPIOB, I2S_SWITCH);
    vTaskDelay(110 / portTICK_PERIOD_MS);
    McpClearPin(MCP1_ADDRESS, GPIOB, I2S_SWITCH);
    vTaskDelay(10 / (portTICK_PERIOD_MS));
}
