# WI-Body

Towards an Ultra-Low-Power and High-Throughput Network for Continuous Body Monitoring

This repository contains the work developed for my Bachelor's thesis at ETH Zurich in 2025.

WI-Body investigates a wireless body-area sensing architecture designed to support continuous and energy-efficient transmission from multiple wearable sensing nodes. The project focuses on the development and experimental characterization of the wireless communication system, with particular attention to throughput, power consumption, synchronization, and multi-node operation.

## Project Overview

Wearable sensing systems for continuous body monitoring require reliable wireless communication while operating under strict energy constraints.

WI-Body explores a Wi-Fi-based architecture composed of:

- an Access Point (AP) responsible for creating and managing the wireless network;
- one or more Station nodes responsible for acquiring and transmitting sensor data;
- firmware mechanisms for communication, synchronization, and power optimization;
- a custom hardware platform designed for wearable sensing applications.

The system was developed and evaluated using ESP-IDF and ESP32-based devices.

## Key Contributions

The work carried out during the thesis includes:

- development of the Access Point and Station firmware;
- implementation and testing of TCP-based wireless communication;
- investigation of multi-node communication;
- implementation of time-synchronization mechanisms;
- evaluation of different ESP32 power-saving strategies;
- characterization of wireless throughput and power consumption;
- development and testing of a custom Wi-Body hardware platform;
- experimental analysis of the trade-off between communication performance and energy efficiency.

## Repository Structure

The repository contains the different firmware versions and experiments developed throughout the project.

Examples include:

- `SoftAP_with_tcp_server` - Access Point implementation with TCP server;
- `SoftAP_with_tcp_server_dual` - Access Point implementation for multi-node communication;
- `Station_with_tcp_client_*` - Station implementations and power-saving configurations;
- `time_synch` - time-synchronization experiments;
- `power_optimization` - power-consumption optimization experiments;
- `two_client_througput` - multi-client throughput characterization;
- `Wi-body` - hardware design files and related project material for the Wi-Body platform.

Some folders represent intermediate experimental implementations retained to document the development and characterization process.

## Development Environment

The firmware was developed using the Espressif IoT Development Framework (ESP-IDF).

### Requirements

- Visual Studio Code
- ESP-IDF extension
- ESP-IDF toolchain
- compatible ESP32 development hardware

Generated build files are intentionally excluded from the repository. When using the firmware on another computer, the corresponding ESP-IDF project may therefore need to be configured and rebuilt locally.

Before flashing any Station firmware, configure Wi-Fi credentials and host addresses locally using ESP-IDF menuconfig or the relevant configuration macros. Do not commit private SSIDs, passwords, tokens, or local machine paths.

## Hardware Setup

The Wi-Body board can be connected to a computer through UART using:

- 3.3 V
- GND
- TX
- RX

To enter flashing mode, hold the corresponding boot button while connecting the board through UART.

After flashing, reset the board to start the firmware.

## Thesis

The complete Bachelor's thesis is intended to be available in the `docs` directory:

[Alberto Fasulo - WI-Body Bachelor Thesis](docs/Alberto_Fasulo_WI-Body_Bachelor_Thesis.pdf)

Alberto Fasulo, "WI-Body: Towards an Ultra-Low-Power and High-Throughput Network for Continuous Body Monitoring", ETH Zurich, 2025.

## Author

Alberto Fasulo

Bachelor's Thesis, ETH Zurich  
2025

The project was carried out at ETH Zurich with the supervision and support of Giusy Spacone, Sebastian Frey, and Andrea Cossettini.

## Acknowledgements

I would like to thank my supervisors and the Integrated Systems Laboratory at ETH Zurich for their guidance and support throughout the project.

## License

Original software developed as part of this project is released under the [Apache License 2.0](LICENSE), unless otherwise specified.

Original documentation and figures are released under the [Creative Commons Attribution 4.0 International license](docs/LICENSE), unless otherwise specified.

Hardware design files are licensed separately under the [Solderpad Hardware License 2.1](Wi-body/LICENSE), unless otherwise specified.

Third-party software and components retain their respective original licenses.

Copyright (c) 2025 Alberto Fasulo.
