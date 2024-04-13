#pragma once

#include <stdint.h>
#include "driver/i2c_master.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

struct tmp117_t {
    i2c_master_dev_handle_t i2c_dev;    // I2C device handle
    uint8_t *buffer;
    int timeout_ms;
};

typedef struct{
    bool mode;
    bool alert_mode;
    bool alert_polarity;
    bool dr_al_select;
    uint8_t addr;
    uint8_t averaging;
    uint16_t conversion_period;
    uint32_t scl_speed_hz;
    int timeout_ms;
} tmp117_conf_t;

typedef struct tmp117_t tmp117_t;

typedef struct tmp117_t *tmp117_handle_t;

esp_err_t tmp117_init(i2c_master_bus_handle_t i2c, tmp117_conf_t *config, tmp117_handle_t *tmp117_handle, int *actual_conversion);
esp_err_t tmp117_write(tmp117_handle_t tmp117_handle, const uint8_t *data, uint8_t size);
esp_err_t tmp117_read(tmp117_handle_t tmp117_handle, uint8_t addr, uint8_t *data, uint8_t size);
esp_err_t tmp117_data_available(tmp117_handle_t tmp117_handle, bool *data_available);
esp_err_t tmp117_read_temp_raw(tmp117_handle_t tmp117_handle, uint16_t *temperature);
esp_err_t tmp117_read_temp_raw_blocking(tmp117_handle_t tmp117_handle, uint16_t *temperature);
int16_t tmp117_convert_to_c(uint16_t raw_temperature);
int16_t tmp117_convert_to_mc(uint16_t raw_temperature);

#ifdef __cplusplus
}
#endif

#ifndef TMP117_DEFAULT_ADDR
#define TMP117_DEFAULT_ADDR 0b1001000
#endif 