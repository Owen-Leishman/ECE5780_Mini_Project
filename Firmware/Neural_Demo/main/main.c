#include <stdio.h>
#include "esp_log.h"
#include "esp_check.h"


#include "tmp117.h"



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

//****************Other Definitions*******************

#define PORT_NUMBER -1
#define I2C_CLK_FREQ 100000    

static const char *TAG = "MAIN";

void app_main(void)
{

    i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = PORT_NUMBER,
        .scl_io_num = I2C_SCL,
        .sda_io_num = I2C_SDA,
        .glitch_ignore_cnt = 7,
    };

    i2c_master_bus_handle_t bus_handle;

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &bus_handle));

    tmp117_conf_t tmp117_conf = {
        .addr = TMP117_DEFAULT_ADDR,
        .alert_mode = 0,
        .alert_polarity = 0,
        .averaging = 8,
        .conversion_period = 125,
        .dr_al_select = 0,
        .mode = 0,
        .scl_speed_hz = I2C_CLK_FREQ,
        .timeout_ms = 500,
    };

    tmp117_handle_t tmp117_handle;

    int conversion_rate;

    ESP_ERROR_CHECK(tmp117_init(bus_handle, &tmp117_conf, &tmp117_handle, &conversion_rate));

    ESP_LOGI(TAG, "tmp117 conversion rate: %d", conversion_rate);
    
    int16_t current_temperature;

    while(1){
      
        ESP_ERROR_CHECK_WITHOUT_ABORT(tmp117_read_temp_raw(tmp117_handle, &current_temperature));
        ESP_LOGI(TAG, "temperature = %d", tmp117_convert_to_c(current_temperature));

    }

}


