# Neural Demo Device Firmware

This firmware is written to showcase the basic working principles of the demonstration device such as wifi communication, ADC measurements, temperature measurements, and current mode stimulation. The software used to interface with the device running this firmware can be found [here](Software).

FreeRTOS is used to create tasks to handle Wifi communication.

The ESP32's onboard ADC is used in one-shot mode to measure the actual stimulation current. Since there is a significant amount of noise in these measurements each measurment is an average of 32 readings.

The ESP32's I2C bus is used to communicate to the TMP117 and record temperature data, it is also connected to the BQ27427YZFR which has not been implemented in firmware yet.

The ESP32's SPI bus is used to comunicate to the MCP3464 which is used to measue the output of the analog frontend. The ESP32 uses a edge trigger interupt to read the IRQ line of the MCP3464 to determine if new data is availabe. The MCP3464 is not used for some demonstrations because it broke, the firmware to use the device without the MCP3464 can be found [here](https://github.com/Owen-Leishman/ECE5780_Mini_Project/tree/onboard_ADC) but will not recieve any updates.

This firmware is written for an ESP32 S2 and may not work for other ESP chips.

## Tasks
- [ ] Read the BQ27427YZFR for battery fuel measurements.
- [ ] Read the status of the wireless charger and battery control ICs.
- [ ] Add sleep and low power modes.
- [ ] Fix issue where readings are temporarily stoped when data is recieved over UDP.
- [ ] Measure stimulation accuracy and adjust accordingly.
- [ ] Remove use of floating points in stimulation code.

>[!note]
>Uses ESP IDF version 5.2.1

