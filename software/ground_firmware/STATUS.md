# Ground Firmware — Status

**Date:** 2026-10-19  
**Build:** `release/main.uf2` builds clean (Release, Pico SDK 2.0.0)  
**Branch state:** uncommitted local changes under `software/ground_firmware` + new `software/common`

---

## Current state

Ground station is a UART ↔ LoRa bridge on RP2040 (Pico), talking to KubiSat at 433 MHz.

### Done in this pass

| Area | What changed |
|------|----------------|
| **RX correctness** | Reject packets `< 2` address bytes; `[dest, sender]` checks aligned with satellite; extract `KBST…TSBK` instead of dumping raw payload; strip trailing NULs safely |
| **UART bridge** | `uart_print` = timestamped logs; `uart_write_raw` = protocol frames (no timestamp wrapping) |
| **TX / radio** | Check `beginPacket` / `endPacket`; payload size cap; no VLA; `onTxDone` → continuous RX; explicit `receive(0)` after TX |
| **RSSI** | Arm with a line containing only `?` (no longer strips `?` from normal payloads); append `;rssi;snr` after the frame |
| **Watchdog** | 8 s, kicked in main loop; LoRa init failure reboots instead of hanging |
| **Config** | `lora_address_local=21` / `lora_address_remote=37` in `pin_config.h`; fixed `PA_OUTPUT_*` (SX127x selects, not GPIOs); removed `pin_config.cpp` |
| **Shared code** | LoRa driver moved to `software/common/LoRa`; frame markers in `software/common/protocol/frame_markers.h` |
| **Build** | Exceptions off; binary always built (no TinyUSB gate); links `hardware_watchdog` |

### Host-facing behavior (important)

- **Frames** print as raw `KBST;…;TSBK` (optional `;rssi;snr`).
- **Logs** still look like `[1234ms] - …`.
- **RSSI:** send `?` alone + Enter; next valid RX frame gets `;rssi;snr` appended.

### Layout

```
software/
  common/
    LoRa/                 # shared SX127x driver
    protocol/
      frame_markers.h     # KBST / TSBK / size limits
      crc16.*             # present; not wired into ground yet
  ground_firmware/
    main.cpp              # bridge loop
    pin_config.h
    utils.{h,cpp}
    release/main.uf2
```

---

## Next steps

### High priority

1. **Point satellite at `software/common/LoRa`** — drop duplicate driver under `kubisat_firmware`; keep modem settings in lockstep.
2. **Adopt `frame_markers.h` on the satellite** — replace local `FRAME_BEGIN` / `FRAME_END` strings so markers cannot drift.
3. **Flash + over-air smoke test** — TX frame from host → sat reply → raw UART frame; arm `?` and confirm `;rssi;snr`.
4. **Host tooling** — update any parsers that still expect `[Nms] -` on protocol lines.

### Medium priority

5. **Non-default LoRa sync word** — change from `0x12` on both ends (weak network isolation).
6. **Wire or drop `crc16`** — either append/check CRC on frames, or remove unused common code until needed.
7. **Continuous RX via IRQ** — prefer `onReceive` over poll + `sleep_ms(1)` to cut miss risk further.
8. **Status LED** — init / TX / RX / fault for field use.

### Lower priority / backlog

9. **App-level ACK / retry** — `REPLY_TIMEOUT` was removed as dead code; reintroduce only with a real seq/ACK design.
10. **Dangerous-command gate** — optionally block or confirm sat bootloader / destructive SETs at the ground bridge.
11. **Host unit tests** — address filter, frame extract, RSSI append, UART overflow (no hardware required).
12. **Docs cleanup** — licence path, `doc.pdf` location, keep `readme.md` in sync with UART GP8/GP9.

---

## Known gaps (not fixed yet)

- No shared protocol module beyond frame markers (no frame encode/decode on ground).
- LoRa sync word still default `0x12`.
- Satellite still has its own LoRa tree / protocol constants.
- No automated tests.
- RF remains cleartext / unauthenticated (document as trusted environment until sync word + optional allowlist).

---

## Quick verify

```sh
cd software/ground_firmware/build
cmake .. && ninja
# Flash release/main.uf2 via BOOTSEL
# Serial 115200 on UART1 (GP8 TX / GP9 RX)
```
