#include <stdint.h>
#include <math.h>
#include <inttypes.h>
#include "esp_err.h"
#include "driver/dac_continuous.h"


#define STIM_P  17
#define STIM_N  18

#define WAVE_LENGTH 400
#define STIM_FREQ_HZ 1000
#define STIM_FREQ_CALC (WAVE_LENGTH * STIM_FREQ_HZ)

//uint8_t sine_wave[WAVE_LENGTH];

//static void generate_wave(){
//    uint32_t point = WAVE_LENGTH;
//
//    for(int i = 0; i < point; i++){
//        
//    }
//}


//esp_err_t stim_dma(){
//
//    esp_err_t ret = ESP_OK;
//
//    dac_continuous_handle_t stim_handle;
//    dac_continuous_config_t stim_conf = {
//        .chan_mask = DAC_CHANNEL_MASK_ALL,
//        .desc_num = 8,
//        .buf_size = 2048,
//        .freq_hz = STIM_FREQ_CALC,
//        .offset = 0,
//        .clk_src = DAC_DIGI_CLK_SRC_DEFAULT,
//        .chan_mode = DAC_CHANNEL_MODE_ALTER,
//    };
//    
//    ret |= dac_continous_new_channels(&stim_conf, &stim_handle);
//    ret |= dac_continous_enable(stim_handle);    
//
//    return ret;
//
//}