#include <stdio.h>
#include "esp_log.h"
#include "esp_check.h"
#include "driver/gpio.h"


// Local includes
#include "tmp117.h"
#include "mcp3464.h"


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
#define MCP3464_CS      10

//****************Other Definitions*******************

// I2C
#define PORT_NUMBER -1
#define I2C_CLK_FREQ 400000    

#define SPI_HOST 1
#define SPI_CLK_FREQ 10000000

static const char *TAG = "MAIN";

void app_main(void)
{

    // Turn on high voltage supply
    gpio_reset_pin(HIGH_VOLTAGE_EN);
    gpio_set_direction(HIGH_VOLTAGE_EN, GPIO_MODE_OUTPUT);
    gpio_set_level(HIGH_VOLTAGE_EN, 1);

    esp_err_t ret;

    ESP_LOGI(TAG, "Initializing bus SPI%d...", SPI_HOST + 1);

    spi_bus_config_t buscfg = {
        .miso_io_num = MCP3464_SDO,
        .mosi_io_num = MCP3464_SDI,
        .sclk_io_num = MCP3464_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(SPI_HOST, &buscfg, SPI_DMA_CH_AUTO));


    mcp3464_conf_t mcp3464_conf = {
        .cs_io = MCP3464_CS,
        .host = SPI_HOST,
        .miso_io = MCP3464_SDO,
        .clock_speed_hz = SPI_CLK_FREQ,
    };

    mcp3464_handle_t mcp3464_handle;

    ESP_LOGI(TAG, "Initializing MCP3464...");
    ESP_ERROR_CHECK(mcp3464_init(&mcp3464_conf, &mcp3464_handle));

    ESP_LOGI(TAG, "Initialization Complete");

    uint16_t adc_val;

    while(1){
        vTaskDelay(10);
        ESP_ERROR_CHECK(mcp3464_adc_read_raw(mcp3464_handle, &adc_val));
        ESP_LOGI(TAG, "ADC Val: %d", adc_val);
    }


//    i2c_master_bus_config_t i2c_bus_config = {
//        .clk_source = I2C_CLK_SRC_DEFAULT,
//        .i2c_port = PORT_NUMBER,
//        .scl_io_num = I2C_SCL,
//        .sda_io_num = I2C_SDA,
//        .glitch_ignore_cnt = 7,
//    };
//
//    i2c_master_bus_handle_t bus_handle;
//
//    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &bus_handle));
//
//    tmp117_conf_t tmp117_conf = {
//        .addr = TMP117_DEFAULT_ADDR,
//        .alert_mode = 0,
//        .alert_polarity = 0,
//        .averaging = 8,
//        .conversion_period = 125,
//        .dr_al_select = 0,
//        .mode = 0,
//        .scl_speed_hz = I2C_CLK_FREQ,
//        .timeout_ms = -1,
//    };
//
//    tmp117_handle_t tmp117_handle;
//
//    int conversion_rate;
//
//    ESP_ERROR_CHECK(tmp117_init(bus_handle, &tmp117_conf, &tmp117_handle, &conversion_rate));
//
//    ESP_LOGI(TAG, "tmp117 conversion rate: %d", conversion_rate);
//    
//    uint16_t current_temperature;
//
//    while(1){
//      
//        ESP_ERROR_CHECK_WITHOUT_ABORT(tmp117_read_temp_raw_blocking(tmp117_handle, &current_temperature));
//        ESP_LOGI(TAG, "temperature = %d", tmp117_convert_to_mc(current_temperature));
//
//    }

}


