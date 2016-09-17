#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/cm3/scb.h>

#include "board.h"
#include "usb.h"
#include "ifusb.h"

const struct usb_device_descriptor dev = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = 0xFF,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = IFUSB_PACKET_SIZE,
	.idVendor = IFUSB_VENDORID,
	.idProduct = IFUSB_PRODUCTID,
	.bcdDevice = 0x0200,
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = 1,
};

static const struct usb_endpoint_descriptor data_endp[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x01,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = IFUSB_PACKET_SIZE,
	.bInterval = 0,
}, {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x82,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = IFUSB_PACKET_SIZE,
	.bInterval = 0,
}};

const struct usb_interface_descriptor iface = {
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,
	.bInterfaceClass = 0xFF,
	.bInterfaceSubClass = 0,
	.bInterfaceProtocol = 0,
	.iInterface = 0,

	.endpoint = data_endp,
};

const struct usb_interface ifaces[] = {{
	.num_altsetting = 1,
	.altsetting = &iface,
}};

const struct usb_config_descriptor config = {
	.bLength = USB_DT_CONFIGURATION_SIZE,
	.bDescriptorType = USB_DT_CONFIGURATION,
	.wTotalLength = 0,
	.bNumInterfaces = 1,
	.bConfigurationValue = 1,
	.iConfiguration = 0,
	.bmAttributes = 0x80,
	.bMaxPower = 0x32,

	.interface = ifaces,
};

const char *usb_strings[] = {
	"ifusb",
	"ifusb board",
	"0001",
};

/* Buffer to be used for control requests. */
uint8_t usbd_control_buffer[128];
usbd_device *usbdev = NULL;

int usb_control_request(usbd_device *usbd_dev, 	
	struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
	usbd_control_complete_callback *complete)
{
	(void)complete;
	(void)buf;
	(void)usbd_dev;

	if (req->bmRequestType != 0x40)
		return 0; /* Only accept vendor request. */

	return 1;
}

void usb_data_rx_cb(usbd_device *usbd_dev, uint8_t ep)
{
	(void)ep;
	(void)usbd_dev;

	char buf[IFUSB_PACKET_SIZE];
	int len = usbd_ep_read_packet(usbd_dev, 0x01, buf, IFUSB_PACKET_SIZE);

	ifusb_parse(buf, len);
}


void usb_set_config(usbd_device *usbd_dev, uint16_t wValue)
{
	(void)wValue;
	(void)usbd_dev;

#ifdef USB_USE_INT
//	usbd_ep_setup(usbd_dev, 0x01, USB_ENDPOINT_ATTR_BULK, IFUSB_PACKET_SIZE, NULL);
	usbd_ep_setup(usbd_dev, 0x01, USB_ENDPOINT_ATTR_BULK, IFUSB_PACKET_SIZE, usb_data_rx_cb);
#else
	usbd_ep_setup(usbd_dev, 0x01, USB_ENDPOINT_ATTR_BULK, IFUSB_PACKET_SIZE, usb_data_rx_cb);
#endif
	usbd_ep_setup(usbd_dev, 0x82, USB_ENDPOINT_ATTR_BULK, IFUSB_PACKET_SIZE, NULL);

	usbd_register_control_callback(
				usbd_dev,
				USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
				USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
				usb_control_request);

//	*(USB_EP_REG(0x01)) |= 0x0040;
}


#define EXTI_USB_WAKEUP EXTI18
void usb_setup()
{
	/* USB pins */
	gpio_mode_setup(USB_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE,
	                USB_DM_PIN | USB_DP_PIN);
	gpio_set_af(USB_PORT, GPIO_AF10, USB_DM_PIN | USB_DP_PIN);


	usbdev = usbd_init(&st_usbfs_v2_usb_driver, &dev, &config, usb_strings, 3, usbd_control_buffer, sizeof(usbd_control_buffer));
	usbd_register_set_config_callback(usbdev, usb_set_config);
#ifdef USB_USE_INT
	nvic_enable_irq(NVIC_USB_IRQ);
#endif
}

void usb_isr(void)
{
	usbd_poll(usbdev);
	nvic_clear_pending_irq(NVIC_USB_IRQ);
}

void usb_read() {
	char buf[IFUSB_PACKET_SIZE];
	int len;
	while ((len = usbd_ep_read_packet(usbdev, 0x01, buf, IFUSB_PACKET_SIZE))==0);
  
	ifusb_parse(buf, len);
}

void usb_poll() {
	usbd_poll(usbdev);
}