/*
  ifusb host library
*/
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include <libusb-1.0/libusb.h>

#include "ifusb_host.h"

//#define DEBUG 1

#define IFUSB_ASYNC_READ 1
#define IFUSB_ASYNC_WRITE 1

static struct libusb_device_handle *devh = NULL;

static int ep_in_addr  = 0x82;
static int ep_out_addr = 0x01;


#define IN_BUFFER_LEN 1024*8
static uint8_t in_buffer[IN_BUFFER_LEN];
int recv_bytes = 0;
int received = 0;

int trfr_bytes = 0;
int transfered = 0;


// OUT-going transfers (OUT from host PC to USB-device)
struct libusb_transfer *transfer_out = NULL;

// IN-coming transfers (IN to host PC from USB-device)
struct libusb_transfer *transfer_in = NULL;


static libusb_context *ctx = NULL;

// Low-level libusb driver

int ifusb_test() {
  uint8_t buf[1];
  uint8_t recv[1];
  buf[0] = 0xAA;
  printf("ifusb testing ... ");
  ifusb_xfer(IFUSB_PING, buf, recv, 1);
  if (recv[0] == 0xAA) {
    printf("OK\n");
    return 1;
}
  else
    return 0;
}

int ifusb_init() {
  int rc;

  /* Initialize libusb
   */
  rc = libusb_init(NULL);
  if (rc < 0) {
    fprintf(stderr, "ifusb error initializing libusb: %s\n", libusb_error_name(rc));
    return 0;
  }

  /* Set debugging output to max level.
   */
  libusb_set_debug(NULL, LIBUSB_LOG_LEVEL_WARNING);

  /* Look for a specific device and open it.
   */
  devh = libusb_open_device_with_vid_pid(ctx, IFUSB_VENDORID, IFUSB_PRODUCTID);
  if (!devh) {
    fprintf(stderr, "ifusb error finding USB device %x:%x\n", IFUSB_VENDORID, IFUSB_PRODUCTID);
    return 0;
  }

  /*
  * Class defines two interfaces: the Control interface and the
  * Data interface.
  */
  int if_num;
  for (if_num = 0; if_num < 1; if_num++) {
    rc = libusb_claim_interface(devh, if_num);
    if (rc < 0) {
      fprintf(stderr, "ifusb error claiming interface %d: %s\n", if_num,
              libusb_error_name(rc));
      return 0;
    }
  }

  sleep(1);
  if (!ifusb_test()) {
    fprintf(stderr, "ifusb error testing protocol\n");
    return 0;
  }


  return 1;
}

void ifusb_close() {
  if (devh) {
    libusb_release_interface(devh, 0);
    libusb_close(devh);
  }
  libusb_exit(NULL);
}

void ifusb_packet_write(uint8_t *buf, uint8_t len)
{
  int r;
  int actual_length;
  if ((r=libusb_bulk_transfer(devh, ep_out_addr, buf, len,
                           &actual_length, 10)) < 0) {
    fprintf(stderr, "ifusb_packet_write blocking transfer error: %d\n",r);
  }
}

int ifusb_packet_read(uint8_t *buf, uint8_t len)
{
  int actual_length;
  int rc = libusb_bulk_transfer(devh, ep_in_addr, buf, len, &actual_length,
                                10);
  if (rc == LIBUSB_ERROR_TIMEOUT) {
//    printf("ifusb timeout (%d)\n", actual_length);
    return -1;
  } else if (rc < 0) {
    fprintf(stderr, "ifusb_packet_read blocking transfer error: %d\n",rc);
    return -1;
  }

  return actual_length;
}

// Multipacket functions

void _ifusb_cb_in(struct libusb_transfer *transfer)
{
  if (transfer->status == LIBUSB_TRANSFER_COMPLETED) {
    int r;
    recv_bytes += transfer->actual_length;
    //submit the next transfer
    if (recv_bytes < transfer->length) {
      printf("ifusb async recv %d, %d remaining\n", recv_bytes, transfer->length-recv_bytes);

      r = libusb_submit_transfer(transfer_in);
    }
    else {
//      printf("ifusb finished async recv %d, %d remaining\n", recv_bytes, transfer->length-recv_bytes);
//      ifusb_dbg_buf(in_buffer,transfer->length);
      received = 1;
    }
  } else {
    printf("ifusb something wrong happen : %d\n",transfer->status);
  }
}

void _ifusb_cb_out(struct libusb_transfer *transfer)
{

  if (transfer->status == LIBUSB_TRANSFER_COMPLETED) {
    int r;
    trfr_bytes += transfer->actual_length;
    if (trfr_bytes < transfer->length) {
      r = libusb_submit_transfer(transfer_out);
    } else {
      transfered = 1;
    }
  }

}

