#include <stdint.h>
#include <math.h>
#include <inttypes.h>
#include "esp_err.h"
#include "driver/dac_continuous.h"
#include "stim.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#define STIM_P  17
#define STIM_N  18





void generate_wave(uint8_t type, uint8_t amplitude, uint8_t* wave, uint32_t wave_len){

    for(int i = 0; i < (wave_len/2); i += 2){
        switch(type){
            case SIN:
                wave[i]     = (uint8_t)(sin(i * 6.2832 / wave_len) * amplitude) ;
                wave[i + 1] = 0;                
                break;
            case SQUARE:
                wave[i]     = amplitude;
                wave[i + 1] = 0;                
                break;
            default:
                wave[i]     = 0;
                wave[i + 1] = 0;
                break;
        };
    }

    vTaskDelay(1);

    for(int i = (wave_len/2); i < wave_len; i += 2){
        switch(type){
            case SIN:
                wave[i]     = 0;
                wave[i + 1] = (uint8_t)(-1 * sin(i * 6.2832 / wave_len) * amplitude);  
                break;
            case SQUARE:
                wave[i]     = 0;
                wave[i + 1] = amplitude;                
                break;
            default:
                wave[i]     = 0;
                wave[i + 1] = 0;
                break;
        };
    }


}


esp_err_t stim_dma(dac_continuous_handle_t *stim_handle){

    esp_err_t ret = ESP_OK;

    
    dac_continuous_config_t stim_conf = {
        .chan_mask = DAC_CHANNEL_MASK_ALL,
        .desc_num = 8,
        .buf_size = 2048,
        .freq_hz = STIM_FREQ_CALC,
        .offset = 0,
        .clk_src = DAC_DIGI_CLK_SRC_DEFAULT,
        .chan_mode = DAC_CHANNEL_MODE_ALTER,
    };
        
    ret |= dac_continuous_new_channels(&stim_conf, stim_handle);
    ret |= dac_continuous_enable(*stim_handle);    

    return ret;

}