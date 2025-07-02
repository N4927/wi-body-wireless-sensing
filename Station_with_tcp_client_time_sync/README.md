
# Wi-Fi Station TCP Client with GPIO-Triggered TSF Timestamping

| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- |

This example demonstrates how to build a TCP client that captures **TSF timestamps** when triggered by a GPIO rising edge and sends this information over a TCP connection. This is useful for synchronization applications, multi-board experiments, and real-time event marking.

---

## Features

- Runs in **Wi-Fi Station (STA)** mode
- Connects to a configurable SoftAP (e.g., ESP32 SoftAP server)
- Captures TSF (Timing Synchronization Function) timestamp via:
  - GPIO rising edge interrupt (default: GPIO18)
  - Periodic polling fallback
- Sends packets over TCP to server
- Differentiates "triggered" packets via payload byte
- Displays TSF logs periodically (optional)

---

## Packet Format

```c
typedef struct {
    uint8_t  board_id;
    uint32_t sequence;
    uint16_t length;
    uint64_t tsf_us;
} PacketHeader;
```

The payload follows this header and may be either:
- `0x01`-filled (triggered)
- `0x00`-filled (background)

---

## GPIO Trigger

- Input GPIO is configured on pin 18.
- Rising edge ISR captures the current TSF timestamp via `esp_wifi_get_tsf_time()`.
- A task is notified to send a “triggered” packet immediately.
- Between triggers, regular packets are sent with background payload.

---

## How It Works

1. STA connects to the specified AP (`WIFI_SSID`/`WIFI_PASS`)
2. Periodically logs current TSF (optional)
3. Establishes a TCP connection to the specified server (`HOST_IP:HOST_PORT`)
4. On GPIO trigger:
    - Captures TSF timestamp
    - Sends a marked packet
5. Continues sending default packets during idle time

--> need to be used with  acess point time synch

---

## Setup

### Configuration Macros

```c
#define WIFI_SSID     "myssid"
#define WIFI_PASS     "mypassword"
#define HOST_IP       "192.168.4.1"
#define HOST_PORT     3333
#define TRIG_GPIO     18
```

### Build and Run

```bash
idf.py build
idf.py -p PORT flash monitor
```

---

## Output Example

```text
I (3256) WIFI_SETUP: TSF = 3912945 µs
I (4056) TCP_TASK: Connected to 192.168.4.1:3333
I (5056) TCP_TASK: Triggered packet sent at TSF = 4100000 µs
```

---

## Notes

- `esp_rom_delay_us(500)` introduces a controlled delay between packets
- Uses `ulTaskNotifyTake()` for fast ISR-to-task signaling
- You can disable TSF debug output by commenting `esp_timer_start_periodic(...)`

---

## Applications

- Multi-board synchronization via GPIO + TSF timestamps
- Benchmarking AP-to-client TSF alignment
- Trigger-to-TSF latency measurements

---

