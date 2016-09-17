#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/cm3/nvic.h>

#include "usb.h"
#include "board.h"

#include "ifusb.h"

extern usbd_device *usbdev;

void ifusb_init() {
}

static void ifusb_usart_write(char *buf, uint8_t len) {
#ifdef USART_USE_DMA
	usart_dma_write(buf + 1, len - 1); // excluding command byte
#else
	usart_write(buf + 1, len - 1);
#endif
}

void ifusb_usart_set_baud_rate(uint32_t baud_rate) {
	usart_disable(USARTN);
	usart_set_baudrate(USARTN, baud_rate);
	usart_enable(USARTN);
}

void ifusb_spi_xfer(uint8_t *ibuf, uint8_t *obuf, uint8_t len)
{
#ifdef SPI_USE_DMA
	spi_dma_xfer(ibuf + 1, obuf + 1, len - 1);
	obuf[0] = IFUSB_SPI_RECV;
#else
	int b;
	obuf[0] = IFUSB_SPI_RECV;
	for (b = 1; b < len; b++) {
		spi_send8(SPI1, ibuf[b]);
		obuf[b] = spi_read8(SPI1);
	}
#endif
}

void ifusb_spi_send(uint8_t *buf, uint8_t len) {
#ifdef SPI_USE_DMA
	char obuf[len];
	spi_dma_xfer(buf + 1, obuf + 1, len - 1);
#else
	int b;
	for (b = 1; b < len; b++) {
		spi_send8(SPI1, buf[b]);
		spi_read8(SPI1);
	}
#endif
}

void ifusb_i2c_send(uint8_t *buf, uint8_t len) {
	i2c_send(buf[1],0x7F,buf+2,len-2);
}

// ifusb_cmd,i2c_addr,i2c_write_len,data
void ifusb_i2c_xfer(uint8_t *ibuf, uint8_t *obuf, uint8_t len) {
	i2c_send(ibuf[1],0xFF,ibuf+3,ibuf[2]);
	obuf[0]=IFUSB_I2C_RECV;
	i2c_recv(ibuf[1],obuf+1,len-1);
}

// SPI_CR1_BR_FPCLK_DIV_2 = 24mhz
// SPI_CR1_BR_FPCLK_DIV_4 = 12mhz
// SPI_CR1_BR_FPCLK_DIV_8 = 6mhz
// SPI_CR1_BR_FPCLK_DIV_16 = 3mhz
// SPI_CR1_BR_FPCLK_DIV_32 = 1.5mhz
// SPI_CR1_BR_FPCLK_DIV_64 = 750khz
// SPI_CR1_BR_FPCLK_DIV_128 = 325khz
// SPI_CR1_BR_FPCLK_DIV_256 = 187.5khz

void ifusb_spi_set_freq(uint8_t freq)
{
	spi_disable(SPI1);
	spi_set_baudrate_prescaler(SPI1, freq);
	spi_enable(SPI1);
}

void ifusb_gpio_config(uint8_t pin, uint8_t conf) {
	if (conf & 0x01)
		gpio_mode_setup(gpio_map[pin][0], GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, gpio_map[pin][1]);
	else
		gpio_mode_setup(gpio_map[pin][0], GPIO_MODE_INPUT, GPIO_PUPD_NONE, gpio_map[pin][1]);
}

void ifusb_gpio_set(uint8_t pin, uint8_t value) {
	if (value & 0x01)
		gpio_set(gpio_map[pin][0], gpio_map[pin][1]);
	else
		gpio_clear(gpio_map[pin][0], gpio_map[pin][1]);
}

uint8_t ifusb_gpio_get(uint8_t pin) {
	if(gpio_get(gpio_map[pin][0], gpio_map[pin][1]))
		return 1;
	else
		return 0;
}

void ifusb_parse(char *buf, uint8_t len)
{
	uint8_t recv[len];
	switch (buf[0]) {
	case IFUSB_PING:
		ifusb_packet_write(buf, len);
		break;
	case IFUSB_GPIO_SEND:
		ifusb_gpio_set(buf[1], buf[2]);
		break;
	case IFUSB_GPIO_RECV:
		recv[0] = buf[0];
		recv[1] = ifusb_gpio_get(buf[1]);
		ifusb_packet_write(recv, 2);
		break;
	case IFUSB_GPIO_CONFIG:
		ifusb_gpio_config(buf[1], buf[2]);
		break;
	case IFUSB_UART_SEND:
		ifusb_usart_write(buf, len);
		break;
	case IFUSB_UART_SET_BAUD_RATE:
		// config baud rate
		ifusb_usart_set_baud_rate(((uint32_t)(buf[1]) << 16) | ((uint32_t)(buf[2]) << 8) | (uint32_t)(buf[3]));
		break;
	case IFUSB_SPI_SEND:
//		ifusb_spi_xfer(buf, recv, len);
		ifusb_spi_send(buf, len);
		break;
	case IFUSB_SPI_XFER:
		ifusb_spi_xfer(buf, recv, len);
		ifusb_packet_write(recv, len);
		break;
	case IFUSB_SPI_SET_FREQ:
		ifusb_spi_set_freq(buf[1]);
		break;
	case IFUSB_I2C_SEND:
		ifusb_i2c_send(buf, len);
		break;
	case IFUSB_I2C_XFER:
		ifusb_i2c_xfer(buf, recv, len);
		ifusb_packet_write(recv, len);		
		break;
	default:
		break;
	}
}

void ifusb_packet_write(uint8_t *buf, uint8_t len) {
	if (usbdev) // check usb is ready
		while (usbd_ep_write_packet(usbdev, 0x82, buf, len) == 0);
}

