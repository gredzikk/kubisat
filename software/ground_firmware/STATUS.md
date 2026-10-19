# Ground Firmware — Status

**Date:** 2026-10-19  
**Build:** `release/main.uf2` (Release, Pico SDK 2.0.0)  
**Shared code:** `software/common` (LoRa, frame markers, CRC, bridge helpers, sync word `0x4B`)

---

## Current state

Ground station is a UART ↔ LoRa bridge on RP2040 (Pico), talking to KubiSat at 433 MHz.

### Done

| Area | What |
|------|------|
| **RX correctness** | Reject packets `< 2` address bytes; `[dest, sender]` checks; extract `KBST…TSBK`; strip trailing NULs |
| **UART bridge** | `uart_print` = timestamped logs; `uart_write_raw` = protocol frames |
| **TX / radio** | Check `beginPacket` / `endPacket`; payload size cap; `onTxDone` → continuous RX |
| **RX path** | `onReceive` IRQ + poll fallback; `parse_packet` no longer forces `RX_SINGLE` over continuous RX |
| **RSSI** | Arm with a line containing only `?`; append `;rssi;snr` after the frame |
| **Watchdog** | 8 s, kicked in main loop; LoRa init failure reboots instead of hanging |
| **Status LED** | Blink on TX/RX; solid on LoRa init fault |
| **Config** | `lora_address_local=21` / `lora_address_remote=37`; shared sync word `0x4B` |
| **Shared code** | LoRa + `frame_markers.h` + `bridge_frame.h` + `lora_defaults.h` |
| **Host tests** | Address filter / extract / RSSI append in `software/tests/host` |
| **Docs** | `readme.md` synced (UART GP8/GP9, not USB CDC) |

### Host-facing behavior

- **Frames** print as raw `KBST;…;TSBK` (optional `;rssi;snr`).
- **Logs** still look like `[1234ms] - …`.
- **RSSI:** send `?` alone + Enter; next valid RX frame gets `;rssi;snr` appended.

### Layout

```
software/
  common/
    LoRa/                 # shared SX127x driver
    protocol/
      frame_markers.h
      bridge_frame.h
      lora_defaults.h     # sync word + frequency
      crc16.*             # sat + host tests (not required on ground bridge)
  ground_firmware/
    main.cpp
    pin_config.h
    utils.{h,cpp}
    release/main.uf2
```

---

## Hardware smoke (PENDING — needs boards)

1. Flash ground + sat UF2s with matching sync word `0x4B`.
2. TX frame from operator (SEQ+CRC) → sat ACK/VAL → raw UART frame.
3. Arm `?` and confirm `;rssi;snr` on next RX.

---

## Backlog

9. App-level ACK / retry on ground — not needed; host owns SEQ/ACK.
10. Dangerous-command gate at bridge (optional BOOTSEL / FORMAT confirm).
11. Further docs / Doxygen PDF naming cleanup.

---

## Quick verify

```sh
export PICO_SDK_PATH=/path/to/pico-sdk
cmake -S software/ground_firmware -B build-gs
cmake --build build-gs
# Flash release/main.uf2 via BOOTSEL
# Serial 115200 on UART1 (GP8 TX / GP9 RX)
```
