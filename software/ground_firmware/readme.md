# KubiSat Ground Station Firmware

UART ↔ LoRa bridge for the KubiSat satellite. Runs on a Raspberry Pi Pico (RP2040).

The ground station is intentionally a **dumb bridge**: it does not encode/decode SEQ or CRC. Host tools (see `../kubisat_operator`) and the satellite own the application protocol described in [`../commands.md`](../commands.md).

## Features

* **LoRa–UART bridge** — bidirectional relay between a host serial port and the satellite radio
* **Addressing** — wire format `[dest][sender][payload…]` (ground local=`21`, satellite=`37`)
* **Raw protocol frames** — `KBST…TSBK` printed without timestamps; logs use `[Nms] - …`
* **RSSI/SNR** — send a line containing only `?` to append `;rssi;snr` to the next RX frame
* **Watchdog** — 8 s hardware watchdog; LoRa init failure reboots instead of hanging
* **Status LED** — onboard LED blinks on TX/RX; solid on LoRa init fault

## Hardware

| Function | Pico pin |
|----------|----------|
| UART TX (to host) | GP8 |
| UART RX (from host) | GP9 |
| LoRa SPI MISO / MOSI / SCK / CS | GP4 / GP3 / GP2 / GP1 |
| LoRa RESET / DIO0 | GP29 / GP10 |

**Host link is UART1 @ 115200 8N1 on GP8/GP9** (USB CDC stdio is disabled). Use a USB–UART adapter or the Pico UART pins, not the USB serial console.

Pins and addresses: [`pin_config.h`](pin_config.h). Shared LoRa driver + sync word: [`../common/`](../common/).

## Build

With the [Pico SDK](https://github.com/raspberrypi/pico-sdk) (2.0.0):

```sh
export PICO_SDK_PATH=/path/to/pico-sdk
cmake -S . -B build
cmake --build build
# UF2 also copied to release/main.uf2
```

Or use VS Code with the Raspberry Pi Pico extension (import this folder).

## Flash and use

1. Hold `BOOTSEL`, plug in USB, copy `release/main.uf2` to `RPI-RP2`.
2. Open serial on the UART adapter at **115200**.
3. Send a full protocol frame ending with newline; replies appear as raw `KBST…TSBK`.
4. Send `?` alone to arm RSSI/SNR on the next received frame.

Status notes: [`STATUS.md`](STATUS.md).

## License

MIT — see the repository root [`LICENSE`](../../LICENSE).
