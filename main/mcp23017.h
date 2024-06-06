#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <esp_err.h>
#include "esp_log.h"
#include "i2c.h"
#ifndef _mcp_parameters_H_
#define _mcp_parameters_H_
static const uint8_t LOGE_MCP = true;
static const uint8_t LOGI_MCP = false;
static char* const MCP_TAG = "MCP23017";
// Registers A
static const char IODIRA = 0x00;
static const char IPOLA = 0x02;
static const char GPINTENA = 0x04;
static const char DEFVALA = 0x06;
static const char INTCONA = 0x08;
static const char IOCONA = 0x0A;
static const char GPPUA = 0x0C;
static const char INTFA = 0x0E;
static const char INTCAPA = 0x10;
static const char GPIOA = 0x12;
static const char OLATA = 0x14;

// Registers B
static const char IODIRB = 0x01;
static const char IPOLB = 0x03;
static const char GPINTENB = 0x05;
static const char DEFVALB = 0x07;
static const char INTCONB = 0x09;
static const char IOCONB = 0x0B;
static const char GPPUB = 0x0D;
static const char INTFB = 0x0F;
static const char INTCAPB = 0x11;
static const char GPIOB = 0x13;
static const char OLATB = 0x15;

// PINs GPIOA
static const uint8_t PA0 = 0;
static const uint8_t PA1 = 1;
static const uint8_t PA2 = 2;
static const uint8_t PA3 = 3;
static const uint8_t PA4 = 4;
static const uint8_t PA5 = 5;
static const uint8_t PA6 = 6;
static const uint8_t PA7 = 7;

// PINs GPIOB
static const uint8_t PB0 = 0;
static const uint8_t PB1 = 1;
static const uint8_t PB2 = 2;
static const uint8_t PB3 = 3;
static const uint8_t PB4 = 4;
static const uint8_t PB5 = 5;
static const uint8_t PB6 = 6;
static const uint8_t PB7 = 7;

esp_err_t McpInit(uint8_t address);
esp_err_t McpWrite(uint8_t address, uint8_t reg, uint8_t size, uint8_t *data);
esp_err_t McpRead(uint8_t address, uint8_t reg, uint8_t *data);
esp_err_t McpSetPin(uint8_t address, uint8_t reg, uint8_t pin);
esp_err_t McpClearPin(uint8_t address, uint8_t reg, uint8_t pin);
#endif