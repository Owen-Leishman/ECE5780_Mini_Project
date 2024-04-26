# Demonstration Neural Interface Hardware
>[!warning]
>This is a demonstration device, this hardware is relatively untested and not gauranteed to be safe! This is not designed live test subjects!

>[!note]
>Made with KiCad version 7.0,  [Kicad Download](https://www.kicad.org/download/)

The hardware for the demonstration neural interface.

## Tasks
- [ ] Test BQ51013BRHLR wirless power IC.
- [ ] Add voltage divider/modify signal path for onboard ADC.
- [ ] Find replacement for AD8067ARTZ operational amplifier used in the current source.
- [ ] Find issues with MCP3464 ADC (may have been ESD)

# Assembled Device
![Manufactured device](Figures/manufactured_device.jpg)
>[!warning]
>This device uses BGA components and is not trivial to solder. Due to the large amount of surface mount components on the device it is recomended to use a stencil and a reflow oven for soldering.

## Current Source Fix
![Current Source Fix](Figures/current_source_fix.jpg)
The AD8067ARTZ was broken (reason still unknown) so a LF353N was used to replace it for testing.

## Onboard ADC Fix
![ Onboard ADC Fix](Figures/onboard_adc_fix.jpg)
The MCP3464RT ADC broke so the ESP32's onboard ADC was used. The onboard ADC can not measure voltages larger than ~200mV so voltage dividers were added to divide all of the signals into that range, 4.7K and 180R resistors were used to create the voltage dividers. This creates a lot of noise and distortion in the signals. Due to this onboard ADC will not be used or supported in future revisions.

# Schematics
## Top Level Schematic
![Top level schematic](Figures/Neural_Interface.svg)
Schematic for the ESP32 S2-mini and supporting circuitry which is used to control the device. Jumpers are added between the ESP32's gpio and board signals to make it possible to rerout signals if there is a conflict, or prevent interference for analog signals if the onboard ADC is not being used. The temperature sensor is also on this schematic.

## Analog Frontend Schematic
![Analog frontend schematic](Figures/Neural_Interface-Analog_Frontend.svg)
Schematic for the analog frontend used to measure "neural" signals and output current mode stimulation. The differential input signals are fed into voltage dividers to ensure that the maximum voltage that reaches the system is ±3.3V, and to create a 1MΩ input impedance. Instrumentation amplifiers are used to amplify the differential signals (satureates at ±1V but is tolerant fo ±9V) and convert them to single ended signals (0-3.3V) centered around Vref (1.6V). The outut of the instrumentation amplifiers is fed into a buffered opamp with a shotkey diode to prevent negative voltages from reaching the ADC when the instrumentation amplifiers get saturated with a negative signal (Ie. an input of -9V), this opamp will likely be removed in future revisions. Low pass filterers and decoupling capacitors are implemented throughout the design to reduce noise.

The current mode stimulation is created using an advanced howland current source implementation based on an [article by Nick Jiang from Analog Devices](https://www.analog.com/en/resources/analog-dialogue/articles/a-large-current-source-with-high-accuracy-and-fast-settling.html). The current sources is controlled by using both DACs on the ESP32, one DAC is used for creating positive currents while the other is used to create negative curents. The maximum output current is ±1mA when 3.3V is supplied by one of the DACs. The maximum probe impedance the current source can handle is 9kΩ, since the stimulation voltage is ±9V. A capacitor is added in series to the output current to prevent continous stimulation in case of a device malfunction (this capacitor was shorted to test the current source at low frequencies).

## Power Manegement Schematic
![Power manegement schematic](Figures/Neural_Interface-Power_Manegement.svg)
Schematic for the power manegement circuitry which supplies power to the device and maneges the battery. The device can be powered from using wireless charging, USB type C, or the onboard battery. The BQ51013BRHLR wireless charging IC automatically selects if the BQ25185DLHR battery charging IC will be suplied with usb power or wireless power based on which one is present. The battery charging IC controlls the battery charge rate, and charge and discharge threshold and determines if the voltage regulators for the rest of the system will be supplied with battery power or the power selected by the wireless charging IC. A BQ27427YZFR is placed in series with the battery to measure battery life.

There are three voltage regulators on the device. A buck converter supplies Vsys (+3.3V) to power the ESP32 and TMP117, it is always on when voltage is present. A differential boost converter suplies Vstim (±9V) which poweres the constant current source and is controlled by the ESP32. The rest of the analog frontend is powered with a differential linear dropout regulator (LDO) which is powered from Vstim and reduces the supply noise caused by the switching regulators creating ±3.3V, this is not very effecient and will likely be changed in future revisions. The LDO is always on whenever Vstim is present.

# Layout

>[!note]
>These figures are for reference, there may be some visual issues due to how KiCad exports SVG files.

## Top Layout
![Top Layout](Figures/Neural_Interface-layout_top.svg)

## Bottom Layout
![Bottom Layout](Figures/Neural_Interface-layout_bottom.svg)

## 3D Render
![3D render](Figures/Neural_Interface_Render.png)

# Ordering
The design is layed out as a two layer board following [JLC PCB's](https://jlcpcb.com/) constraints, and all components are placed on the top layer. To order zip the [NSD_V1-0](Hardware/NSD_V1-0) directory and submit it to JLC PCB. A stencil can also be ordered from JLC PCB.

All of the parts other than the battery and battery connector can be ordered from [Mouser Electronics](https://www.mouser.com/) and listed [here](Hardware/MouserPartsOrder.xls). The battery and battery connector can be ordered from [DigiKey](https://www.digikey.com/) and are listed [here](Hardware/DigiKeyPartsOrder.xlsx). The [BOM](Hardware/BOM.csv) does not reference passive parts with manufacturer names. Additionaly 2.54mm headers are not included in either of these lists.



