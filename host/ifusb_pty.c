/*
  ifusb userspace TTY emulator driver

  Source:
  https://gist.github.com/posborne/7225894
  http://stackoverflow.com/questions/21641754/when-pty-pseudo-terminal-slave-fd-settings-are-changed-by-tcsetattr-how-ca/23469095#23469095
  https://github.com/kimmoli/tohcom/blob/master/daemon/src/pseudoport.cpp
*/

#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/cdefs.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <termio.h>
#include <sys/stat.h>

#include "ifusb_host.h"

#ifndef EXTPROC
#define EXTPROC 0200000
#endif

#define MAX_BUF_LEN 64

int process_control(int fd, const char c) {
   if (c & TIOCPKT_IOCTL)
    {
        tcflush(fd, TCIOFLUSH);

        struct termios params;
        tcgetattr(fd, &params);

        unsigned int bps = 0;

        switch (params.c_ispeed)
        {
            case B300:    bps = 300;   break;
            case B600:    bps = 600;   break;
            case B1200:   bps = 1200;   break;
            case B2400:   bps = 2400;   break;
            case B4800:   bps = 4800;   break;
            case B9600:   bps = 9600;   break;
            case B19200:  bps = 19200;  break;
            case B38400:  bps = 38400;  break;
            case B57600:  bps = 57600;  break;
            case B115200: bps = 115200; break;
            case B230400: bps = 230400; break;
            case B460800: bps = 460800; break;
            case B921600: bps = 921600; break;
            case B2000000: bps = 2000000; break;
            default: break;
        }

        /* TODO parity and other bits */

        if (bps != 0)
        {
            printf("Set baudrate to %d\n", bps);
            ifusb_uart_set_baud_rate(bps);
        }
        else
            printf("Unsupported baudrate\n");
    }
}

int process_data(int fd, const char *buf, int len) {
/*  int i;
  for(i=0;i<len;i++) {
    printf("[Rx]: %c [%02X]\n", buf[i], buf[i]);
  }
  */
  ifusb_uart_send(buf,len);
}

int process(int fd)
{
  int i;
  char rxbuf[MAX_BUF_LEN];
  char txbuf[MAX_BUF_LEN];
  int bytes_read = 0;
  int bytes_write = 0;

  while (1) {
    bytes_write = ifusb_uart_recv(txbuf,MAX_BUF_LEN);
    if (bytes_write > 0) {
      write(fd,txbuf+1,bytes_write-1);
    }

    bytes_read = read(fd, rxbuf, MAX_BUF_LEN);
    if (bytes_read > 0) {
      if (rxbuf[0] == TIOCPKT_DATA) // data
      {
        process_data(fd,rxbuf+1,bytes_read-1);
      } else {
        process_control(fd,rxbuf[0]);
      }
    }


  }
  return 0;
}


int main(int argc, char **argv)
{
  char *slavepath;

  printf("ifusb TTY emulator\n");

  if (!ifusb_init())
    goto out;

  int fd = posix_openpt(O_RDWR| O_NONBLOCK);
  (void)grantpt(fd);
  (void)unlockpt(fd);

  int flag = 1;
  ioctl(fd, TIOCPKT, &flag);

  struct termios params;

  tcgetattr(fd, &params);

  /* Set EXTPROC to get IOCTL */
  params.c_lflag |= EXTPROC;
  tcsetattr(fd, TCSANOW, &params);

  tcflush(fd, TCIOFLUSH);

  slavepath = ptsname(fd);
  printf("TTY available as %s\n", slavepath);
  
  process(fd);

  out:
    ifusb_close();
}