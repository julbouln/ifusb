#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/gpio.h>

#include "board.h"


void i2c_wait() {
	int wait;
    wait = true;
    while (wait) {
        if (i2c_transmit_int_status(I2C1)) {
            wait = false;
        }
        while (i2c_nack(I2C1));
    }
}

void i2c_send(uint8_t i2c_addr, uint8_t dir_mask, uint8_t *data, uint8_t size) {
    int i;
    while (i2c_busy(I2C1) == 1);
    while (i2c_is_start(I2C1) == 1);
    /*Setting transfer properties*/
    i2c_set_bytes_to_transfer(I2C1, size);
    i2c_set_7bit_address(I2C1, (i2c_addr & dir_mask));
    i2c_set_write_transfer_dir(I2C1);
    i2c_enable_autoend(I2C1);
    /*start transfer*/
    i2c_send_start(I2C1);
    for (i = 0; i < size; i++) {
    	i2c_wait();
        i2c_send_data(I2C1, data[i]);
    }
}

void i2c_recv(uint8_t i2c_addr, uint8_t *data, uint8_t size) {
    int i;

    while (i2c_is_start(I2C1) == 1);
    /*Setting transfer properties*/
    i2c_set_bytes_to_transfer(I2C1, size);
    i2c_set_7bit_address(I2C1, i2c_addr);
    i2c_set_read_transfer_dir(I2C1);
    i2c_enable_autoend(I2C1);
    /*start transfer*/
    i2c_send_start(I2C1);

    for (i = 0; i < size; i++) {
        while (i2c_received_data(I2C1) == 0);
        data[i] = i2c_get_data(I2C1);
    }
}

void i2c_setup(void)
{

	i2c_reset(I2C1);

	/* Set alternate functions for the SCL and SDA pins of I2C1. */
    gpio_mode_setup(GPIOF, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO0|GPIO1);
    gpio_set_af(GPIOF, GPIO_AF1, GPIO0|GPIO1);

	/* Disable the I2C before changing any configuration. */
	i2c_peripheral_disable(I2C1);

	/* I2C clocked by default from HSI (8MHz) */
	/* can be configured to use SYSCLK: RCC_CFGR3 |= (1<<4); */

	/* The timing settings for I2C are all set in I2C_TIMINGR; */
	/* There's a xls for computing it: STSW-STM32126 AN4235 (For F0 and F3) */

	// I2C2_TIMINGR = 0x00301D2B;

	i2c_100khz_i2cclk8mhz(I2C1);

	/* If everything is configured -> enable the peripheral. */
	i2c_peripheral_enable(I2C1);
}