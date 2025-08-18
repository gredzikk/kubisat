# KubiSat Communication Protocol

This document outlines the command structure for communicating with the KubiSat satellite firmware.

## Command Frame Structure

All communication follows a standardized frame format, separated by semicolons (`;`).

`START_TOKEN;ADDRESS;COMMAND_TYPE;GROUP_ID;COMMAND_ID;PAYLOAD;END_TOKEN`

| Field          | Description                                                               | Example      |
| -------------- | ------------------------------------------------------------------------- | ------------ |
| `START_TOKEN`  | A fixed 4-character string that marks the beginning of a frame.           | `KBST`       |
| `ADDRESS`      | The address of the destination device. `0` is the satellite.              | `0`          |
| `COMMAND_TYPE` | The type of action to perform. Common types are `GET` and `SET`.          | `GET`        |
| `GROUP_ID`     | A number identifying the functional group of the command (e.g., `1` for Diagnostics). | `1`          |
| `COMMAND_ID`   | A number identifying the specific command within the group.               | `0`          |
| `PAYLOAD`      | Optional data for the command. Can be empty.                              | `12345`      |
| `END_TOKEN`    | A fixed 4-character string that marks the end of a frame.                 | `TSBK`       |

---

## Available Commands

Below is a list of available commands with examples.

### Group 1: Diagnostic Commands

-   **`1.0` - Get Commands List**
    -   Description: Lists all available commands.
    -   Example Request: `KBST;0;GET;1;0;;TSBK`

-   **`1.1` - Get Build Version**
    -   Description: Retrieves the current firmware build version.
    -   Example Request: `KBST;0;GET;1;1;;TSBK`

-   **`1.2` - Get Power Mode**
    -   Description: Gets the system power mode (BATTERY or USB).
    -   Example Request: `KBST;0;GET;1;2;;TSBK`

-   **`1.3` - Get Uptime**
    -   Description: Gets the system uptime in seconds.
    -   Example Request: `KBST;0;GET;1;3;;TSBK`

-   **`1.8` - Get/Set UART Verbosity**
    -   Description: Gets or sets the verbosity level for UART debug messages.
    -   Example Get: `KBST;0;GET;1;8;;TSBK`
    -   Example Set (to level 2): `KBST;0;SET;1;8;2;TSBK`

-   **`1.9` - Enter Bootloader Mode**
    -   Description: Resets the Pico into BOOTSEL mode for firmware updates.
    -   Example Request: `KBST;0;SET;1;9;;TSBK`

### Group 3: Clock Commands

-   **`3.0` - Get/Set System Time**
    -   Description: Gets or sets the system time using a Unix timestamp.
    -   Example Get: `KBST;0;GET;3;0;;TSBK`
    -   Example Set (to timestamp 1755312000): `KBST;0;SET;3;0;1755312000;TSBK`

-   **`3.1` - Get/Set Timezone Offset**
    -   Description: Manages the timezone offset in seconds from UTC.
    -   Example Get: `KBST;0;GET;3;1;;TSBK`
    -   Example Set (to UTC-5 / -18000 seconds): `KBST;0;SET;3;1;-18000;TSBK`

-   **`3.4` - Get Internal Temperature**
    -   Description: Reads the temperature from the internal sensor of the RTC.
    -   Example Request: `KBST;0;GET;3;4;;TSBK`

### Group 5: Event Commands

-   **`5.1` - Get Last Events**
    -   Description: Retrieves the last N events from the log. If N is 0, it retrieves all events.
    -   Example Request (last 10 events): `KBST;0;GET;5;1;10;TSBK`

-   **`5.2` - Get Event Count**
    -   Description: Gets the total number of events currently stored in the event log.
    -   Example Request: `KBST;0;GET;5;2;;TSBK`

### Group 7: GPS Commands

-   **`7.1` - Get/Set GPS Power Status**
    -   Description: Controls the power state of the GPS module (0=OFF, 1=ON).
    -   Example Get: `KBST;0;GET;7;1;;TSBK`
    -   Example Set (Turn ON): `KBST;0;SET;7;1;1;TSBK`

-   **`7.2` - Enable GPS UART Passthrough**
    -   Description: Enables direct UART communication with GPS module for X seconds.
    -   Example Set (Enable): `KBST;0;SET;7;2;120;TSBK`

### Group 8: Telemetry Commands

-   **`8.2` - Get Last Telemetry Record**
    -   Description: Retrieves the last complete telemetry record from the buffer.
    -   Example Request: `KBST;0;GET;8;2;;TSBK`

-   **`8.3` - Get Last Sensor Record**
    -   Description: Retrieves the last sensor data record.
    -   Example Request: `KBST;0;GET;8;3;;TSBK`