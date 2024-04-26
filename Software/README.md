# Demonstration Software

This software is used to give a real-time demonstrate the neural interface. The software connects to the device over wifi and graphs the readings of the device in real time, and allows you to set the current stimulation waveform and amplitude.

![Software Example](/Figures/current_measurement_sine.png)
The channel 1-4 graphs show what voltages the device is measuring over time (in volts). The waveform dropdown selects if the device should output a sine wave or a square wave using the current source, and the amplitude slider selects the amplitude of the stimulation wave in miliamperes. The current measurement graph shows the measured stimulation current over time (in miliamperes).The x-axis of the graphs don't have tick marks to prevent the formating of the graphs to change every time they are updated. The temperature shows the measured temperature in degrees celcius.

This demonstration used a fixed load resistor connected between the current stimulation output and ground, and a signal generator creating a sine wave connected to channel 1. The other channles are left unconnected.

## Usage
>[!note]
>Made with [Matlab](https://www.mathworks.com/products/matlab.html) version 2022a 

To use this software you must have Matlab installed and change the [host IP address](https://github.com/Owen-Leishman/ECE5780_Mini_Project/blob/515b5f4717a34815e817afd9543e5ca668a39d97/Firmware/Neural_Demo/main/udp.c#L21C30-L21C43) in firmware and the [udp IP address](https://github.com/Owen-Leishman/ECE5780_Mini_Project/blob/515b5f4717a34815e817afd9543e5ca668a39d97/Software/neural_ui_udp.m#L6C32-L6C47) in the matlab script to your computers IP address. You must also update the [wifi credentials](https://github.com/Owen-Leishman/ECE5780_Mini_Project/blob/515b5f4717a34815e817afd9543e5ca668a39d97/Firmware/Neural_Demo/main/wifi.c#L16C1-L17C29) in the firmware to allow the device to connect to your router.

