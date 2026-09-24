
| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- |

# Wi-Fi SoftAP Example with Multi-node TCP Server

This example demonstrates how to use the ESP32 Wi-Fi SoftAP functionality to create an Access Point and handle multiple TCP clients simultaneously. It also tracks and logs throughput per client node.

## Overview

- Creates a Wi-Fi SoftAP with configurable SSID and password.
- Runs a TCP server that accepts multiple incoming client connections.
- Receives data from multiple nodes, each tagged with a board ID.
- Computes and logs average throughput for each node every 10 seconds.

> ✅ This version supports multiple stations (e.g., board 1 and board 2) sending data concurrently.

---

## How to Use This Example

### Configure the Project

Open the configuration menu:

```bash
idf.py menuconfig
```

Under `Example Configuration`:

- Set the `WiFi SSID`
- Set the `WiFi Password` (leave empty for open network)

Optional: Adjust channel and max connections in the source code if needed.

---

### Build and Flash

```bash
idf.py -p PORT flash monitor
```

To exit the monitor, press `Ctrl+]`.

See the ESP-IDF Getting Started Guide if you're new to the toolchain:

- [ESP-IDF Getting Started](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html)

---

## Example Output

The log will show:

```text
I (917) wifi: mode : softAP (30:ae:a4:80:45:69)
I (917) wifi softAP: wifi_init_softap finished.SSID:YOUR_WIFI_SSID password:YOUR_WIFI_PASSWORD
I (26457) wifi: station: 70:ef:00:43:96:67 join, AID=1
I (27657) esp_netif_lwip: DHCP server assigned IP to a station, IP is: 192.168.4.2
I (10000000) AP: [AVG 10s] B1=1024000 B/s B2=980000 B/s AVG=1002000 B/s
```

---

## Troubleshooting

If you encounter issues, check the following:

- SSID and password are correct.
- Clients are sending properly formatted packets with headers.
- The board is connected to a serial terminal for logs.

For technical queries, please open an [issue](https://github.com/espressif/esp-idf/issues) on GitHub.
