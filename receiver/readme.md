# Receiver Schematics
![Receiver Audio Path Schematic](../receiver/receiver_audio_path.png)
![Receiver User Interface Schematic](../receiver/receiver_interface_control_signals.png)
![Receiver Power Supply Schematic](../receiver/receiver_power_supply_schematics.png)


# Receiver Components

### Power Supply
- **Switching Power Supply (60W, 12V)**
  - No-Name, taken from notebook
  - [Photo](../receiver/No_Name_switching_power_supply.jpg)

### Converters & Stabilizers
- **Step-Down Converter Modules (2x)**
  - No-Name, based on TPS5430 chip (Texas Instruments)
  - [Datasheet](../receiver/tps5430.pdf)
  - ![Photo](../receiver/tps5430_module1.jpg)
  - ![Photo](../receiver/tps5430_module2.jpg)
- **Low-Noise Linear Stabilizer Module (+5V, 500mA)**
  - No-Name, based on 2x LT3045 chip (Analog Devices)
  - [Datasheet](../receiver/lt3045_stabilizer.pdf)
- **Low-Noise Linear Stabilizer Module (+5V and -5V, 300mA)**
  - No-Name, based on LT3045 and LT3094 chips (Analog Devices)
  - [Datasheet](../receiver/lt3045_lt3094_stabilizer.pdf)

### Microcontroller & Extenders
- **Microcontroller Module**
  - SoC ESP32 WROVER DEV KIT (Espressif Systems)
  - [Datasheet](../receiver/esp32_wrover.pdf)
- **I2C Bus Extender Module**
  - MCP23017 (Waveshare 15391)
  - [Datasheet](../receiver/mcp23017.pdf)

### Displays & Controllers
- **LCD Display Module**
  - LCD1602 (No-Name)
  - [Datasheet](../receiver/lcd1602.pdf)
- **LCD Controller Module (via I2C)**
  - Based on PCF8574 (Philips Semiconductors)
  - [Datasheet](../receiver/pcf8574.pdf)

### Audio Components
- **DAC Module**
  - Based on PCM5102 chip (Burr-Brown)
  - [Datasheet](../receiver/pcm5102.pdf)
- **Analog Audio Control Module**
  - Based on NJW1194 chip (New Japan Radio)
  - [Datasheet](../receiver/njw1194.pdf)
- **Bluetooth 5.0 Stereo Module**
  - Based on QCC5125 chip (Qualcomm)
  - [Datasheet](../receiver/qcc5125.pdf)
- **Digital Signal Converter (SPDIF to I2S)**
  - Based on DIR9001 chip (Texas Instruments)
  - [Datasheet](../receiver/dir9001.pdf)
- **I2S Signal Switch Module**
  - Based on HC245 chip (ONSemi)
  - [Datasheet](../receiver/hc245.pdf)

### HDMI & Logic Converters
- **HDMI Source and Output Control Module**
  - No-Name, based on LT8641SXE chip (Lontium Semiconductors)
  - [Datasheet](../receiver/lt8641sxe.pdf)
- **5V-3.3V Logic Level Converter Module**
  - No-Name
  - [Datasheet](../receiver/logic_level_converter.pdf)

### Controls & Interfaces
- **Control Module with Encoders**
  - Two PEC11H series encoders and switch (Bourns)
  - [Datasheet](../receiver/pec11h.pdf)
- **Inverter Module with Schmitt Circuit**
  - Based on 74HC14 integrated circuit (Diodes Incorporated)
  - [Datasheet](../receiver/74hc14.pdf)

### Miscellaneous
- **Other Components**
  - NPN transistors, resistors, capacitors, universal PCBs, wiring, RCA connectors, USB, Bluetooth antennas, heat sink
  - [Various Components](../receiver/miscellaneous_components.pdf)
- **Enclosure**
  - See [enclosure directory](../enclosure) for STL files


