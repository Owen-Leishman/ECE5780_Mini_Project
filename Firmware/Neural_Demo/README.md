# Neural Demo Device Firmware
>[!warning]
>Due to the limited performance of the onboard ADC this firmware will not recieve updates.

This firmware is written to showcase the basic working principles of the demonstration device such as wifi communication, ADC measurements, temperature measurements, and current mode stimulation. The software used to interface with the device running this firmware can be found [here](Software).

FreeRTOS is used to create tasks to handle Wifi communication.

The ESP32's onboard ADC is used in one-shot mode to measure the output of the analog frontend as well as the actual stimulation current. Since there is a significant amount of noise in these measurements each measurment is an average of 32 readings.

The ESP32's I2C bus is used to communicate to the TMP117 and record temperature data, it is also connected to the BQ27427YZFR which has not been implemented in firmware yet.

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