void ifusb_dbg_buf(uint8_t *buf, int len) {
  int i = 0;
  for (i = 0; i < len; i++) {
    printf("%03d:%02x ", i, buf[i]);
  }
  printf("\n");
}

void ifusb_async_read(int len) {
  int i;
  int rc;
  recv_bytes = 0;
  received = 0;

  for(i=0;i<len;i++) {
    in_buffer[i]=0xFF;
  }

  transfer_in  = libusb_alloc_transfer(0);

  libusb_fill_bulk_transfer(transfer_in,
                            devh,
                            ep_in_addr,
                            in_buffer,
                            len,
                            _ifusb_cb_in,
                            NULL,
                            1000);

  rc = libusb_submit_transfer(transfer_in);
  if (rc < 0)
  {
    fprintf(stderr, "ifusb_async_read non-blocking transfer error: %d\n",rc);
    libusb_free_transfer(transfer_in);
  }

}

void ifusb_async_write(uint8_t *data, int len) {
  int rc;
  trfr_bytes = 0;
  transfered = 0;

  transfer_out = libusb_alloc_transfer(0);

  libusb_fill_bulk_transfer(transfer_out,
                            devh,
                            ep_out_addr,
                            data,
                            len,
                            _ifusb_cb_out,
                            NULL,
                            1000
                           );

  rc = libusb_submit_transfer(transfer_out);
  if (rc < 0)
  {
    fprintf(stderr, "ifusb_async_write non-blocking transfer error transfered:%d, cmd:%x, len:%d: %d\n",
      transfered,data[0],len,rc);
    libusb_free_transfer(transfer_out);
  }
}

static int _ifusb_needed_len(int len) {
//  len + (len >> 6) + 1;
//  return len + (len / (IFUSB_PACKET_SIZE - 1)) + 1;
  return (int)ceil(((float)len/((float)(IFUSB_PACKET_SIZE - 1))) * (float)IFUSB_PACKET_SIZE);
}


void ifusb_write(uint8_t id, uint8_t *data, int len) {
  int total_len = _ifusb_needed_len(len);
//  printf("WRITE %d\n",total_len);
  uint8_t tmpbuf[total_len];
  int cur = 0;
  int sent_packets = 0;
  int sent_bytes;
  for (sent_bytes = 0; sent_bytes < total_len; sent_bytes += (IFUSB_PACKET_SIZE - 1)) {
    int i;
    uint8_t t_data[IFUSB_PACKET_SIZE];
    uint8_t send = IFUSB_PACKET_SIZE;
    int remaining = total_len - sent_bytes - sent_packets;
    if (remaining < IFUSB_PACKET_SIZE)
      send = remaining;

    if (send == 0)
      break;

    // add command id
    t_data[0] = id;
    tmpbuf[cur] = id;
    cur++;
    for (i = 1; i < send; i++) {
//      printf("%d %d\n",cur,sent_bytes + i - 1);
      if((sent_bytes + i - 1) < len) {
      t_data[i] = data[sent_bytes + i - 1];
      tmpbuf[cur] = data[sent_bytes + i - 1];
    }
      cur++;
  }

#ifdef DEBUG
    printf("ifusb send %d, %d remaining\n", sent_bytes, send);
#endif
#ifdef IFUSB_ASYNC_WRITE
#else
    ifusb_packet_write(t_data, send);
#endif
    sent_packets++;
  }

#ifdef IFUSB_ASYNC_WRITE
  ifusb_async_write(tmpbuf, total_len);

  // wait send completion
  while (transfered == 0) {
//      printf("wait for transfer end\n");
    libusb_handle_events_completed(ctx, NULL);
  }
  //    printf("transferred\n");
#endif

}


