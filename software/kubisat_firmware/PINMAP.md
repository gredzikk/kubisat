# KubiSat satellite pin map (RP2040)

Board-safe reference for [`lib/pin_config.h`](lib/pin_config.h). Do not change pins without checking the `hardware/kubisat_main` schematic — boards cannot be rewired in the field.

| MCU pin | Function | Notes |
|--------:|----------|--------|
| GP0 | Debug UART TX | `uart0` |
| GP1 | Debug UART RX | `uart0` |
| GP4 | Sensors I2C SDA | `i2c0` (BME280, BH1750, MPU-6050) |
| GP5 | Sensors I2C SCL | `i2c0` |
| GP6 | Main I2C SDA | `i2c1` (DS3231, INA3221) |
| GP7 | Main I2C SCL | `i2c1` |
| GP8 | GPS UART TX | `uart1` |
| GP9 | GPS UART RX | `uart1` |
| GP10 | SD SPI SCK | `spi1` |
| GP11 | SD SPI MOSI | `spi1` |
| GP12 | SD SPI MISO | `spi1` |
| GP13 | SD SPI CS | |
| GP14 | GPS power enable | |
| GP15 | Sensors power enable | |
| GP16 | SX1278 MISO | `spi0` |
| GP17 | SX1278 CS / NSS | |
| GP18 | SX1278 SCK | `spi0` |
| GP19 | SX1278 MOSI | `spi0` |
| GP20 | SX1278 DIO0 (IRQ) | Must match `lora_irq_pin` |
| GP22 | SX1278 RESET | |
| GP28 | SD card detect (optional) | Not used for LoRa |

LoRa addresses: local satellite `37`, remote ground `21`.
