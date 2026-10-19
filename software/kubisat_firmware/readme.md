# Kubisat Firmware

This repository contains the firmware for the Kubisat project, a small satellite based on the Raspberry Pi Pico microcontroller. The firmware is responsible for managing the satellite's core operations, including sensor data acquisition, data logging to an SD card, and long-range (LoRa) communication.

This project was originally conceived as the subject of my engineering thesis.
I later decided to transform it into an open-source project, hoping it would be useful to a wider community and open for contributions.

## Features


*   **Modular Architecture:** Organized into libraries for communications, sensor handling ([`lib/sensors`](lib/sensors)), power management ([`lib/powerman`](lib/powerman)), and more.
*   **Pin map:** See [`PINMAP.md`](PINMAP.md) before changing GPIO assignments.
*   **Flight checklist:** See [`FLIGHT_CHECKLIST.md`](FLIGHT_CHECKLIST.md).
*   **Data Logging:** Utilizes the [`pico-vfs`](lib/storage/pico-vfs/README.md) library to manage a virtual file system, enabling robust logging to an SD card.
*   **LoRa Communication:** Integrates LoRa functionality for sending and receiving data frames.
*   **Automated Build Process:** Uses CMake for building the firmware, with automatic build numbering and release artifact generation.
*   **Documentation Generation:** Includes support for generating detailed project documentation using Doxygen and LaTeX.

## Prerequisites

Before you can build the firmware, you need to set up the Raspberry Pi Pico C/C++ development environment. Please follow the official [Getting started with Raspberry Pi Pico](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf) guide.

Ensure you have the following installed:
*   ARM GCC Compiler
*   CMake
*   Raspberry Pi Pico SDK

For generating documentation, you will also need:
*   Doxygen
*   Graphviz
*   A LaTeX distribution (e.g., [TeX Live](https://www.tug.org/texlive/))

## Building the Firmware

The recommended way to build the firmware is by using [Visual Studio Code](https://code.visualstudio.com/) with the official [Raspberry Pi Pico/RP2040](https://marketplace.visualstudio.com/items?itemName=raspberry-pi.pico-wgo) extension.

1.  Open the project folder in VS Code.
2.  Open Pico SDK Extension and select "Import project"
3.  Select directory of repository on disk
4.  Click import button
5.  Use the "Build" button in the status bar at the bottom of the VS Code window to compile the project.

Alternatively, you can build from the command line:

1.  **Clone the repository:**
    ```sh
    git clone https://github.com/gredzikk/kubisat_firmware.git
    cd kubisat_firmware
    ```

2.  **Create a build directory:**
    ```sh
    mkdir build
    cd build
    ```

3.  **Run CMake:**
    Make sure to replace `/path/to/pico-sdk` with the actual path to your Pico SDK installation.
    ```sh
    cmake .. -DPICO_SDK_PATH=/path/to/pico-sdk
    ```

4.  **Compile the project:**
    ```sh
    make
    ```

Upon successful compilation, the firmware file `main.uf2` will be available in the `build` directory. A versioned copy, like `kubisat_firmware_v541.uf2`, will also be placed in the `release` directory.

## Flashing the Firmware

1.  Connect your Raspberry Pi Pico to your computer while holding down the `BOOTSEL` button. It will appear as a mass storage device.
2.  Drag and drop the `main.uf2` file from the `build` directory onto the Pico's storage.
3.  The Pico will automatically reboot and run the new firmware.

## Building the Documentation

To generate the project documentation as a PDF, run the `doc` target from the build directory:

```sh
make doc
```

This will generate a PDF file named `doc_<build_number>.pdf` inside the `release/doc` directory.

## Contributing

Contributions are what make the open-source community such an amazing place to learn, inspire, and create. Any contributions you make are **greatly appreciated**.

If you have a suggestion that would make this better, please fork the repo and create a pull request. You can also simply open an issue with the tag "enhancement".
Don't forget to give the project a star! Thanks again!

1.  Fork the Project
2.  Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3.  Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4.  Push to the Branch (`git push origin feature/AmazingFeature`)
5.  Open a Pull Request

## License

This project is licensed under the MIT License. See the `LICENSE` file for more information.

If you use this code in your project, please provide attribution by including the original copyright notice and a link back to this repository.

To ensure your authorship is clearly marked, I also recommend adding a copyright header to the top of your source files, like this:
`// Copyright (c) 2025 <Your Name or GitHub Handle>`

## Credits

This software uses the following external libraries:

### pico-vfs

Copyright 2024, Hiroyuki OYAMA. All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.
- Neither the name of the copyright holder nor the names of its contributors may
  be used to endorse or promote products derived from this software without
  specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “ AS IS”  AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The following source code has been written with reference to the [ARM Embed OS](https://github.com/ARMmbed/mbed-os) source
code; ARM Embed OS is licensed under the Apache 2.0 licence.

- `src/blockdevice/sd.c`

Folders containing files by third parties are listed below. Each folder must contain
a README file specifying the licence for that file. The original licence text is
contained in these source files.

- `vendor/ff15` - BSD-2-Clause, [Copyright ChaN](http://www.elm-chan.org/fsw/ff/doc/appnote.html)
- `vendor/littlefs`- BSD-3-Clause

### pico-lora

MIT License

Copyright (c) 2021 Akshaya Bali

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.