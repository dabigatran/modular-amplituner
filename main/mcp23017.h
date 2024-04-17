#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <esp_err.h>
#include "esp_log.h"
#include "i2c.h"

//Registers A
#define IODIRA	    0x00
#define IPOLA 		0x02
#define GPINTENA 	0x04
#define DEFVALA 	0x06
#define INTCONA 	0x08
#define IOCONA 	    0x0A
#define GPPUA 		0x0C
#define INTFA 		0x0E
#define INTCAPA 	0x10
#define GPIOA 		0x12
#define OLATA 		0x14

//Registers B
#define IODIRB 	    0x01
#define IPOLB 		0x03
#define GPINTENB 	0x05
#define DEFVALB 	0x07
#define INTCONB 	0x09
#define IOCONB 	    0x0B
#define GPPUB 		0x0D
#define INTFB 		0x0F
#define INTCAPB 	0x11
#define GPIOB 		0x13
#define OLATB 		0x15

//PINs GPIOA
#define PA0 (uint8_t) 0x00
#define PA1 (uint8_t) 0x01
#define PA2 (uint8_t) 0x02
#define PA3 (uint8_t) 0x03
#define PA4 (uint8_t) 0x04
#define PA5 (uint8_t) 0x05
#define PA6 (uint8_t) 0x06
#define PA7 (uint8_t) 0x07

//PINs GPIOB
#define PB0 (uint8_t) 0x00
#define PB1 (uint8_t) 0x01
#define PB2 (uint8_t) 0x02
#define PB3 (uint8_t) 0x03
#define PB4 (uint8_t) 0x04
#define PB5 (uint8_t) 0x05
#define PB6 (uint8_t) 0x06
#define PB7 (uint8_t) 0x07

#define LOGE_MCP    (uint8_t)0x1
#define LOGI_MCP    (uint8_t)0x0

esp_err_t mcp_init(uint8_t address);
esp_err_t mcp_write(uint8_t address, uint8_t reg, uint8_t size, uint8_t *data); 
esp_err_t mcp_read(uint8_t address, uint8_t reg, uint8_t *data);
esp_err_t mcp_set_pin(uint8_t address,  uint8_t reg, uint8_t pin);
esp_err_t mcp_clear_pin(uint8_t address,  uint8_t reg, uint8_t pin);