#include <ruby.h>
#include "ifusb_host.h"

VALUE Ifusb = Qnil;
VALUE IfusbUart = Qnil;
VALUE IfusbGpio = Qnil;

void Init_ifusb();
VALUE method_ifusb_init(VALUE self);
VALUE method_ifusb_close(VALUE self);

VALUE method_ifusb_uart_set_baud_rate(VALUE self, VALUE bauds);
VALUE method_ifusb_uart_send(VALUE self, VALUE bytes);
VALUE method_ifusb_uart_recv(VALUE self, VALUE len);

VALUE method_ifusb_gpio_config(VALUE self, VALUE pin, VALUE conf);
VALUE method_ifusb_gpio_set(VALUE self, VALUE pin);
VALUE method_ifusb_gpio_clear(VALUE self, VALUE pin);
VALUE method_ifusb_gpio_get(VALUE self, VALUE pin);

void Init_ifusb() {
	Ifusb = rb_define_module("Ifusb");
	rb_define_singleton_method(Ifusb, "init", method_ifusb_init, 0);
	rb_define_singleton_method(Ifusb, "close", method_ifusb_close, 0);

	IfusbUart = rb_define_module_under(Ifusb, "UART");
	rb_define_singleton_method(IfusbUart, "set_baud_rate", method_ifusb_uart_set_baud_rate, 1);
	rb_define_singleton_method(IfusbUart, "send", method_ifusb_uart_send, 1);
	rb_define_singleton_method(IfusbUart, "recv", method_ifusb_uart_recv, 1);

	IfusbGpio = rb_define_module_under(Ifusb, "GPIO");
	rb_define_singleton_method(IfusbGpio, "config", method_ifusb_gpio_config, 2);
	rb_define_singleton_method(IfusbGpio, "set", method_ifusb_gpio_set, 1);
	rb_define_singleton_method(IfusbGpio, "clear", method_ifusb_gpio_clear, 1);
	rb_define_singleton_method(IfusbGpio, "get", method_ifusb_gpio_get, 1);
}

VALUE method_ifusb_init(VALUE self) {
	int ret = ifusb_init();
	return INT2NUM(ret);
}

VALUE method_ifusb_close(VALUE self) {
	ifusb_close();
	return Qnil;
}

// UART
VALUE method_ifusb_uart_set_baud_rate(VALUE self, VALUE bauds) {
	ifusb_uart_set_baud_rate(NUM2INT(bauds));
	return Qnil;
}

VALUE method_ifusb_uart_send(VALUE self, VALUE bytes) {
	int i;
	int n = RARRAY_LEN(bytes);
	uint8_t *buf = (uint8_t *)malloc(sizeof(uint8_t) * n);
	for (i = 0; i < n; i++) {
		buf[i] = NUM2INT(rb_ary_entry(bytes, i));
	}
	ifusb_uart_send(buf, n);
	free(buf);
	return Qnil;
}

VALUE method_ifusb_uart_recv(VALUE self, VALUE len) {
	int i;
	int n = NUM2INT(len);
	VALUE bytes = rb_ary_new2(n);
	uint8_t *buf = (uint8_t *)malloc(sizeof(uint8_t) * n);
	ifusb_uart_recv(buf, n);
	for (i = 0; i < n; i++) {
		rb_ary_store(bytes, i, INT2NUM(buf[i]));
	}
	return bytes;
}

// GPIO
VALUE method_ifusb_gpio_config(VALUE self, VALUE pin, VALUE conf) {
	ifusb_gpio_config(NUM2INT(pin),NUM2INT(conf));
	return Qnil;
}

VALUE method_ifusb_gpio_set(VALUE self, VALUE pin) {
	ifusb_gpio_set(NUM2INT(pin));
	return Qnil;
}

VALUE method_ifusb_gpio_clear(VALUE self, VALUE pin) {
	ifusb_gpio_clear(NUM2INT(pin));
	return Qnil;
}

VALUE method_ifusb_gpio_get(VALUE self, VALUE pin) {
	int ret=ifusb_gpio_get(NUM2INT(pin));
	return INT2NUM(ret);
}

// TODO SPI & I2C