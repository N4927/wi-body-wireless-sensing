### **Experiment ESP32-C6 to Router Setup:**

This setup consists of an ESP32-C6 module communicating wirelessly with a Wi-Fi router positioned approximately 50 cm away to ensure stable signal quality. The router is connected to a PC via Ethernet, allowing the PC to serve as a data receiver and logger.

A custom Python script running on the PC opens a socket to receive data from the ESP32-C6 and calculates the average throughput over a 50-second window. Power consumption is measured using a Keysight N6705B DC Power Analyzer, which also powers the ESP32-C6. For most test conditions, power is averaged over a 10-second window. In the case of a 100-second transmission period, both throughput and power consumption are measured over a 200-second window to capture long-term behavior.

Throughput as a function of current consumption was evaluated across different duty cycle ratios, with transmission periods of 100 ms, 250 ms  1 s, and 100 s. 