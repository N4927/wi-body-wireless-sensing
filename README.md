# WirelessSensingNode


## Install the Visual Studio Code ESP-IDF Extension

1. Follow the guidelines at: https://github.com/espressif/vscode-esp-idf-extension

Important: after the installation, make sure that the IDF_PATH is added to your user profile

https://docs.espressif.com/projects/esp-idf/en/release-v3.3/get-started-cmake/add-idf_path-to-profile.html

## Clone the existing repository

```
cd existing_repo
git remote add origin https://gitlab.ethz.ch/iis_thesis_bio/wirelesssensingnode.git
git branch -M main
git push -uf origin main
```
## Physical Board Setup

The board connects to your computer via a UART interface using the following pins: **3.3V**, **GND**, **TX**, and **RX**.

### Wiring Instructions

- Connect **RX (board)** to **TX (UART connector)**
- Connect **TX (board)** to **RX (UART connector)**
- Connect **3.3V** and **GND** to their respective counterparts

### Entering Flash Mode

To flash the board via UART:

1. **Press and hold the upper-right button** on the board.
2. While holding the button, connect the board via the UART interface.
3. Once the board is detected, you can release the button.

### Resetting After Flash

- After flashing is complete, press the **reset button** to reboot the board and begin monitoring its output.
