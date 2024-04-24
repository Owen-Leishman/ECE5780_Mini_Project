#include <stdint.h>
#include <math.h>
#include <inttypes.h>
#include "esp_err.h"
#include "driver/dac_continuous.h"

#pragma once

#define WAVE_LENGTH 4000
#define STIM_FREQ_HZ 1
#define STIM_FREQ_CALC (WAVE_LENGTH * STIM_FREQ_HZ)

#define SQUARE  1
#define SIN     2

void generate_wave(uint8_t type, uint8_t amplitude, uint8_t* wave, uint32_t wave_len);
esp_err_t stim_dma(dac_continuous_handle_t *stim_handle);

