#include <stdio.h>
#include <esp32s2/rom/gpio.h>



/**
 * @brief Initialize a gpio pin as a simple input or output
 * 
 * @param pin The pin to initialize
 * @param direction 0: input, 1: output
 */
void gpio_init(uint8_t pin, uint8_t direction){

    if(direction){

        // Set GPIO pin to use GPIO_OUT_REG and GPIO_ENABLE_REG for controll
        WRITE_PERI_REG(GPIO_FUNC0_OUT_SEL_CFG_REG + 4*pin, 0x100);  

        // Enable GPIO register
        if(pin < 32){
            WRITE_PERI_REG(GPIO_ENABLE_W1TS_REG, 0b1 << pin);
        }else{
            WRITE_PERI_REG(GPIO_ENABLE1_W1TS_REG, 0b1 << (pin - 32));
        }

    }else{

        // Enable GPIO input for the pin using the IO matrix
        WRITE_PERI_REG(DR_REG_IO_MUX_BASE + 0x4 + (4 * pin), 0b1<<9);

    }
}

/**
 * @brief Return the value of a gpio pin initialized as a simple input
 * 
 * @param pin The pin to read
 * @return uint8_t The value of the pin
 */
uint8_t gpio_read(uint8_t pin){

    // Choose low or high register
    if(pin < 32){
        return (READ_PERI_REG(GPIO_IN_REG) >> pin) & 0b1;
    }else{
        return (READ_PERI_REG(GPIO_IN1_REG) >> (pin - 32)) & 0b1;
    }

}

/**
 * @brief Set the value of a gpio pin initialized as a simple output
 * 
 * @param pin The pin to set
 * @param value The value of the pin
 */
void gpio_write(uint8_t pin, uint8_t value){

    // Choose low or high register
    if(pin < 32){
        if(value){
            // Set the output
            WRITE_PERI_REG(GPIO_OUT_W1TS_REG, 0b1 << pin);
        }else{
            // Clear the output
            WRITE_PERI_REG(GPIO_OUT_W1TC_REG, 0b1 << pin);
        }
    }else{
        if(value){
            // Set the output
            WRITE_PERI_REG(GPIO_OUT1_W1TS_REG, 0b1 << (pin - 32));
        }else{
            // Clear the output
            WRITE_PERI_REG(GPIO_OUT1_W1TC_REG, 0b1 << (pin - 32));
        }
    }

}