#include <stdint.h>
#include "esp_err.h"
#include <inttypes.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
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

#define MCP3464_CHANNEL_0       0b0000
#define MCP3464_CHANNEL_1       0b0001
#define MCP3464_CHANNEL_2       0b0010
#define MCP3464_CHANNEL_3       0b0011

static const char *TAG = "MCP3464";

/**
 * @brief MCP3464 IQR handler
 * 
 * @param arg 
 */
static void mcp3464_irq(void* arg){
    mcp3464_context_t* ctx = arg;
    uint32_t data;
    mcp3464_adc_read_raw(ctx, &data);

//    mcp3464_write(ctx, MCP3464_MUX_ADDR, 0b00001011); // chan 0
//    mcp3464_write(ctx, MCP3464_MUX_ADDR, 0b00011011); // chan 1
//   mcp3464_write(ctx, MCP3464_MUX_ADDR, 0b00101011); // chan 2
//    mcp3464_write(ctx, MCP3464_MUX_ADDR, 0b00111011); // chan 3

    mcp3464_start_conversion(ctx);
    xQueueSendFromISR(ctx->irq_queue, &data, NULL);

}

/**
 * @brief initialize the MCP3464
 * 
 * @param cfg MCP3464 configuration
 * @param out_ctx MCP3464 handle
 * @return esp_err_t 
 */
esp_err_t mcp3464_init(mcp3464_conf_t *cfg, mcp3464_context_t** out_ctx){

    ESP_LOGI(TAG, "Initializing MCP3464...");

    esp_err_t err = ESP_OK;

    // Create handle
    mcp3464_context_t* ctx = (mcp3464_context_t*)malloc(sizeof(mcp3464_context_t));
    if(!ctx){
        return ESP_ERR_NO_MEM;
    }

    // Copy configuration into handle
    *ctx = (mcp3464_context_t) {
        .cfg = *cfg,
    };

    // Configure MCP3464 SPI settings
    spi_device_interface_config_t devcfg = {
        .command_bits = 8,
        .clock_speed_hz = ctx->cfg.clock_speed_hz,
        .mode  = 0,
        .spics_io_num = MCP3464_CS,
        .queue_size = 1,
        .input_delay_ns = MCP3464_INPUT_DELAY_NS,
    };

    // Add MCP3464 to SPI bus
    err = spi_bus_add_device(ctx->cfg.host, &devcfg, &ctx->spi);
    if (err != ESP_OK){
        goto cleanup;
    }

    // Write configuration
    mcp3464_write(ctx, MCP3464_CONFIG0_ADDR, 0b10100010); // Set VREF to external, clock to internal, no current source, and ADC standby mode
    mcp3464_write(ctx, MCP3464_CONFIG1_ADDR, 0b11001100); // set conversion period
    mcp3464_write(ctx, MCP3464_CONFIG2_ADDR, 0b10101001); // sets adc gain to 16x
    mcp3464_write(ctx, MCP3464_CONFIG3_ADDR, 0b10000000);
    mcp3464_write(ctx, MCP3464_MUX_ADDR, 0b00001011); // sets VIN+ to channel 0 and VIN- to VREF+
    mcp3464_write(ctx, MCP3464_IRQ_ADDR, 0b00000110);

    // Check configuration
    uint32_t data_check;

    mcp3464_read(ctx, MCP3464_CONFIG0_ADDR, &data_check, 8);
    if(data_check != 0b10100010){
        ESP_LOGE(TAG, "configuration readback error %d", (uint8_t) data_check);
    }else{
        ESP_LOGI(TAG, "configuration readback %d", (uint8_t) data_check);
    }

    mcp3464_read(ctx, MCP3464_CONFIG1_ADDR, &data_check, 8);
    if(data_check != 0b11001100){
        ESP_LOGE(TAG, "configuration readback error %d", (uint8_t) data_check);
    }else{
        ESP_LOGI(TAG, "configuration readback %d", (uint8_t) data_check);
    }

    mcp3464_read(ctx, MCP3464_CONFIG2_ADDR, &data_check, 8);
    if(data_check != 0b10001001){
        ESP_LOGE(TAG, "configuration readback error %d", (uint8_t) data_check);
    }else{
        ESP_LOGI(TAG, "configuration readback %d", (uint8_t) data_check);
    }

    mcp3464_read(ctx, MCP3464_CONFIG3_ADDR, &data_check, 8);
    if(data_check != 0b10000000){
        ESP_LOGE(TAG, "configuration readback error %d", (uint8_t) data_check);
    }else{
        ESP_LOGI(TAG, "configuration readback %d", (uint8_t) data_check);
    }

    mcp3464_read(ctx, MCP3464_MUX_ADDR, &data_check, 8);
    if(data_check != 0b00001011){
        ESP_LOGE(TAG, "configuration readback error %d", (uint8_t) data_check);
    }else{
        ESP_LOGI(TAG, "configuration readback %d", (uint8_t) data_check);
    }


    
    // Create queue to allow interupt to transmit data
    ctx->irq_queue = xQueueCreate(100, sizeof(uint32_t));

    // Initialize IRQ interupt
    ESP_LOGI(TAG, "Initializeing interrupt");
    gpio_reset_pin(MCP3464_IRQ);

    gpio_config_t irq_pin = {
        .intr_type = GPIO_INTR_NEGEDGE,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pin_bit_mask = 0b1 << MCP3464_IRQ, 
    };

    gpio_config(&irq_pin);

    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);
    gpio_isr_handler_add(MCP3464_IRQ, mcp3464_irq, (void*) ctx);

    ESP_LOGI(TAG, "MCP3464 initialization complete");

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


