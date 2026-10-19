# KubiSat Ground Station Firmware

This repository contains the firmware for the KubiSat Ground Station, designed to run on a Raspberry Pi Pico. It acts as a communication bridge, relaying data between a host computer connected via USB/UART and a remote LoRa device, such as the KubiSat satellite.

Please read doc.pdf from release directory to get grasp of satellite internal workings first.

## Features

*   **LoRa-UART Bridge:** Facilitates bidirectional communication between a serial terminal and a LoRa radio module.
*   **Simple Addressing:** Uses a basic addressing scheme to ensure messages are sent to and received from the correct device.
*   **Timestamped Logging:** All messages printed to the serial console are timestamped for easier debugging and data analysis.
*   **Signal Strength Reporting:** Can be triggered to report the RSSI (Received Signal Strength Indicator) and SNR (Signal-to-Noise Ratio) of the last received packet.
*   **Pico C/C++ SDK:** Built using the standard Raspberry Pi Pico C/C++ SDK and CMake.

## Hardware Prerequisites

*   A Raspberry Pi Pico or Pico W.
*   A LoRa module (e.g., based on the Semtech SX1276/7/8) connected to the Pico via SPI.

The default pin configuration and LoRa addresses are defined in [`pin_config.h`](pin_config.h).

## Building the Firmware

The recommended way to build the firmware is by using [Visual Studio Code](https://code.visualstudio.com/) with the official [Raspberry Pi Pico/RP2040](https://marketplace.visualstudio.com/items?itemName=raspberry-pi.pico-wgo) extension.

1.  Open the project folder in VS Code.
2.  Open Pico SDK Extension and select "Import project"
3.  Select directory of repository on disk
4.  Click import button
5.  Use the "Build" button in the status bar at the bottom of the VS Code window to compile the project.

Alternatively, you can build from the command line:

1.  Ensure you have the [Raspberry Pi Pico C/C++ SDK](https://github.com/raspberrypi/pico-sdk) set up.
2.  Clone this repository.
3.  Create a build directory:
    ```sh
    mkdir build
    cd build
    ```
4.  Run CMake and Make:
    ```sh
    cmake ..
    make
    ```
5.  A `main.uf2` file will be generated in the `build/` directory and also copied to the `release/` directory.

## Flashing and Usage

1.  Connect your Raspberry Pi Pico to your computer while holding down the `BOOTSEL` button.
2.  Drag and drop the `main.uf2` file onto the `RPI-RP2` mass storage device. The Pico will reboot automatically.
3.  Connect to the Pico's serial port using a terminal emulator (e.g., PuTTY, minicom, VS Code Serial Monitor) with a baud rate of `115200`.
4.  Type a message and press Enter to send it via LoRa.
5.  Incoming LoRa messages will be displayed in the terminal.
6.  To request signal strength on the next incoming packet, send a line containing only `?`. RSSI and SNR are appended to that frame as `;rssi;snr` (protocol frames themselves are printed without timestamps).

## License

This project is licensed under the MIT License. See the [LICENCE](software/ground_firmware/LICENCE) file for details.