##
## This file is part of the libopencm3 project.
##
## Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
##
## This library is free software: you can redistribute it and/or modify
## it under the terms of the GNU Lesser General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This library is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public License
## along with this library.  If not, see <http://www.gnu.org/licenses/>.
##

DEVICE=stm32f042
BINARY = main
OPENCM3_DIR=/home/julbouln/Perso/stm/libopencm3
OBJS += usart.o spi.o i2c.o usb.o ifusb.o 
CFLAGS += -I../common/
CPPFLAGS += -I$(CURDIR)/include
LDSCRIPT = ./stm32.ld
JLINK_EXE = /usr/local/bin/JLinkExe
include ./libopencm3.target.mk

prog:
	sudo dfu-util -a 0 -d 0483:df11 -s 0x08000000:leave -D main.bin
	