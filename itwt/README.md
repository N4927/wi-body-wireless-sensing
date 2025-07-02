
# WiFi iTWT Example with TCP Task and Power Optimization

| Supported Targets | ESP32-C6 | ESP32-C61 |
| ----------------- | -------- | --------- |

## Overview

This example demonstrates how to use **Individual Target Wake Time (iTWT)** on ESP32-C6 along with:
- a custom TCP client task
- optional static IP configuration
- basic light sleep support for power optimization

It integrates with the existing **Espressif iTWT APIs** and can be extended or benchmarked using the provided TCP client.

---

## Features

- **iTWT setup** with trigger-enabled, announced flow
- TCP client sending payload on wake event
- Configurable static IP
- Optional light sleep power optimization (if enabled in `sdkconfig`)
- CLI shell interface (when power management is not enabled)
- Automatic reconnect on disconnect
- Checksum + header wrapping of payload
- Predefined long payload string for stress testing

---

## Requirements

- ESP-IDF v5.4.1 or higher
- WiFi 6 capable router (AX) with TWT support
- ESP32-C6/C61 dev board
- Access to the target router for SSID and password configuration

---

## Setup

### 1. Configure Project

Before building, open the project configuration menu and set your Wi-Fi credentials and optional static IP:

```bash
idf.py menuconfig
```

Set the following:
- `Example Configuration` → `WiFi SSID`
- `Example Configuration` → `WiFi Password`
- (Optional) Enable and configure static IP under `Example Configuration`

> 💡 If using static IP, make sure the IP and netmask match your local network.

---

### 2. Build and Flash

```bash
idf.py build
idf.py -p <PORT> flash monitor
```

---

## How It Works

### iTWT Setup

- After connecting to the AP, the device negotiates an **individual TWT agreement**.
- Trigger-enabled, announced mode is used with specified mantissa and exponent values for service period and wake duration.

### TCP Task

- Once iTWT is established, a TCP client connects to the configured host IP and port (default: `3333`).
- At each TWT wake event, a predefined string payload is sent repeatedly until the wake window closes.
- If no wake event occurs, a keep-alive byte (`0x00`) is sent periodically.

### Light Sleep Mode

- If light sleep is enabled in `sdkconfig`, the console is disabled.
- The device enters light sleep automatically based on system configuration.



## File Structure

- `app_main.c`: Entry point, initializes NVS, WiFi, and CLI (or light sleep)
- `wifi_itwt()`: Configures and starts WiFi, handles TWT negotiation
- `tcp_client_task()`: TCP send loop during wake periods
- `encode_arr() / encode_str()`: Utility functions for packet formatting with headers and checksums


## Known Limitations

- Only works with routers supporting WiFi 6 and TWT.
- not all wi-fi 6 router support all wi-fi 6 feature




## TWT Parameter Configuration

To configure the iTWT behavior, the following parameters are defined in the code:

| Parameter              | Description |
|------------------------|-------------|
| `wake_value`           | Minimum wake duration (unit = 256 µs). For example, `1` means 256 µs |
| `wake_invl_mant`       | Wake interval mantissa. Combined with exponent for full wake interval |
| `wake_invl_expn`       | Wake interval exponent. Wake interval = `mantissa * 2^exponent` |
| `min_wake_dura`        | Number of units for the wake duration |
| `wake_duration_unit`   | Unit of wake duration: `0` = 256 µs, `1` = 1024 µs |

You can adjust these in the `got_ip_handler()` under the `wifi_itwt_setup_config_t` structure. This controls how often and for how long the device wakes up from TWT sleep.

---

## Sending Only During Awake Window

The code ensures data is sent **only during the TWT-awake window** using the following logic in `tcp_client_task()`:

```c
int64_t start_us = esp_timer_get_time();
int64_t deadline = start_us + awake_us * INCREASE;

while (esp_timer_get_time() < deadline) {
    send(sock, payload_str, strlen(payload_str), 0);
}
```

This guarantees that TCP traffic is transmitted **only when the device is scheduled to be awake**, reducing power consumption and aligning with the negotiated TWT schedule.
it is also needed to tune the INCREASE parameter to assure a small margin between the end of data sending and the entering in sleep mode



This mechanism respects the TWT agreement while maintaining the connection to the server.
