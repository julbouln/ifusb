/*
	SPI Flash chip ifusb SPI driver
*/
#include <stdio.h>
#include <stdint.h>

#include "ifusb_host.h"

#define SPIFLASH_READID 0x9F

#define SPIFLASH_BULK_ERASE  0xC7

#define SPIFLASH_STATUSWRITE 0x01
#define SPIFLASH_WRITE  0x02
#define SPIFLASH_READ   0x03
#define SPIFLASH_WRITE_DISABLE  0x04
#define SPIFLASH_READREG  0x05
#define SPIFLASH_WRITE_ENABLE 0x06

#define SPIFLASH_POWER_UP 0xAB
#define SPIFLASH_POWER_DOWN 0xB9

#define BLOCK_SIZE 256

static uint8_t spiflash_cs_pin;

void spiflash_init(uint8_t cs_pin) {
	spiflash_cs_pin = cs_pin;

	ifusb_gpio_config(spiflash_cs_pin, IFUSB_OUTPUT_MODE);
	ifusb_gpio_set(spiflash_cs_pin);
}

void spiflash_readid(uint8_t *manufacturer_id, uint8_t *device_id1, uint8_t *device_id2) {
	uint8_t buf[4];
	uint8_t recv[4];
	buf[0] = SPIFLASH_READID;
	buf[1] = 0x00;
	buf[2] = 0x00;
	buf[3] = 0x00;

	ifusb_gpio_clear(spiflash_cs_pin);
	ifusb_spi_xfer(buf, recv, 4);
	ifusb_gpio_set(spiflash_cs_pin);

	*manufacturer_id = recv[1];
	*device_id1 = recv[2];
	*device_id2 = recv[3];
}

void spiflash_write_enable() {
	uint8_t buf[1];
	buf[0] = SPIFLASH_WRITE_ENABLE;
	ifusb_gpio_clear(spiflash_cs_pin);
	ifusb_spi_send(buf, 1);
	ifusb_gpio_set(spiflash_cs_pin);
}

void spiflash_write_disable() {
	uint8_t buf[1];
	buf[0] = SPIFLASH_WRITE_DISABLE;
	ifusb_gpio_clear(spiflash_cs_pin);
	ifusb_spi_send(buf, 1);
	ifusb_gpio_set(spiflash_cs_pin);
}

void spiflash_status_write(uint8_t status) {

	uint8_t buf[2];
	buf[0] = SPIFLASH_STATUSWRITE;
	buf[1] = status;
	ifusb_gpio_clear(spiflash_cs_pin);
	ifusb_spi_send(buf, 2);
	ifusb_gpio_set(spiflash_cs_pin);

}

uint8_t spiflash_read_register() {
	uint8_t buf[2];
	uint8_t recv[2];

	buf[0] = SPIFLASH_READREG;
	buf[1] = 0x00;

	ifusb_gpio_clear(spiflash_cs_pin);
	ifusb_spi_xfer(buf, recv, 2);
	ifusb_gpio_set(spiflash_cs_pin);
	return recv[1];
}

void spiflash_power_down() {
	uint8_t buf[1];
	buf[0] = SPIFLASH_POWER_DOWN;
	ifusb_gpio_clear(spiflash_cs_pin);
	ifusb_spi_send(buf, 1);
	ifusb_gpio_set(spiflash_cs_pin);
}

void spiflash_power_up() {
	uint8_t buf[1];
	buf[0] = SPIFLASH_POWER_UP;
	ifusb_gpio_clear(spiflash_cs_pin);
	ifusb_spi_send(buf, 1);
	ifusb_gpio_set(spiflash_cs_pin);
}

void spiflash_erase() {
	uint8_t buf[1];
	buf[0] = SPIFLASH_BULK_ERASE;
	ifusb_gpio_clear(spiflash_cs_pin);
	ifusb_spi_send(buf, 1);
	ifusb_gpio_set(spiflash_cs_pin);
//	printf("spiflash erasing...\n");
}

void spiflash_write(int addr, uint8_t *data, int size) {
	int j;
	uint8_t buf[size + 4];

//	printf("spiflash write %x\n", addr);
	buf[0] = SPIFLASH_WRITE;
	buf[1] = (uint8_t)(addr >> 16);
	buf[2] = (uint8_t)(addr >> 8);
	buf[3] = (uint8_t)addr;
	for (j = 0; j < size; j++) {
		buf[j + 4] = data[j];
	}
	ifusb_gpio_clear(spiflash_cs_pin);
	ifusb_spi_send(buf, size + 4);
	ifusb_gpio_set(spiflash_cs_pin);
}

void spiflash_read(int addr, uint8_t *data, int size) {
	int j;
	uint8_t buf[size + 4];
	uint8_t recv[size + 4];

//	printf("spiflash read %x\n", addr);

	buf[0] = SPIFLASH_READ;
	buf[1] = (uint8_t)(addr >> 16);
	buf[2] = (uint8_t)(addr >> 8);
	buf[3] = (uint8_t)addr;

	ifusb_gpio_clear(spiflash_cs_pin);
	ifusb_spi_xfer(buf, recv, size + 4);
	ifusb_gpio_set(spiflash_cs_pin);

	for (j = 0; j < size; j++) {
		data[j] = recv[j + 4];
	}

}

void spiflash_wait(int tm) {
	while (spiflash_read_register() & 0x01) {
		usleep(tm);
	}
}