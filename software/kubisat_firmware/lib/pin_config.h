// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#include <stdint.h>

// DEBUG uart
#define DEBUG_UART_PORT uart0
#define DEBUG_UART_BAUD_RATE 115200

#define DEBUG_UART_TX_PIN 0
#define DEBUG_UART_RX_PIN 1

#define MAIN_I2C_PORT i2c1
#define MAIN_I2C_SDA_PIN 6
#define MAIN_I2C_SCL_PIN 7

// GPS configuration
#define GPS_UART_PORT uart1
#define GPS_UART_BAUD_RATE 9600
#define GPS_UART_TX_PIN 8
#define GPS_UART_RX_PIN 9
#define GPS_POWER_ENABLE_PIN 14

#define SENSORS_POWER_ENABLE_PIN 15
#define SENSORS_I2C_PORT i2c0
#define SENSORS_I2C_SDA_PIN 4
#define SENSORS_I2C_SCL_PIN 5

#define BUFFER_SIZE 85

// SPI configuration for SD card
#define SD_SPI_PORT spi1
#define SD_MISO_PIN 12
#define SD_MOSI_PIN 11
#define SD_SCK_PIN 10
#define SD_CS_PIN 13
// Optional card-detect net (unused by storage unless enabled in fs_init).
// Do not reuse this pin for LoRa IRQ — DIO0 is on GP20.
#define SD_CARD_DETECT_PIN 28

#define SX1278_MISO 16
#define SX1278_CS 17
#define SX1278_SCK 18
#define SX1278_MOSI 19

#define SPI_PORT spi0
#define READ_BIT 0x80

#define LORA_DEFAULT_SPI spi0
#define LORA_DEFAULT_SPI_FREQUENCY 8E6
#define LORA_DEFAULT_SS_PIN 17
#define LORA_DEFAULT_RESET_PIN 22
#define LORA_DEFAULT_DIO0_PIN 20

// SX127x PA select constants (not RP2040 GPIO numbers)
#define PA_OUTPUT_RFO_PIN 0
#define PA_OUTPUT_PA_BOOST_PIN 1

inline constexpr int lora_cs_pin = 17;    // LoRa radio chip select
inline constexpr int lora_reset_pin = 22; // LoRa radio reset
inline constexpr int lora_irq_pin = 20;   // LoRa DIO0 (must match LORA_DEFAULT_DIO0_PIN / schematic)

inline constexpr int lora_address_local = 37;  // address of this device
inline constexpr int lora_address_remote = 21; // destination to send to

#endif
