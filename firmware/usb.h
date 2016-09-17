#ifndef USB_H
#define USB_H
#include <stdlib.h>

#include <libopencm3/stm32/st_usbfs.h>

#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>

int usb_control_request(usbd_device *usbd_dev, 	struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
	usbd_control_complete_callback *complete);
void usb_data_rx_cb(usbd_device *usbd_dev, uint8_t ep);
void usb_set_config(usbd_device *usbd_dev, uint16_t wValue);

void usb_setup(void);
void usb_read(void);
void usb_poll(void);
#endif