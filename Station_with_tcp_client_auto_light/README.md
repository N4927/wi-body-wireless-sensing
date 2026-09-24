
# Wi-Fi STA TCP Client with Timestamped Data Streaming

| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- |

This example demonstrates how to use an ESP32 as a Wi-Fi Station (STA) that connects to a TCP server (e.g., an ESP32 SoftAP), and streams packets with embedded timestamps and headers.

---

## Features

- Connects to a predefined Wi-Fi Access Point (SoftAP)
- Initiates TCP connection to a given IP and port
- Sends timestamped data packets with custom headers
- Dynamically controls duty cycle: active vs idle time
- Uses light sleep during idle periods for power saving
- Supports packet sequence tracking and per-packet timestamps
- Easily extendable for synchronization experiments

---

## Packet Format

Each packet sent is prefixed with the following structure:

```c
typedef struct {
    uint8_t  board_id;
    uint32_t sequence;
    uint16_t length;
    uint64_t tsf_us;
} PacketHeader;
```

- `board_id`: identifier of the sending board
- `sequence`: auto-incremented counter
- `length`: payload size (default: 1460 bytes)
- `tsf_us`: timestamp in microseconds (from `esp_timer_get_time()`)

---

## Setup

### 1. Configure Wi-Fi and Server IP

Edit the following macros in code if needed:

```c
#define EXAMPLE_ESP_WIFI_SSID   "YOUR_WIFI_SSID"
#define EXAMPLE_ESP_WIFI_PASS   "YOUR_WIFI_PASSWORD"
#define SERVER_IP               "192.168.4.1"
#define SERVER_PORT             3333
```

### 2. Flash and Monitor

```bash
idf.py -p PORT flash monitor
```

To exit the monitor, press `Ctrl+]`.

---

## Duty Cycling

The application uses an **active + idle loop** to control transmission and power usage:

- **Active phase**:
  - Wi-Fi power saving is disabled (`WIFI_PS_NONE`)
  - Packets are sent as fast as possible
- **Idle phase**:
  - Power saving is enabled (`WIFI_PS_MIN_MODEM`)
  - ESP enters light sleep for remaining cycle time

```c
#define LATENCY_HMI_MS      100
#define ACTIVE_PERCENTAGE   100
#define ACTIVE_TIME_US      (LATENCY_HMI_MS * 1000 * ACTIVE_PERCENTAGE / 100)
#define DUTY_CYCLE_US       (LATENCY_HMI_MS * 1000)
```

These values can be tuned for use cases such as HMI, BIO, or SPORT timing profiles.

---

## Example Log Output

```text
I (3829) STA: connected to ap SSID:YOUR_WIFI_SSID password:YOUR_WIFI_PASSWORD
I (3929) TCP_CLIENT: TCP client task started
I (3939) TCP_CLIENT: Connected to 192.168.4.1:3333
```


## Power Management

The app uses:

```c
esp_pm_config_t pm_cfg = {
    .max_freq_mhz       = 160,
    .min_freq_mhz       = 40,
    .light_sleep_enable = true
};
```

This allows dynamic frequency scaling and automatic light sleep during idle periods.

---
