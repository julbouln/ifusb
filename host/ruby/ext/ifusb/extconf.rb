require 'mkmf'
$CFLAGS += " -I/home/julbouln/Perso/ifusb/common -I/home/julbouln/Perso/ifusb/host"
$LDFLAGS += " /home/julbouln/Perso/ifusb/host/libifusb.a -lm -lusb-1.0"
extension_name = 'ifusb'
dir_config(extension_name)
create_makefile(extension_name)