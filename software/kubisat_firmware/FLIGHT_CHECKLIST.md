# KubiSat flight / field checklist

## Lab validation (record pass/fail in FIRMWARE_STATUS.md)

1. Format SD once in the lab: `KBST;0;SET;1;5;FORMAT;;<seq>;<crc>;TSBK` (or operator Format SD), then verify mount via health `1.4`.
2. Reboot with existing logs; confirm `/telemetry.csv` and `/sensors.csv` are **not** wiped (append-only).
3. Set RTC time (`3.0`) and timezone (`3.1`).
4. Power on satellite, confirm `HELLO` / `START_MODE_*` and periodic health beacon (`1.4`) in operator.
5. Smoke over LoRa via ground: build `1.1`, uptime `1.3`, health `1.4`, last telemetry `8.2`, IMU `2.1` (SEQ+CRC required).
6. Arm radio debug `?` and confirm `;rssi;snr` on next RX.
7. Force a hang; confirm watchdog reboot + `WATCHDOG_RESET` event.
8. Confirm DIO0 = GP20 and IMU WHO_AM_I on the assembled board.
9. Confirm GPS lock events after outdoor sky view; battery mode duty-cycles GPS power.

## Pre-flight

10. Verbosity ≤ INFO, USB disconnected, SD seated, antenna attached.
