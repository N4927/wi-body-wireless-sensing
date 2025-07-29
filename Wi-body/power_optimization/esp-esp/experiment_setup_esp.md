### **Experiment ESP32-C6 to ESP32-C6 setup:**

This setup involves two ESP32-C6 modules communicating directly over Wi-Fi at a fixed distance of approximately 50 cm. One module is configured as a Wi-Fi access point (AP) and is connected to a PC via USB for serial monitoring. The second module acts as a Wi-Fi client and is powered and analyzed using a Keysight N6705B DC Power Analyzer.

The AP firmware calculates throughput by computing the average number of bytes received over a 50-second window. These results are printed in real time to the ESP log using `esp_log`.

Power consumption on the client is measured with the Keysight analyzer. In standard test cases, power is averaged over 10-second windows. For the 100-second transmission case, both throughput and power are measured over a 200-second window to assess extended behavior.

Throughput versus current consumption was analyzed for different duty cycle ratios, with transmission periods of 100 ms, 1 s, and 100 s.

After completing all three test cases, the roles of the two ESP32-C6 modules were switched—reconfiguring the client as the AP and vice versa—in order to measure the power consumption of the AP and ensure a complete evaluation of both roles under identical conditions.