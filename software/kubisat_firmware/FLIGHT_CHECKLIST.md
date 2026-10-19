# KubiSat flight / field checklist

1. Format SD once in the lab: `KBST;0;SET;1;5;FORMAT;;<seq>;<crc>;TSBK` (or UART legacy), then verify mount via health `1.4`.
2. Set RTC time (`3.0`) and timezone (`3.1`).
3. Power on satellite, confirm `HELLO` / `START_MODE_*` and periodic health beacon (`1.4`).
4. Smoke commands: build `1.1`, uptime `1.3`, last telemetry `8.2`, IMU `2.1`.
5. Confirm GPS lock events appear after outdoor sky view; battery mode duty-cycles GPS power.
6. Before flight: verbosity ≤ INFO, USB disconnected, SD seated, antenna attached.
