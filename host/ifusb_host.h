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

#ifdef __cplusplus
}
#endif

#endif