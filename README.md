# KubiSat: An Open-Source Pico-Satellite Project

Welcome to the KubiSat project! This repository contains all the necessary files to build your own small satellite based on the Raspberry Pi Pico, along with the corresponding ground station firmware. This project was originally conceived as an engineering thesis and has been open-sourced to be useful to a wider community.

## Project Structure

The repository is organized into several key directories:

-   [**`software/`**](software/): Firmware, shared protocol/radio code, and host tooling.
    -   [**`kubisat_firmware/`**](software/kubisat_firmware/): Satellite firmware. See the [firmware README](software/kubisat_firmware/readme.md).
    -   [**`ground_firmware/`**](software/ground_firmware/): Ground station LoRa↔UART bridge. See the [ground README](software/ground_firmware/readme.md).
    -   [**`common/`**](software/common/): Shared LoRa driver and protocol helpers (CRC, frame markers, sync word).
    -   [**`kubisat_operator/`**](software/kubisat_operator/): Avalonia/.NET operator GUI for talking to the ground station over serial.
    -   Status: [`software/FIRMWARE_STATUS.md`](software/FIRMWARE_STATUS.md), protocol: [`software/commands.md`](software/commands.md).
-   [**`hardware/`**](hardware/): KiCad project files for the satellite's printed circuit boards (PCBs), including the main board, power board, and sensor board.
-   [**`case_stl/`**](case_stl/): Contains the STL files for the 3D-printable satellite chassis.
-   [**`doc/`**](doc/): A collection of datasheets for the main components used in the project.

## Features

### Satellite (`kubisat_firmware`)

-   **Modular Architecture:** Code is organized into libraries for communications, sensors, power management, and data logging.
-   **Data Logging:** Uses the `pico-vfs` library to log telemetry and events to an SD card.
-   **LoRa Communication:** Sends and receives data frames using a LoRa module for long-range communication.
-   **Remote Commands:** Supports a command protocol for remote operation and diagnostics.
-   **Automated Build:** Uses CMake for a managed build process with automatic versioning.

### Ground Station (`ground_firmware`)

-   **LoRa-UART Bridge:** Relays messages between a host computer (via USB/Serial) and the satellite (via LoRa).
-   **Simple Addressing:** Ensures messages are correctly routed between the ground station and the satellite.
-   **Signal Strength Reporting:** Can be triggered to report the RSSI and SNR of received packets.

## Getting Started

To get started with building and flashing the firmware, please refer to the detailed instructions in the README files for each respective firmware:

-   **For the satellite:** [kubisat_firmware/readme.md](software/kubisat_firmware/readme.md)
-   **For the ground station:** [ground_firmware/readme.md](software/ground_firmware/readme.md)

The hardware can be manufactured using the KiCad files in the [`hardware/`](hardware/) directory, and the case can be 3D printed using the files in [`case_stl/`](case_stl/).

## Contributing

Contributions are welcome! Please read our [contributing guidelines](CONTRIBUTING.md) to get started. You can report bugs, suggest features, or submit pull requests.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.