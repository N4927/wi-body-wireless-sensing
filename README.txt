README:

Firmware:
The firmware ultimately consists of two code projects: AP (Access Point) and Station. To run a WBAN both are necessary. AP provides the wifi network, SSID and accepts connection upon a correct password is configured on the stations. Therefore one AP is necessary, stations can range from one to an indefinite number.

The projects on the repo are currently built on my computer. Meaning datapaths of linkerfiles and so on are configured to my computer. When using the code on another computer, the project has to be built from scratch (built has to be deleted and completely redone). I'd recommend creating a new project on the ESP-IDF in VScode and inserting the .c files in there. It's important to note that rebuilding projects resets certain config header files, those should be stored and the content copied into them again.

When running the firmware it's important to take the configuration of the code into consideration. In the .c files there are on top macros that configure/toggle certain features. (Such as whether benchmarking is enabled, but also whether UDP and or TCP transmissions are enabled.) It's adviced to study the macros and their functions beforehand. (especially the password/ssid/host address macros theyy're crucial for establishing connections) 


Hardware:
The design files have been created with KiCAD 8.0.6. They contain: component Library/schematics/layout files. 
It's important to note that when updating the layout from the schematics, e.g. when changes are made, the component descriptions on the silkscreen reappear. Also layout footprints are reverted into their 'original' state. This can be avoided by changing the library footprint. (E.g. the footprint of the Espressif MCU would include the antenna and would lead to the requirement of it being on the board and not 'sticking out of the edge'.) Eventually original footprints need to be redownloaded.