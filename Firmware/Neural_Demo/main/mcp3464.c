

#include <stdint.h>
#include "esp_err.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "mcp3464.h"


#define MCP3464_MCLK    5
#define MCP3464_IRQ     6
#define MCP3464_SDO     7
#define MCP3464_SDI     8
#define MCP3464_SCK     9
#define MCP3464_CS      10
#define MCP3464_INPUT_DELAY_NS 0

#define MCP3464_ADDR 0b01 

#define MCP3464_CONVERSION_START    0b101000
#define MCP3464_STANDBY_FAST        0b101100
#define MCP3464_SHUTDOWN_FAST       0b110000
#define MCP3464_FULL_SHUTDOWN       0b110100
#define MCP3464_FULL_RESET          0b111000

#define MCP3464_STATIC_READ         0b000001 // ADDRESS[5:2] CMD [1:0]
#define MCP3464_INCREMENTAL_WRITE   0b000010 // ADDRESS[5:2] CMD [1:0]
#define MCP3464_INCREMENTAL_READ    0b000011 // ADDRESS[5:2] CMD [1:0]

#define MCP3464_ADCDATA_ADDR    0x0 // 4/16/32 bits
#define MCP3464_CONFIG0_ADDR    0x1 // 8  bits
#define MCP3464_CONFIG1_ADDR    0x2 // 8  bits
#define MCP3464_CONFIG2_ADDR    0x3 // 8  bits
#define MCP3464_CONFIG3_ADDR    0x4 // 8  bits
#define MCP3464_IRQ_ADDR        0x5 // 8  bits
#define MCP3464_MUX_ADDR        0x6 // 8  bits
#define MCP3464_SCAN_ADDR       0x7 // 24 bits
#define MCP3464_TIMER_ADDR      0x8 // 24 bits
#define MCP3464_OFSETCAL_ADDR   0x9 // 24 bits
#define MCP3464_GAINCAL_ADDR    0xA // 24 bits
#define MCP3464_LOCK_ADDR       0xD // 8  bits
#define MCP3464_CRCCFG_ADDR     0xF // 16 bits

static const char *TAG = "MCP3464";





esp_err_t mcp3464_init(mcp3464_conf_t *cfg, mcp3464_context_t** out_ctx){

    esp_err_t err = ESP_OK;

    mcp3464_context_t* ctx = (mcp3464_context_t*)malloc(sizeof(mcp3464_context_t));
    if(!ctx){
        return ESP_ERR_NO_MEM;
    }

    *ctx = (mcp3464_context_t) {
        .cfg = *cfg,
    };

    spi_device_interface_config_t devcfg = {
        .command_bits = 8,
        .clock_speed_hz = ctx->cfg.clock_speed_hz,
        .mode  = 0,
        .spics_io_num = MCP3464_CS,
        .queue_size = 1,
        .input_delay_ns = MCP3464_INPUT_DELAY_NS,
    };

    err = spi_bus_add_device(ctx->cfg.host, &devcfg, &ctx->spi);
    if (err != ESP_OK){
        goto cleanup;
    }


    mcp3464_write(ctx, MCP3464_CONFIG0_ADDR, 0b10100011, 8);
    mcp3464_write(ctx, MCP3464_CONFIG1_ADDR, 0b11001100, 8); // set conversion period
    mcp3464_write(ctx, MCP3464_CONFIG2_ADDR, 0b10001001, 8); // sets adc gain
    mcp3464_write(ctx, MCP3464_CONFIG3_ADDR, 0b11000000, 8);
    mcp3464_write(ctx, MCP3464_IRQ_ADDR, 0b00000010, 8);
        //mcp3464_write(ctx, MCP3464_MUX_ADDR, 0b00001000, 8); // sets VIN+ to channel 0 and VIN- to gnd
    mcp3464_write(ctx, MCP3464_SCAN_ADDR, 0b000000000000100100011, 24); // scan mode perform single ended measuements on channels 0-3
    mcp3464_write(ctx, MCP3464_TIMER_ADDR, 0x00, 24); // delay between scan operations

    uint8_t data_check;

    mcp3464_read(ctx, MCP3464_CONFIG0_ADDR, &data_check, 8);
    if(data_check != 0b10100011){
        ESP_LOGE(TAG, "configuration readback error %d", data_check);
    }else{
        ESP_LOGI(TAG, "configuration readback %d", data_check);
    }

    mcp3464_read(ctx, MCP3464_CONFIG1_ADDR, &data_check, 8);
    if(data_check != 0b11001100){
        ESP_LOGE(TAG, "configuration readback error %d", data_check);
    }else{
        ESP_LOGI(TAG, "configuration readback %d", data_check);
    }

    mcp3464_read(ctx, MCP3464_CONFIG2_ADDR, &data_check, 8);
    if(data_check != 0b10001001){
        ESP_LOGE(TAG, "configuration readback error %d", data_check);
    }else{
        ESP_LOGI(TAG, "configuration readback %d", data_check);
    }

    mcp3464_read(ctx, MCP3464_CONFIG3_ADDR, &data_check, 8);
    if(data_check != 0b11000000){
        ESP_LOGE(TAG, "configuration readback error %d", data_check);
    }else{
        ESP_LOGI(TAG, "configuration readback %d", data_check);
    }


    *out_ctx = ctx;
    return ESP_OK;

cleanup:
    if(ctx->spi){
        spi_bus_remove_device(ctx->spi);
        ctx->spi = NULL;
    }

    free(ctx);

    return err;
}


esp_err_t mcp3464_write(mcp3464_context_t* ctx, uint8_t addr, uint32_t data, uint8_t bits){

    esp_err_t err;
    err = spi_device_acquire_bus(ctx->spi,portMAX_DELAY);
    if(err != ESP_OK){
        return err;
    }

    spi_transaction_t t = {
        .cmd = (MCP3464_ADDR << 6) | (addr << 2) | MCP3464_INCREMENTAL_WRITE,
        .length = bits,
        .flags = SPI_TRANS_USE_TXDATA,
        .tx_data = {data},
        .user = ctx,
    };

    err = spi_device_polling_transmit(ctx->spi, &t);

    // MAY NEED TO ADD DELAY!!!!

    spi_device_release_bus(ctx->spi);
    return err;
}


esp_err_t mcp3464_read(mcp3464_context_t* ctx, uint8_t addr, uint8_t* data, uint8_t bits){

    spi_transaction_t t = {
        .cmd = (MCP3464_ADDR << 6) | (addr << 2) | MCP3464_INCREMENTAL_READ,
        .rxlength = bits,
        .flags = SPI_TRANS_USE_RXDATA,
        .user = ctx,
        .length = bits,
    };

    esp_err_t err = spi_device_polling_transmit(ctx->spi, &t);
    if(err != ESP_OK){
        return err;
    }

    *data = t.rx_data[0];
    return ESP_OK;
}

esp_err_t mcp3464_adc_read_raw(mcp3464_context_t* ctx, uint16_t* data){

    spi_transaction_t t = {
        .cmd = (MCP3464_ADDR << 6) | MCP3464_INCREMENTAL_READ,
        .rxlength = 16,
        .flags = SPI_TRANS_USE_RXDATA,
        .user = ctx,
        .length = 16, 
    };

    esp_err_t err = spi_device_polling_transmit(ctx->spi, &t);
    if(err != ESP_OK){
        return err;
    }

    *data = t.rx_data[0];
    return ESP_OK;    

}
