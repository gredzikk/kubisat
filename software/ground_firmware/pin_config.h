// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include <stdint.h>

#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#define DEBUG_UART_PORT uart1
#define BAUD_RATE 115200
#define UART_TX_PIN 8
#define UART_RX_PIN 9

#define PIN_MISO 4
#define PIN_CS   1
#define PIN_SCK  2
#define PIN_MOSI 3

#define SPI_PORT spi0
#define READ_BIT 0x80

#define LORA_DEFAULT_SPI           spi0
#define LORA_DEFAULT_SPI_FREQUENCY 8E6
#define LORA_DEFAULT_SS_PIN        1
#define LORA_DEFAULT_RESET_PIN     29
#define LORA_DEFAULT_DIO0_PIN      0

#define PA_OUTPUT_RFO_PIN          11
#define PA_OUTPUT_PA_BOOST_PIN     12

// LoRa constants - declare as extern
extern const int csPin;      // LoRa radio chip select
extern const int resetPin;   // LoRa radio reset  
extern const int irqPin;     // LoRa hardware interrupt pin
extern uint8_t localAddress; // address of this device
extern uint8_t destination;  // destination to send to


#endif // PIN_CONFIG_H