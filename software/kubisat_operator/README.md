# KubiSat Operator

Avalonia/.NET 8 desktop app for commanding the satellite through the ground station UART↔LoRa bridge.

Part of the main KubiSat repository. Keep protocol changes in sync with [`../commands.md`](../commands.md) and satellite firmware.

## Protocol

Frames use SEQ + CRC-16/CCITT-FALSE:

```text
KBST;ADDR;TYPE;GROUP;CMD;PAYLOAD;UNIT;SEQ;CRC16;TSBK
```

- Connect to the ground Pico UART at **115200** (GP8/GP9), not USB CDC.
- Radio debug: enable the checkbox; the app sends a standalone `?` line before each command.
- Unsolicited health beacons (`VAL 1.4`) are auto-ACKed and shown on the diagnostics tab.

## Build / run

```sh
dotnet build
dotnet run
```

Demo mode: select port `demo` (no hardware).

Linux install helper: [`scripts/install-linux.sh`](scripts/install-linux.sh).
