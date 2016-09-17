OBJS=layer1.o
CFLAGS=-I../common/ -g

%.o : %.c
	gcc $(CFLAGS) -o $@ -c $<

%.o : %.cpp
	gcc $(READER_CFLAGS) $(CFLAGS) -o $@ -c $<

all: ifusb_ping ifusb_pty ifusb_iceprog

libifusb.a: ifusb_host.o
	ar cr libifusb.a ifusb_host.o

ifusb_ping: ifusb_host.o spi_flash.o ifusb_ping.o
	gcc -o ifusb_ping ifusb_host.o spi_flash.o ifusb_ping.o -lm -lusb-1.0

ifusb_pty: ifusb_host.o ifusb_pty.o
	gcc -o ifusb_pty ifusb_host.o ifusb_pty.o -lm -lusb-1.0

ifusb_iceprog: ifusb_host.o spi_flash.o ifusb_iceprog.o
	gcc -o ifusb_iceprog ifusb_host.o spi_flash.o ifusb_iceprog.o -lm -lusb-1.0

clean:
	rm -f *.o
	rm -f *.a
	rm -f ifusb_ping
	rm -f ifusb_pty
	rm -f ifusb_iceprog
