# KubiSat Firmware Status

Snapshot of satellite + ground firmware after the reliability/features roadmap.  
Last updated: 2026-10-19.

## Scope

| Tree | Role |
|------|------|
| [`kubisat_firmware/`](kubisat_firmware/) | Satellite (RP2040 + SX1278 + sensors + SD) |
| [`ground_firmware/`](ground_firmware/) | LoRa ↔ UART bridge |
| [`common/`](common/) | Shared LoRa driver + CRC16 |
| [`commands.md`](commands.md) | Command / frame protocol reference |

---

## Current state (implemented)

### Correctness / build

- [x] LoRa DIO0 pin set to **GP20** (`lora_irq_pin` aligned with `LORA_DEFAULT_DIO0_PIN`)
- [x] Pin map documented: [`kubisat_firmware/PINMAP.md`](kubisat_firmware/PINMAP.md)
- [x] Single shared LoRa library under [`common/LoRa/`](common/LoRa/)
- [x] C++ exceptions / RTTI disabled for sat firmware
- [x] Program version tied to build number; README URL fixes; Linux-friendly `make doc`
- [x] Both firmwares build to `main.uf2` (local verify: Pico SDK 2.0.0)

### Reliability

- [x] Hardware watchdog armed (~8 s); `WATCHDOG_RESET` emitted on watchdog reboot
- [x] Watchdog fed from core0 main loop and core1 telemetry loop
- [x] Periodic health beacon (~30 s) on LoRa (`GET/VAL` group **1.4**)
- [x] SD mount is non-destructive (no auto-format on boot)
- [x] Explicit SD format command: `SET;1;5;FORMAT` (lab only)
- [x] Text protocol with `SEQ` + CRC-16/CCITT + `ACK` type  
  Frame shape: `KBST;ADDR;TYPE;GROUP;CMD;PAYLOAD;UNIT;SEQ;CRC16;TSBK`  
  LoRa RX requires integrity; UART still accepts legacy frames
- [x] Health / unsolicited beacon TX retries with ACK wait (up to 3×)
- [x] Radio init retries; system continues if radio or SD fails (beacon reports flags)
- [x] Sensor init retries (light / env / IMU)

### GPS / sensors / features

- [x] NMEA `*xx` checksum validation; `$GNRMC` / `$GNGGA` accepted
- [x] GPS lock / loss events emitted
- [x] GPS power duty-cycle in battery mode
- [x] MPU-6050 driver + commands group **2** (`2.0` status, `2.1` accel, `2.2` gyro)
- [x] IMU columns in sensor CSV / last-sensor record
- [x] Recent telemetry dump command **8.4**

### Quality / docs / CI

- [x] Host CRC/frame tests: [`tests/host/`](tests/host/)
- [x] GitHub Actions: [`.github/workflows/firmware.yml`](../.github/workflows/firmware.yml) (host tests + both UF2 builds)
- [x] Protocol + event IDs: [`commands.md`](commands.md)
- [x] Operator checklist: [`kubisat_firmware/FLIGHT_CHECKLIST.md`](kubisat_firmware/FLIGHT_CHECKLIST.md)
- [x] Ground “secure handshake” comment removed (bridge only)

### Command map (handlers present)

| Group | Commands |
|------:|----------|
| 1 | 0 list, 1 build, 2 power mode, 3 uptime, 4 health, 5 format SD, 8 verbosity, 9 BOOTSEL |
| 2 | 0 IMU status, 1 accel, 2 gyro |
| 3 | 0 time, 1 timezone, 4 RTC temp |
| 5 | 1 last events, 2 event count |
| 7 | 1 GPS power, 2 UART passthrough |
| 8 | 2 last telem, 3 last sensors, 4 recent N telem |

---

## Planned future fixes / follow-ups

These were deferred by the roadmap or need hardware/ops work after flash testing.

### Must validate on hardware (before flight)

1. **Confirm DIO0 = GP20** with continuity / working board (silk + defaults assumed; no rewiring possible).
2. **Smoke-test** new CRC frames end-to-end through ground bridge (host must send `SEQ;CRC`; ACK replies).
3. **Exercise SD format once** in lab (`1.5`), then confirm boot never wipes a card with existing logs.
4. **Watchdog**: force a hang and confirm reboot + `WATCHDOG_RESET` in event log.
5. **IMU**: confirm MPU-6050 address/WHO_AM_I on the assembled sensor board.

### Comms / protocol hardening

| Item | Notes |
|------|--------|
| Host CLI / script with SEQ+CRC+retry | Sat replies include SEQ; ground stays dumb bridge — tooling still needed |
| Auto-ACK from host for health beacon | Beacon uses `require_ack`; without host ACK, sat spends ~2 s retrying every 30 s |
| Drop UART legacy frames | After tools migrate, require CRC on UART too |
| Command-response sat-side ACK retry | Currently replies are fire-and-forget with SEQ; host should retry |
| Optional app-layer anti-replay | SEQ window / reject duplicates (not started) |

### Storage / telemetry

| Item | Notes |
|------|--------|
| Optional use of `SD_CARD_DETECT_PIN` | Defined but unused — wire into `fs_init` or remove |
| Append-only telemetry on boot | Telemetry CSV still truncated/recreated on init when SD mounts — prefer append + rotate |
| On-demand SD file dump over LoRa | `8.4` is RAM buffer only; full SD download not implemented |
| Compressed / binary beacon | Summary beacon is text today |

### Platform / architecture (out of current roadmap)

| Item | Notes |
|------|--------|
| True OTA (dual-slot / UF2 over LoRa) | Remote BOOTSEL only for now |
| FreeRTOS / message queues | Intentionally deferred; stay multicore + mutexes |
| Binary radio framing | Deferred; keep text `KBST…TSBK` |
| Crypto / authenticated commands | Not planned; remove any remaining “secure” wording if found |
| PCB respin | Out of scope (pin_config only) |

### CI / quality gaps

| Item | Notes |
|------|--------|
| Expand host tests | Decode/encode of full firmware `frame.cpp`; command dispatch table check |
| CI cache for Pico SDK | Workflow clones SDK each run |
| `-Werror` on first-party code | Not enabled yet |
| Generate `commands.md` from handler map | Docs are hand-synced to [`commands.cpp`](kubisat_firmware/lib/comms/commands/commands.cpp) |
| HIL / on-target smoke job | None |

### Smaller cleanups

- Deduplicate Pico SDK object compilation across static libs (link `pico_stdlib` privately where possible) to shrink build time.
- Align ground `documentation.cmake` / Doxyfile with sat if PDF docs are still required.
- Release UF2 naming vs configure-time build number (copy can lag PRE_BUILD increment by one).
- Magnetometer (HMC5883L) on sensor board schematic — no driver yet.

---

## Quick operator pointers

1. Protocol details: [`commands.md`](commands.md)  
2. Pins: [`kubisat_firmware/PINMAP.md`](kubisat_firmware/PINMAP.md)  
3. Pre-flight steps: [`kubisat_firmware/FLIGHT_CHECKLIST.md`](kubisat_firmware/FLIGHT_CHECKLIST.md)  
4. Host CRC test: `cmake -S software/tests/host -B build-host-tests && cmake --build build-host-tests && ctest --test-dir build-host-tests`