/**
 * @brief send fast start command to MCP3464
 * 
 * @param ctx MCP3464 handle
 * @return esp_err_t 
 */
esp_err_t mcp3464_start_conversion(mcp3464_context_t* ctx){
    
    esp_err_t err;
    err = spi_device_acquire_bus(ctx->spi,portMAX_DELAY);
    if(err != ESP_OK){
        return err;
    }

    spi_transaction_t t = {
        .cmd = (MCP3464_ADDR << 6) | MCP3464_CONVERSION_START,
        .length = 0,
        .user = ctx,
    };    

    err = spi_device_polling_transmit(ctx->spi, &t);

    spi_device_release_bus(ctx->spi);
    return err;
}

/**
 * @brief write 8 bits of data to the MCP3464
 * 
 * @param ctx MCP3464 handle
 * @param addr address to write the data
 * @param data data to write
 * @return esp_err_t 
 */
esp_err_t mcp3464_write(mcp3464_context_t* ctx, uint8_t addr, uint8_t data){

    esp_err_t err;
    err = spi_device_acquire_bus(ctx->spi,portMAX_DELAY);
    if(err != ESP_OK){
        return err;
    }

    spi_transaction_t t = {
        .cmd = (MCP3464_ADDR << 6) | (addr << 2) | MCP3464_INCREMENTAL_WRITE,
        .length = 8,
        .flags = SPI_TRANS_USE_TXDATA,
        .tx_data = {data},
        .user = ctx,
        
    };    

    err = spi_device_polling_transmit(ctx->spi, &t);

    spi_device_release_bus(ctx->spi);
    return err;
}

/**
 * @brief write to the MCP3464 from a buffer
 * 
 * @param ctx MCP3464 handle
 * @param addr address to write to
 * @param data data buffer
 * @param length lenght of buffer in bytes
 * @return esp_err_t 
 */
esp_err_t mcp3464_write_buff(mcp3464_context_t* ctx, uint8_t addr, uint8_t *data, uint8_t length){

    esp_err_t err;
    err = spi_device_acquire_bus(ctx->spi,portMAX_DELAY);
    if(err != ESP_OK){
        return err;
    }


    spi_transaction_t t = {
        .cmd = (MCP3464_ADDR << 6) | (addr << 2) | MCP3464_INCREMENTAL_WRITE,
        .length = 8 * length,
        .tx_buffer = data,
        .user = ctx,
        
    };    

    err = spi_device_polling_transmit(ctx->spi, &t);


    spi_device_release_bus(ctx->spi);
    return err;

}

/**
 * @brief read data from the MCP3464
 * 
 * @param ctx MCP3464 handle
 * @param addr address to read from
 * @param data data read
 * @param bits lenght of read in bits
 * @return esp_err_t 
 */
esp_err_t mcp3464_read(mcp3464_context_t* ctx, uint8_t addr, uint32_t* data, uint8_t bits){

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

    *data = ((uint32_t)t.rx_data[3] << 24) | ((uint32_t)t.rx_data[2] << 16) | ((uint32_t)t.rx_data[1] << 8) | t.rx_data[0];
    return ESP_OK;
}

/**
 * @brief read the raw adc value from the MCP3464
 * 
 * @param ctx MCP3464 handle
 * @param data adc reading
 * @return esp_err_t 
 */
esp_err_t mcp3464_adc_read_raw(mcp3464_context_t* ctx, uint32_t* data){

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

    //*data = (t.rx_data[3] << 24) | (t.rx_data[2] << 16) | (t.rx_data[1] << 8) | t.rx_data[0];
        *data = (t.rx_data[1] << 8) | t.rx_data[0];
    return ESP_OK;    

}
