#pragma once

#include <stdio.h>
#include <esp32s2/rom/gpio.h>


void gpio_init(uint8_t pin, uint8_t direction);
uint8_t gpio_read(uint8_t pin);
void gpio_write(uint8_t pin, uint8_t value);