void ifusb_xfer(uint8_t id, uint8_t *data_in, uint8_t *data_out, int len) {
  int total_len = _ifusb_needed_len(len);
  uint8_t tmpbuf[total_len];
  int cur = 0;
  int sent_packets = 0;
  int sent_bytes;

#ifdef IFUSB_ASYNC_READ
  ifusb_async_read(total_len);
#endif

  for (sent_bytes = 0; sent_bytes < total_len; sent_bytes += (IFUSB_PACKET_SIZE - 1)) {
    int i;
    uint8_t t_data_in[IFUSB_PACKET_SIZE];
    uint8_t t_data_out[IFUSB_PACKET_SIZE];
    uint8_t send = IFUSB_PACKET_SIZE;
    int remaining = total_len - sent_bytes - sent_packets;
    if (remaining < IFUSB_PACKET_SIZE)
      send = remaining;

    if (send == 0)
      break;

    // add command id
    t_data_in[0] = id;
    tmpbuf[cur] = id;
    cur++;
    for (i = 1; i < send; i++) {
      t_data_in[i] = data_in[sent_bytes + i - 1];
      tmpbuf[cur] = data_in[sent_bytes + i - 1];
      cur++;
    }

#ifdef DEBUG
    printf("ifusb xfer %d, %d remaining\n", sent_bytes, send);
#endif

#ifdef IFUSB_ASYNC_WRITE
#else
    ifusb_packet_write(t_data_in, send);
#endif
#ifdef IFUSB_ASYNC_READ
#else
    // merge received packets
    ifusb_packet_read(t_data_out, send);
    for (i = 1; i < send; i++) {
      data_out[sent_bytes + i - 1] = t_data_out[i];
    }
#endif
    sent_packets++;
  }

#ifdef IFUSB_ASYNC_WRITE
  ifusb_async_write(tmpbuf, total_len);

  // wait send completion
  while (transfered == 0) {
//      printf("wait for transfer end\n");
    libusb_handle_events_completed(ctx, NULL);
  }
#endif

#ifdef IFUSB_ASYNC_READ
  // wait receive completion
  while (received == 0) {
//    printf("wait for transfer end\n");
    libusb_handle_events_completed(ctx, NULL);
  }
  cur = 0;
  int i, j;
  // merge received packets
  for (i = 0; i < sent_packets; i++) {
    for (j = 1; j < IFUSB_PACKET_SIZE; j++) {
      if (cur < len) {
        data_out[cur] = in_buffer[cur + i + 1];
        cur++;
      }
    }
  }
  libusb_cancel_transfer(transfer_in); // TODO check if this is needed
#endif

}

// High-level helpers

// GPIO

void ifusb_gpio_config(uint8_t pin, uint8_t conf) {
  uint8_t buf[2];
  buf[0] = pin;
  buf[1] = conf;
  ifusb_write(IFUSB_GPIO_CONFIG, buf, 2);
}

void ifusb_gpio_set(uint8_t pin) {
  uint8_t buf[2];
  buf[0] = pin;
  buf[1] = IFUSB_OUTPUT_SET;
  ifusb_write(IFUSB_GPIO_SEND, buf, 2);
}

void ifusb_gpio_clear(uint8_t pin) {
  uint8_t buf[2];
  buf[0] = pin;
  buf[1] = IFUSB_OUTPUT_CLR;
  ifusb_write(IFUSB_GPIO_SEND, buf, 2);
}

uint8_t ifusb_gpio_get(uint8_t pin) {
  uint8_t buf[1];
  uint8_t recv[1];
  buf[0] = pin;
  ifusb_xfer(IFUSB_GPIO_RECV, buf, recv, 1);
  return recv[0];
}

// UART

void ifusb_uart_send(uint8_t *data, int len) {
  ifusb_write(IFUSB_UART_SEND, data, len);
}


int ifusb_uart_recv(uint8_t *data, int len) {
  return ifusb_packet_read(data, len);
}

void ifusb_uart_set_baud_rate(int rate) {
  uint8_t buf[3];
  buf[2] = (rate & 0x000000FF);
  buf[1] = (rate & 0x0000FF00) >> 8;
  buf[0] = (rate & 0x00FF0000) >> 16;

  ifusb_write(IFUSB_UART_SET_BAUD_RATE, buf, 3);
}

// SPI

void ifusb_spi_send(uint8_t *data, int len) {
  ifusb_write(IFUSB_SPI_SEND, data, len);
}

void ifusb_spi_xfer(uint8_t *data_in, uint8_t *data_out, int len) {
  ifusb_xfer(IFUSB_SPI_XFER, data_in, data_out, len);
}

void ifusb_spi_set_freq(uint8_t freq) {
  uint8_t buf[1];
  buf[0] = freq;
  ifusb_write(IFUSB_SPI_SET_FREQ, buf, 1);
}

// I2C

void ifusb_i2c_send(uint8_t addr, uint8_t *data, int len) {
  int i=0;
  uint8_t buf[len+1];
  buf[0]=addr;
  for(i=1;i<len+1;i++) {
    buf[i]=data[i-1];
  }
  ifusb_write(IFUSB_I2C_SEND, buf, len+1);
}

void ifusb_i2c_xfer(uint8_t addr, uint8_t *data_in, int data_in_len, uint8_t *data_out, int len) {
  int i=0;
  uint8_t buf_in[len+2];
  buf_in[0]=addr;
  buf_in[1]=data_in_len;
  for(i=2;i<len+2;i++) {
    buf_in[i]=data_in[i-2];
  }
  ifusb_xfer(IFUSB_I2C_XFER, buf_in, data_out, len);
}
