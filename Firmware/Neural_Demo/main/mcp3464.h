#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    spi_host_device_t host;
    gpio_num_t cs_io;
    gpio_num_t miso_io;
    uint32_t clock_speed_hz;
} mcp3464_conf_t;

typedef struct mcp3464_context_t* mcp3464_handle_t;

struct mcp3464_context_t {
    mcp3464_conf_t cfg;
    spi_device_handle_t spi;
    QueueHandle_t irq_queue;
};

typedef struct mcp3464_context_t mcp3464_context_t;



esp_err_t mcp3464_init(mcp3464_conf_t *cfg, mcp3464_context_t** out_ctx);
esp_err_t mcp3464_write(mcp3464_context_t* ctx, uint8_t addr, uint8_t data);
esp_err_t mcp3464_write_buff(mcp3464_context_t* ctx, uint8_t addr, uint8_t *data, uint8_t length);
esp_err_t mcp3464_read(mcp3464_context_t* ctx, uint8_t addr, uint32_t* data, uint8_t bits);
esp_err_t mcp3464_adc_read_raw(mcp3464_context_t* ctx, uint16_t* data);

esp_err_t mcp3464_start_conversion(mcp3464_context_t* ctx);
//static void mcp3464_irq(void* arg);

#ifdef __cplusplus
}
#endif