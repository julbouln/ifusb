/*
	SPI Flash chip ifusb SPI driver
*/
#ifndef SPI_FLASH_H
#define SPI_FLASH_H
void spiflash_init();
void spiflash_readid(uint8_t *manufacturer_id, uint8_t *device_id1, uint8_t *device_id2);
void spiflash_write_enable();
void spiflash_write_disable();
uint8_t spiflash_read_register();
void spiflash_erase();
void spiflash_page_write(int page, uint8_t *data);
void spiflash_page_read(int page, uint8_t *data);
#endif