# KubiSat Communication Protocol

All frames use semicolon-separated fields:

```text
KBST;ADDR;TYPE;GROUP;CMD;PAYLOAD;UNIT;SEQ;CRC16;TSBK
```

| Field | Description |
|-------|-------------|
| `KBST` / `TSBK` | Start / end tokens |
| `ADDR` | Direction: `0` ground→sat, `1` sat→ground |
| `TYPE` | `GET`, `SET`, `VAL`, `RES`, `SEQ`, `ERR`, `ACK` |
| `GROUP` / `CMD` | Command identifiers |
| `PAYLOAD` | Optional value |
| `UNIT` | Optional unit (may be empty) |
| `SEQ` | uint16 sequence number |
| `CRC16` | 4-digit hex CRC-16/CCITT-FALSE over `ADDR;TYPE;GROUP;CMD;PAYLOAD;UNIT;SEQ` |

Example (build version request with integrity):

```text
KBST;0;GET;1;1;;;1;XXXX;TSBK
```

- **LoRa RX** requires `SEQ` + valid `CRC16`.
- **UART** still accepts legacy frames without `SEQ`/`CRC` during transition.
- On LoRa command RX, the satellite sends an `ACK` with the same `SEQ`, then the response.
- Host tools should retry unanswered commands (same or new `SEQ`).
- Ground firmware remains a transparent UART↔LoRa bridge.

---

## Group 1: Diagnostics

| ID | Ops | Description |
|----|-----|-------------|
| 1.0 | GET | List commands |
| 1.1 | GET | Build version |
| 1.2 | GET | Power mode (`BATTERY` / `USB`) |
| 1.3 | GET | Uptime (seconds) |
| 1.4 | GET | Health summary (`UP,RAD,SD,ENV,LUX,IMU,BAT`) |
| 1.5 | SET | Format SD — payload must be `FORMAT` (destructive, lab only) |
| 1.8 | GET/SET | UART verbosity 0–4 |
| 1.9 | SET | Enter BOOTSEL — payload `USB`, USB-powered only |

Periodic unsolicited health beacon uses group/cmd `1.4` every ~30 s over LoRa (with ACK retry).

## Group 2: IMU (MPU-6050)

| ID | Ops | Description |
|----|-----|-------------|
| 2.0 | GET | IMU init status |
| 2.1 | GET | Accel `ax,ay,az` (g) |
| 2.2 | GET | Gyro `gx,gy,gz` (dps) |

## Group 3: Clock

| ID | Ops | Description |
|----|-----|-------------|
| 3.0 | GET/SET | Unix time |
| 3.1 | GET/SET | Timezone offset (minutes, −720…720) |
| 3.4 | GET | DS3231 internal temperature |

## Group 5: Events

| ID | Ops | Description |
|----|-----|-------------|
| 5.1 | GET | Last N events (param N, 0 = all) |
| 5.2 | GET | Event count |

## Group 7: GPS

| ID | Ops | Description |
|----|-----|-------------|
| 7.1 | GET/SET | GPS power (`0`/`1`) |
| 7.2 | SET | UART passthrough for N seconds |

## Group 8: Telemetry

| ID | Ops | Description |
|----|-----|-------------|
| 8.2 | GET | Last telemetry CSV record |
| 8.3 | GET | Last sensor CSV record (includes IMU) |
| 8.4 | GET | Recent N telemetry records (`\|`-separated) |

---

## Event IDs (summary)

| Group | Codes |
|-------|--------|
| SYSTEM (0) | BOOT=1, SHUTDOWN=2, WATCHDOG_RESET=3, CORE1_START=4, CORE1_STOP=5 |
| POWER (1) | BATTERY_LOW=1 … CHARGING=0x0A |
| COMMS (2) | RADIO_INIT=1, RADIO_ERROR=2, MSG_RECEIVED=3, MSG_SENT=4, UART_ERROR=6 |
| GPS (3) | LOCK=1, LOST=2, ERROR=3, POWER_ON=4, POWER_OFF=5, … |
| CLOCK (4) | CHANGED=1, GPS_SYNC=2, GPS_SYNC_DATA_NOT_READY=3 |
