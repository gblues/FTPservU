#ifndef _DTP_H
#define _DTP_H

#include "ftpservu_types.h"
#include "wiiu/types.h"

#define DTP_PENDING     0 // result of either a PORT or PASV command
#define DTP_ESTABLISHED 1 // TCP/IP connection has been established
#define DTP_XFER        2 // file is being transferred
#define DTP_CLOSED      3 // file transfer is complete
#define DTP_ERROR       4 // a TCP/IP error occurred on the connection

struct data_channel {
  data_channel_t *next;
  u8 state;
  int local_fd;
  int remote_fd;
  u32 listen_fd;
  u32 ip;
  u16 port;
  
  io_buffer_t *buffer;
};

struct data_interface {
  data_channel_t *(*new)(u32 ip, u16 port);
  void (*free)(data_channel_t *);
};

extern data_interface_t passive;
extern data_interface_t active;

#endif /* _DTP_H */
