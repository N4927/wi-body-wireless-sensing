# Wi-Fi Station TCP Client with Manual Light Sleep (Experimental)

| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- |

This example is an **experimental attempt** to implement **manual light sleep** in a TCP client for the ESP32 platform. It demonstrates basic active/idle duty cycling and is intended as a starting point for more advanced low-power implementations.

---

## ⚠️ Important Note

> ✅ For a working and validated light sleep example, **refer to**:  
> **`Station_tcp_client_auto_light`**  
> which uses the ESP-IDF built-in power management and tickless idle support for proper light sleep handling.

This current example does **not yet fully support automatic or reliable light sleep** and is under development for manual power control.