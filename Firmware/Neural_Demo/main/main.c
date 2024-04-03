#include <stdio.h>
//#include <driver/gpio.h>
//#include <driver/gpio_etm.h>
//#include <driver/dedic_gpio.h>
//#include <driver/gpio_filter.h>

//#include <esp32s2/rom/gpio.h>
#include <gpio.h>


//****************Pin Definitions*******************

#define ADC_1   1
#define ADC_2   2
#define ADC_3   3
#define ADC_4   4
#define ADC_5   11

#define STIM_P  17
#define STIM_N  18

#define WIRELESS_STATUS 45
#define FUEL_STATUS     36
#define HIGH_VOLTAGE_EN 35
#define CHARGE_STAT_1   34
#define CHARGE_STAT_2   33

#define I2C_SCL 38
#define I2C_SDA 37

#define MCP3464_MCLK    5
#define MCP3464_IRQ     6
#define MCP3464_SDO     7
#define MCP3464_SDI     8
#define MCP3464_SCK     9
#define MCP3463_CS      10

void app_main(void)
{

    gpio_init(1,1);

    while(1){

        

    }

}


