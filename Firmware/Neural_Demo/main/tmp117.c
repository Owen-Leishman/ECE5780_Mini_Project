#include <string.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "tmp117.h"
#include "esp_types.h"
#include "esp_check.h"


static const char *TAG = "TMP117";

#define TMP117_TEMP     0x00
#define TMP117_CONFIG   0x01
#define TMP117_LIMITL   0x02
#define TMP117_LIMITH   0x03
#define TMP117_ID       0x0F

#define TMP117_DR       0b1<<5

#define TMP117_MAX_COMMAND_LEN 10

/**
 * @brief Initialize the TMP117 digital temperature sensor
 * 
 * @param i2c the i2c instance the TMP117 is on
 * @param config the configuration for the TMP117
 * @param tmp117_handle the device handle for the TMP117
 * @param actual_conversion the actual conversion period set for the TMP117
 * @return esp_err_t 
 */
esp_err_t tmp117_init(i2c_master_bus_handle_t i2c, tmp117_conf_t *config, tmp117_handle_t *tmp117_handle, int *actual_conversion){

    esp_err_t ret = ESP_OK;
    uint8_t average_map;



    if(config->averaging > 16){
        average_map = 0b11;
    }else if(config->averaging > 8){
        average_map = 0b10;
    }else if(config->averaging > 0){
        average_map = 0b01;
    }else{
        average_map = 0b00;
    }

    uint8_t conversion_map = 0;

    if(config->conversion_period >= 16000){
        conversion_map = 0b111;
        *actual_conversion = 16000;
    }else if(config->conversion_period >= 8000){
        conversion_map = 0b110;
        *actual_conversion = 8000;
    }else if(config->conversion_period >= 4000){
        conversion_map = 0b101;
        *actual_conversion = 4000;
    }else if(config->conversion_period >= 1000){
        conversion_map = 0b100;
        *actual_conversion = 1000;
    }else if(average_map == 0b11){
        conversion_map = 0b000;
        *actual_conversion = 1000;
    }else if(average_map == 0b10){
        conversion_map = 0b000;
        *actual_conversion = 500;
    }else if(average_map == 0b01){
        if(config->conversion_period >= 500){
            conversion_map = 0b011;
            *actual_conversion = 500;
        }else if(conversion_map >= 250){
            conversion_map = 0b010;
            *actual_conversion = 250;
        }else{
            conversion_map = 0b000;
            *actual_conversion = 125;
        }
    }else{
        if(config->conversion_period >= 500){
            conversion_map = 0b011;
            *actual_conversion = 500;
        }else if(conversion_map >= 250){
            conversion_map = 0b010;
            *actual_conversion = 250;
        }else if(conversion_map >= 125){
            conversion_map = 0b001;
            *actual_conversion = 125;
        }else{
            conversion_map = 0b000;
            *actual_conversion = 15;
        }
    }

    uint8_t config_high = (config->mode << 3)|(config->mode << 2)|(conversion_map >> 1);
    uint8_t config_low = (conversion_map << 7)|(average_map << 5)|(config->alert_mode << 4)|(config->alert_polarity << 3)|(config->dr_al_select << 2) ;
    uint8_t tmp117_configuration[] = {(uint8_t) TMP117_CONFIG, config_high, config_low};

    tmp117_handle_t out_handle;
    out_handle  = (tmp117_handle_t)calloc(1, sizeof(tmp117_handle_t));
    ESP_GOTO_ON_FALSE(out_handle, ESP_ERR_NO_MEM, err, TAG, "no memory for i2c eeprom device");

    i2c_device_config_t i2c_dev_conf = {
        .scl_speed_hz = config->scl_speed_hz,
        .device_address = config->addr,
    };

    if (out_handle->i2c_dev == NULL){
        ESP_GOTO_ON_ERROR(i2c_master_bus_add_device(i2c, &i2c_dev_conf, &out_handle->i2c_dev), err, TAG, "i2c new bus failed");
    }

    out_handle->timeout_ms = config->timeout_ms;

    out_handle->buffer = (uint8_t*)calloc(1, TMP117_MAX_COMMAND_LEN);
    ESP_GOTO_ON_FALSE(out_handle->buffer, ESP_ERR_NO_MEM, err, TAG, "no memory for i2c output buffer");

    ESP_GOTO_ON_FALSE(tmp117_write(out_handle, &tmp117_configuration, 3), ESP_ERR_INVALID_RESPONSE, err, TAG, "could not communicate with TMP117");

    return ESP_OK;

err:

    if (out_handle && out_handle->i2c_dev){
        i2c_master_bus_rm_device(out_handle->i2c_dev);
    }

    free(out_handle);
    return ret;

}

/**
 * @brief write data to the tmp117
 * 
 * @param tmp117_handle the device handle for the TMP117
 * @param data the data to transmit
 * @param size the length of the data array
 * @return esp_err_t 
 */
esp_err_t tmp117_write(tmp117_handle_t tmp117_handle, uint8_t *data, uint8_t size){

    return  i2c_master_transmit(tmp117_handle->i2c_dev, tmp117_handle->buffer, size, tmp117_handle->timeout_ms);

}


/**
 * @brief read data from the tmp117
 * 
 * @param tmp117_handle the device handle for the TMP117
 * @param addr the tmp117 address to read
 * @param data where the data is written
 * @param size the length of the data to recieve
 * @return esp_err_t 
 */
esp_err_t tmp117_read(tmp117_handle_t tmp117_handle, uint8_t addr, uint8_t *data, uint8_t size){

    return i2c_master_transmit_receive(tmp117_handle->i2c_dev, addr, 1, data, size, tmp117_handle->timeout_ms);

    
}

/**
 * @brief Returns if the there is a new temperature reading available
 * 
 * @param tmp117_handle the device handle for the TMP117
 * @param data_available 1: new reading available, 0: no new reading
 * @return esp_err_t 
 */
esp_err_t tmp117_data_available(tmp117_handle_t tmp117_handle, bool *data_available){

    uint8_t config_addr = TMP117_CONFIG;
    uint8_t data[2];


    ESP_RETURN_ON_ERROR(tmp117_read(tmp117_handle, &config_addr,  data, 2), TAG, "could not communicate with TMP117");

    if((data[0] & TMP117_DR) != 0){
        *data_available = 1;
    }else{
        *data_available = 0;
    }

    return ESP_OK;

}

/**
 * @brief read the raw temperature of TMP117
 * 
 * @param tmp117_handle the device handle for the TMP117
 * @param temperature the raw temperature
 * @return esp_err_t 
 */
esp_err_t tmp117_read_temp_raw(tmp117_handle_t tmp117_handle, int16_t *temperature){
    uint8_t data[2];
    uint8_t temp_addr = TMP117_TEMP;


    ESP_RETURN_ON_ERROR(tmp117_read(tmp117_handle,&temp_addr, data, 2), TAG, "could not communicate with TMP117");

    *temperature = (data[0] << 8) | data[1];

    return ESP_OK;
}

/**
 * @brief convert raw temperature reading to degrees c
 * 
 * @param raw_temperature tmp117 raw temperature reading
 * @return int16_t the temperature in c
 */
int16_t tmp117_convert_to_c(int16_t raw_temperature){
    return (raw_temperature * 78125)/1000;
}

/**
 * @brief convert raw temperature reading to milli-degrees c
 * 
 * @param raw_temperature tmp117 raw temperature reading
 * @return int16_t the temperature in mc
 */
int16_t tmp117_convert_to_mc(int16_t raw_temperature){
    return raw_temperature * 78125;
}
