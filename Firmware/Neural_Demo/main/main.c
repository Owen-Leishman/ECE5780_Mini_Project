#include <stdio.h>
#include "esp_log.h"
#include "esp_check.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

// Local includes
#include "tmp117.h"
#include "mcp3464.h"
#include "wifi.h"
#include "udp.h"

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

// SPI
#define SPI_ADC_HOST 1
#define SPI_CLK_FREQ 20000000

// MCP3464
#define MCP3464_CHANNEL_0       0b0000
#define MCP3464_CHANNEL_1       0b0001
#define MCP3464_CHANNEL_2       0b0010
#define MCP3464_CHANNEL_3       0b0011

// How many readings are averaged for every adc value
#define AVERAGING   32

// Transmission parameters
#define PAYLOAD_LENGTH      405 
#define PAYLOAD_SAMPLES     200

static const char *TAG = "MAIN";

static char payload[PAYLOAD_LENGTH];


void app_main(void)
{

    // Initialize the nvs flash (used for wifi)
    esp_err_t ret = nvs_flash_init();
    if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND){
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    // Initialize wifi
    wifiInit();

    // Make sure current output is set to zero by default
    gpio_reset_pin(STIM_P);
    gpio_set_direction(STIM_P, GPIO_MODE_OUTPUT);
    gpio_set_level(STIM_P, 0);

    gpio_reset_pin(STIM_N);
    gpio_set_direction(STIM_N, GPIO_MODE_OUTPUT);
    gpio_set_level(STIM_N, 0);

    // Turn on high voltage supply
    gpio_reset_pin(HIGH_VOLTAGE_EN);
    gpio_set_direction(HIGH_VOLTAGE_EN, GPIO_MODE_OUTPUT);
    gpio_set_level(HIGH_VOLTAGE_EN, 1);    


    // Initialize SPI bus
    ESP_LOGI(TAG, "Initializing bus SPI%d...", SPI_ADC_HOST + 1);

    spi_bus_config_t buscfg = {
        .miso_io_num = MCP3464_SDO,
        .mosi_io_num = MCP3464_SDI,
        .sclk_io_num = MCP3464_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(SPI_ADC_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // Initialize ADC
    mcp3464_conf_t mcp3464_conf = {
        .cs_io = MCP3464_CS,
        .host = SPI_ADC_HOST,
        .miso_io = MCP3464_SDO,
        .clock_speed_hz = SPI_CLK_FREQ,
    };

    mcp3464_handle_t mcp3464_handle;

    ESP_ERROR_CHECK(mcp3464_init(&mcp3464_conf, &mcp3464_handle));


    // Configure I2C
    i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = PORT_NUMBER,
        .scl_io_num = I2C_SCL,
        .sda_io_num = I2C_SDA,
        .glitch_ignore_cnt = 7,
    };
    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &bus_handle));


    // Configure the TMP117
    tmp117_conf_t tmp117_conf = {
        .addr = TMP117_DEFAULT_ADDR,
        .alert_mode = 0,
        .alert_polarity = 0,
        .averaging = 8,
        .conversion_period = 125,
        .dr_al_select = 0,
        .mode = 0,
        .scl_speed_hz = I2C_CLK_FREQ,
        .timeout_ms = -1,
    };
    tmp117_handle_t tmp117_handle;
    int conversion_rate;
    ESP_ERROR_CHECK(tmp117_init(bus_handle, &tmp117_conf, &tmp117_handle, &conversion_rate));
    ESP_LOGI(TAG, "tmp117 conversion rate: %d", conversion_rate);
    uint16_t current_temperature;
    

    // Configure the onboard ADC
    adc_oneshot_unit_handle_t adc2_handle;
    adc_oneshot_unit_init_cfg_t init_config2 = {
        .unit_id = ADC_UNIT_2,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config2, &adc2_handle));
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,        // makes it possible to measure values larger than vref
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc2_handle, ADC_CHANNEL_0, &config));

    // Calibrate the onboard ADC
    adc_cali_handle_t adc2_cali_chan0_handle = NULL;
    adc_cali_line_fitting_config_t cali_config_2 = {
        .unit_id = ADC_UNIT_2,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    adc_cali_create_scheme_line_fitting(&cali_config_2, &adc2_cali_chan0_handle); 
    

    uint32_t adc_val;  

    // Start first conversion, interrupt starts conversions afterwards
    mcp3464_start_conversion(mcp3464_handle);

    // Create the task to revieve messages over UDP
    xTaskCreate(udp_recieve_task, "udp_client", 4096, NULL, 5, NULL);

    while(1){

        // Set the start byte of the payload
        payload[0] = 'S';

        int samples = 0;
        int channel_0_samples = 0;
        int channel_1_samples = 0;
        int channel_2_samples = 0;
        int channel_3_samples = 0;
        int channel_4_samples = 0;

        int adc_val;
        int adc_onboard_val = 0;

        int adc_val_cal = 0;


        while(samples < PAYLOAD_SAMPLES){
            if(xQueueReceive(mcp3464_handle->irq_queue, &adc_val, portMAX_DELAY)){
                uint8_t channel = (adc_val >> 24) & 0b1111;
                


                if((samples == 0) && (channel != MCP3464_CHANNEL_0)){
                    channel = 200;
                }

                switch(channel){
                    case MCP3464_CHANNEL_0:
                        ESP_LOGI(TAG,"ADC channel 0 Reading: %d", (uint16_t)(adc_val & 0b1111111111111111));
                        payload[3 + (channel_0_samples * 2)]      = adc_val >> 8;
                        payload[3 + (channel_0_samples * 2) + 1]  = adc_val;
                        channel_0_samples += 1;
                        samples += 1;
                        break;
                    case MCP3464_CHANNEL_1:
                        payload[3 + (PAYLOAD_SAMPLES * 2 / 5) + (channel_1_samples * 2)]      = adc_val >> 8;
                        payload[3 + (PAYLOAD_SAMPLES * 2 / 5) + (channel_1_samples * 2) + 1]  = adc_val;
                        channel_1_samples += 1;
                        samples += 1;
                        break;
                    case MCP3464_CHANNEL_2:
                        payload[3 + 2 * (PAYLOAD_SAMPLES * 2 / 5) + (channel_2_samples * 2)]      = adc_val >> 8;
                        payload[3 + 2 * (PAYLOAD_SAMPLES * 2 / 5) + (channel_2_samples * 2) + 1]  = adc_val;
                        channel_2_samples += 1;
                        samples += 1;
                        break;
                    case MCP3464_CHANNEL_3:
                        
                        payload[3 + 3 * (PAYLOAD_SAMPLES * 2 / 5) + (channel_3_samples * 2)]      = adc_val >> 8;
                        payload[3 + 3 * (PAYLOAD_SAMPLES * 2 / 5) + (channel_3_samples * 2) + 1]  = adc_val;
                        channel_3_samples += 1;
                        samples += 1;

                        // Do onboard adc reading
                        uint32_t adc_val_4 = 0;

                        // Average the ADC readings
                        for(int i = 0; i < AVERAGING; i++){

                            adc_oneshot_read(adc2_handle, ADC_CHANNEL_0, &adc_onboard_val);
                            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc2_cali_chan0_handle, adc_onboard_val, &adc_val_cal));
                            adc_val_4 += adc_val_cal;
                
                        }

                        // Divide the ADC readings by 32
                        adc_val_4 = adc_val_4 >> 5;

                        payload[3 + 4 * (PAYLOAD_SAMPLES * 2 / 5) + (channel_4_samples * 2)]      = adc_val_4 >> 8;
                        payload[3 + 4 * (PAYLOAD_SAMPLES * 2 / 5) + (channel_4_samples * 2) + 1]  = adc_val_4;
                        channel_4_samples += 1;
                        samples += 1;

                        break;
                    default:
                        ESP_LOGE(TAG,"Unexpected ADC channel");
                }

                
            }
     
    
        }

        // Add stop bytes to payload (used for debugging)
        payload[PAYLOAD_LENGTH - 2] =  '\r';
        payload[PAYLOAD_LENGTH - 1] =  '\n';

        // Read the temperature
        ESP_ERROR_CHECK_WITHOUT_ABORT(tmp117_read_temp_raw_blocking(tmp117_handle, &current_temperature));
        ESP_LOGI(TAG, "temperature = %d", tmp117_convert_to_c(current_temperature));
        ESP_LOGI(TAG, "ESP ADC VAL: %d", adc_val);
        
        // Write the temperature to the payload
        payload[1] = current_temperature >> 8;
        payload[2] = current_temperature;

        // Transmit the payload
        xTaskCreate(udp_client_task, "udp_client", 4096, &payload, 5, NULL); 


    }


}


