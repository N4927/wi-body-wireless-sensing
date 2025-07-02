
# Wi-Fi SoftAP with TSF Synchronization and GPIO Trigger

| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- |

This example demonstrates how to use the ESP32 Wi-Fi SoftAP functionality with advanced features such as:

- Multiple TCP clients (e.g., sensor nodes)
- Timestamp synchronization using Target Synchronization Function (TSF)
- External trigger capture via GPIO interrupt
- Packet parsing with custom headers per client
- Throughput tracking

---

## Features

- Acts as a Wi-Fi Access Point (`SoftAP`) 
- Accepts TCP connections on port 3333
- Parses packets with a custom header containing board ID, sequence number, and timestamps
- Captures AP-side TSF timestamp on rising GPIO edge
- Compares AP TSF with synchronized TSF timestamps from multiple boards
- Prints aligned timestamps when both boards trigger
- Calculates network throughput every 10 seconds (disabled by default)

---

## Setup

### Configure the Project

Open project menuconfig:

```bash
idf.py menuconfig
```

Update Wi-Fi SSID and Password if necessary (also defined in the code):

- `WIFI_SSID`: default is `"myssid"`
- `WIFI_PASSWORD`: default is `"mypassword"`

### Flash the Board

```bash
idf.py -p PORT flash monitor
```

To stop the monitor, press `Ctrl+]`.

---

## How It Works

### Data Packet Format

The expected format for incoming packets is:

```c
typedef struct __attribute__((packed)) {
    uint8_t  board_id;
    uint32_t sequence;
    uint16_t length;
    uint64_t synced_timestamp_us;
} PacketHeader;
```

The payload immediately follows the header.

### TSF Synchronization

- When a board sends a payload with `payload[20] == 0x01`, it's considered a synchronization event.
- The AP captures its own TSF via `esp_wifi_get_tsf_time()` on GPIO trigger.
- Once both `board_id = 1` and `board_id = 2` have sent synchronization packets, it prints:

```text
<board1_tsf>,<board2_tsf>,<ap_tsf>
```

This allows offline TSF comparison and latency analysis.

### GPIO Trigger

- The GPIO pin 18 is used to detect an external synchronization signal.
- The interrupt stores the AP TSF timestamp, which is later printed alongside board TSFs.

---

## Output Example

```text
SoftAP started: SSID=myssid, chan=1
TCP server listening on port 3333
STA aa:bb:cc:dd:ee:ff joined, AID=1
123456789,987654321,192837465
```

---

## Notes

- You can monitor throughput calculation by enabling the `tp_cb()` timer callback.
- Beacon interval can also be cahnged to decrease power consumption.



This example is in the Public Domain (or CC0 licensed, at your option).
