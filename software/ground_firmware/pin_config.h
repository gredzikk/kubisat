// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#include <stdint.h>

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
#define LORA_DEFAULT_DIO0_PIN      10

// SX127x PA select constants (not RP2040 GPIO numbers)
#define PA_OUTPUT_RFO_PIN          0
#define PA_OUTPUT_PA_BOOST_PIN     1

inline constexpr int csPin = 1;      // LoRa radio chip select
inline constexpr int resetPin = 29;  // LoRa radio reset
inline constexpr int irqPin = 10;    // LoRa DIO0

// Ground station addresses (mirror of satellite: local=37, remote=21)
inline constexpr uint8_t lora_address_local = 21;  // this device
inline constexpr uint8_t lora_address_remote = 37; // satellite

#endif // PIN_CONFIG_H
