# Demonstration Neural Interface
>[!warning]
>This is a demonstration device, this hardware is relatively untested and not gauranteed to be safe! This is not designed live test subjects!

>[!warning]
>Due to the limited performance of the onboard ADC the onboard ADC firmware will not recieve updates.

## Overview
This is a demonstration neural interface which is meant to explore how neural interfaces work.  This device can measure four differential signals at a time which can be used to measure nerve impulses and are tolerant of ±9V. It also has a current mode stimulation channel which can create ±1mA signals with 9 bits of resolution as a means to give feedback to the nerves. The stimulation channel can handle electrodes with up to 9kΩ. The device also has a temperature sensor to measure how much it heats up to ensure that it would be safe if implanted (at least temperature wise). The device can be controlled over wifi, and is fully battery powered.

![Manufactured device](/Figures/manufactured_device.jpg)

## Organization
- [Firmware](/Firmware/Neural_Demo/) contains the code that the device runs.
- [Hardware](/Hardware/) contains the design of the device.
- [Software](/Software/) contains the code used to interface with the device.

For information on how to use the device refer to the [software directory](/Software/).

For information on how to manufacture the device refer to the [hardware directory](/Hardware/).

## Results
The device has exceptional accuracy and can measure down to the uV using its discreate ADC.
![Low Voltage Test](/Figures/1mV_test.png)
This test was run using an external signal generator to create a 1mV peak-to-peak sine wave at 500mHz. (This test was also done with old software and firmware some of the ADC configuration has been changed).

The demonstration software makes it possible to measure all four channels in real time over wifi, and the measured stimulation current. The software also makes it possible to control the amplitude and waveform the current stimulation uses.
![Full sweet](/Figures/current_measurement_sine.png)
The discreate ADC broke so the ESP32's onboard ADC (which is much worse) was used for this test. An external signal generator was connected to channel 1 to create a 1V peak-to-peak sine wave, and an external load resistor was used to connect the stimulation output to ground.

# Design Overview
Here are is the top level design of the device, for detailed information on the desing refer to the [hardware directory](Hardware).

## Top Level Design
![Top level block diagram](/Figures/Top_Level.png)
The ESP32 microcontroller is used to control the whole device. The analog frontend is used to aquire data from the differential inputs, and output stimulation signals. The power distrobution block manages the battery and all of the supply voltages.

## Analog Frontend
![Analog frontend block diagram](/Figures/Analog_Frontend.png)

## Power Distrobution
![Power Distrobution block diagram](/Figures/Power_Distrobution.png)
