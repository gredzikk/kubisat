# KubiSat Firmware Status

Snapshot of satellite + ground firmware after the reliability/features roadmap.  
Last updated: 2026-10-19 (improvement plan implementation).

## Scope

| Tree | Role |
|------|------|
| [`kubisat_firmware/`](kubisat_firmware/) | Satellite (RP2040 + SX1278 + sensors + SD) |
| [`ground_firmware/`](ground_firmware/) | LoRa ↔ UART bridge |
| [`common/`](common/) | Shared LoRa driver + CRC16 + frame markers + sync word |
| [`kubisat_operator/`](kubisat_operator/) | Host GUI (nested git repo) — SEQ+CRC+retry |
| [`commands.md`](commands.md) | Command / frame protocol reference |

---

## Current state (implemented)

### Correctness / build

- [x] LoRa DIO0 pin set to **GP20** (`lora_irq_pin` aligned with `LORA_DEFAULT_DIO0_PIN`)
- [x] Pin map documented: [`kubisat_firmware/PINMAP.md`](kubisat_firmware/PINMAP.md)
- [x] Single shared LoRa library under [`common/LoRa/`](common/LoRa/)
- [x] Shared `frame_markers.h` + `lora_defaults.h` (sync word `0x4B`)
- [x] C++ exceptions / RTTI disabled for sat firmware
- [x] Program version tied to build number; README URL fixes; Linux-friendly `make doc`
- [x] Both firmwares build to `main.uf2` (local verify: Pico SDK 2.0.0)

### Reliability

- [x] Hardware watchdog armed (~8 s); `WATCHDOG_RESET` emitted on watchdog reboot
- [x] Watchdog fed from core0 main loop and core1 telemetry loop
- [x] Periodic health beacon (~30 s) on LoRa (`GET/VAL` group **1.4**) — fire-and-forget (host may still ACK)
- [x] SD mount is non-destructive (no auto-format on boot)
- [x] Telemetry/sensor CSV **append-or-create** on boot (no truncate of existing logs)
- [x] Explicit SD format command: `SET;1;5;FORMAT` (lab only)
- [x] Text protocol with `SEQ` + CRC-16/CCITT + `ACK` type  
  Frame shape: `KBST;ADDR;TYPE;GROUP;CMD;PAYLOAD;UNIT;SEQ;CRC16;TSBK`  
  LoRa RX requires integrity; UART still accepts legacy frames
- [x] Radio init retries; system continues if radio or SD fails (beacon reports flags)
- [x] Sensor init retries (light / env / IMU); IMU WHO_AM_I fail-closed
- [x] Multicore-safe last telem/sensor copies; event log returned by value

### GPS / sensors / features

- [x] NMEA `*xx` checksum validation; `$GNRMC` / `$GNGGA` accepted
- [x] GPS lock / loss events emitted
- [x] GPS power duty-cycle in battery mode
- [x] MPU-6050 driver + commands group **2** (`2.0` status, `2.1` accel, `2.2` gyro)
- [x] IMU columns in sensor CSV / last-sensor record
- [x] Recent telemetry dump command **8.4**

### Operator / host tooling

- [x] Operator SEQ + CRC-16 encode/decode + command retry
- [x] Radio debug: standalone `?` line (not frame suffix)
- [x] Auto-ACK for unsolicited health beacons; live beacon UI
- [x] Commands UI: health `1.4`, IMU `2.x`, format SD `1.5`, recent telem `8.4`
- [x] Host CRC + ground bridge framing tests: [`tests/host/`](tests/host/)

### Quality / docs / CI

- [x] GitHub Actions: [`.github/workflows/firmware.yml`](../.github/workflows/firmware.yml) (host tests + both UF2 builds; Pico SDK cache)
- [x] Protocol + event IDs: [`commands.md`](commands.md)
- [x] Operator checklist: [`kubisat_firmware/FLIGHT_CHECKLIST.md`](kubisat_firmware/FLIGHT_CHECKLIST.md)

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

## Hardware validation (lab — requires physical boards)

These cannot be completed in CI. Track pass/fail on the assembled hardware:

| # | Check | Status |
|---|--------|--------|
| 1 | Confirm DIO0 = GP20 (continuity / IRQ) | **PENDING** |
| 2 | Smoke-test CRC frames E2E via ground bridge + operator | **PENDING** |
| 3 | Lab `1.5 FORMAT` once; confirm boot never auto-formats / never wipes logs | **PENDING** |
| 4 | Watchdog hang → reboot + `WATCHDOG_RESET` in event log | **PENDING** |
| 5 | IMU WHO_AM_I on assembled sensor board | **PENDING** |
| 6 | Battery vs USB mode heuristic vs real INA3221 | **PENDING** |

Update this table after each lab session. See [`kubisat_firmware/FLIGHT_CHECKLIST.md`](kubisat_firmware/FLIGHT_CHECKLIST.md).

---

## Planned future fixes / follow-ups

### Comms / protocol hardening

| Item | Notes |
|------|--------|
| Drop UART legacy frames | After all tools migrate, require CRC on UART too |
| Optional app-layer anti-replay | SEQ window / reject duplicates (not started) |
| Re-enable beacon `require_ack` | Optional once host ACK is field-proven |

### Storage / telemetry

| Item | Notes |
|------|--------|
| Optional use of `SD_CARD_DETECT_PIN` | Defined but unused — wire into `fs_init` or remove |
| On-demand SD file dump over LoRa | `8.4` is RAM buffer only; full SD download not implemented |
| Log rotate | Prefer rotate after large append-only files |

### Platform / architecture (out of current roadmap)

| Item | Notes |
|------|--------|
| True OTA (dual-slot / UF2 over LoRa) | Remote BOOTSEL only for now |
| FreeRTOS / message queues | Intentionally deferred; stay multicore + mutexes |
| Binary radio framing | Deferred; keep text `KBST…TSBK` |
| Crypto / authenticated commands | Not planned |
| Magnetometer (HMC5883L) | Schematic only; no driver yet |

### CI / quality gaps

| Item | Notes |
|------|--------|
| Expand host tests further | Full firmware `frame.cpp` decode; command dispatch table check |
| `-Werror` on first-party code | Not enabled yet |
| Generate `commands.md` from handler map | Docs are hand-synced |
| HIL / on-target smoke job | None |

---

## Quick operator pointers

1. Protocol details: [`commands.md`](commands.md)  
2. Pins: [`kubisat_firmware/PINMAP.md`](kubisat_firmware/PINMAP.md)  
3. Pre-flight steps: [`kubisat_firmware/FLIGHT_CHECKLIST.md`](kubisat_firmware/FLIGHT_CHECKLIST.md)  
4. Host tests: `cmake -S software/tests/host -B build-host-tests && cmake --build build-host-tests && ctest --test-dir build-host-tests`
