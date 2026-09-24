
# Wi-Fi Station TCP Client with Configurable Duty Cycle

| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- |

This example demonstrates a simple TCP client running in Wi-Fi Station (STA) mode, which connects to a remote TCP server and sends payloads periodically with a configurable **active/sleep duty cycle** for energy efficiency. This code use MIN MODEDM sleep.

---

## Features

- Connects to a predefined Wi-Fi network (SSID + password)
- Establishes a TCP connection to a remote server (`SERVER_IP`)
- Sends predefined payload continuously during active period
- Enters modem sleep between cycles for power saving
- Uses software timers for precise cycle scheduling

---

## Configuration

### Wi-Fi Settings

```c
#define EXAMPLE_ESP_WIFI_SSID  "YOUR_WIFI_SSID"
#define EXAMPLE_ESP_WIFI_PASS  "YOUR_WIFI_PASSWORD"
```

### TCP Server IP and Port

```c
#define SERVER_IP      "192.168.4.1"
#define SERVER_PORT    3333
```

### Duty Cycle Parameters

```c
#define LATENCY_BIO_MS     100000  // Total period in ms
#define PERCENTAGE         100     // Active portion in %

#define DUTY_CYCLE_US      (LATENCY_BIO_MS * 1000)
#define ACTIVE_TIME_US     ((PERCENTAGE * DUTY_CYCLE_US) / 100)
```

This allows full control of how much time the Wi-Fi stays active per cycle (e.g., 100 ms ON, 900 ms OFF).

---

## How It Works

1. Connects to the configured Wi-Fi AP
2. Opens a TCP socket to the server
3. In the **active phase**, Wi-Fi PS is disabled (`WIFI_PS_NONE`) and the payload is sent repeatedly
4. In the **idle phase**, Wi-Fi PS is re-enabled (`WIFI_PS_MIN_MODEM`) and the task delays until the next cycle
5. The process repeats unless the socket disconnects

---

## Example Log Output

```text
I (3000) STA: Connected to AP: YOUR_WIFI_SSID
I (3010) TCP_CLIENT: Starting tcp_client task
I (4010) TCP_CLIENT: Connected to 192.168.4.1:3333
```

---



## Power Saving

- Active phase: `esp_wifi_set_ps(WIFI_PS_NONE)`
- Idle phase: `esp_wifi_set_ps(WIFI_PS_MIN_MODEM)`

This reduces average current during long idle cycles, ideal for battery-operated applications.

---

## License

This example is in the Public Domain (or CC0 licensed, at your option).
