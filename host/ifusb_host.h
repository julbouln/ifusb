#ifndef IFUSB_HOST_H
#define IFUSB_HOST_H
#include "ifusb.h"

#ifdef __cplusplus
extern "C" {
#endif

int ifusb_init();
void ifusb_close();

void ifusb_packet_write(uint8_t *buf, uint8_t len);
int ifusb_packet_read(uint8_t *buf, uint8_t len);
void ifusb_write(uint8_t id, uint8_t *data, int len);
void ifusb_xfer(uint8_t id, uint8_t *data_in, uint8_t *data_out, int len);

// GPIO
void ifusb_gpio_config(uint8_t pin, uint8_t conf);
void ifusb_gpio_set(uint8_t pin);
void ifusb_gpio_clear(uint8_t pin);
uint8_t ifusb_gpio_get(uint8_t pin);

// UART
void ifusb_uart_send(uint8_t *data, int len);
int ifusb_uart_recv(uint8_t *data, int len);
void ifusb_uart_set_baud_rate(int rate);

// SPI
void ifusb_spi_send(uint8_t *data, int len);
void ifusb_spi_xfer(uint8_t *data_in, uint8_t *data_out, int len);
void ifusb_spi_set_freq(uint8_t freq);

// I2C
void ifusb_i2c_send(uint8_t addr, uint8_t *data, int len);
void ifusb_i2c_xfer(uint8_t addr, uint8_t *data_in, int data_in_len, uint8_t *data_out, int len);


#ifdef __cplusplus
}
#endif

#endif