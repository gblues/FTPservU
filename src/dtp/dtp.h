#ifndef _DTP_H
#define _DTP_H

#include "ftpservu_types.h"
#include "wiiu/types.h"

#define DTP_PENDING       0 // result of either a PORT or PASV command
#define DTP_ESTABLISHED   1 // TCP/IP connection has been established
#define DTP_SENDING       2 // is sending
#define DTP_RECVING       3 // is receiving
/* states DTP_CLOSED or higher mean the connection is dead.
   new states go above here, and then renumber so that these
   are always highest. */
#define DTP_CLOSED        4 // file transfer is complete
#define DTP_ERROR         5 // a TCP/IP error occurred on the connection
#define DTP_FREE          6 // references are cleaned up; ready to free
#define DTP_RECV       0x20 // channel is receiving from remote
#define DTP_XMIT       0x40 // channel is sending data to remote
#define DTP_LOCAL_BUF  0x80 // local is a buffer, not a file

#define GET_STATE(dtp) (dtp->state & 0x0F)
#define SET_STATE(dtp, flag) ((dtp)->state = ((dtp)->state & DTP_LOCAL_BUF) | flag)

struct data_channel {
  data_channel_t *next;
  u8 state;
  union {
    int fd;
    char *buffer;
  } local;
  int remote_fd;
  int listen_fd;
  u32 ip;
  u16 port;

  client_t *client;
  io_buffer_t *buffer;
  data_interface_t *iface;
};

struct data_interface {
  data_channel_t *(*new)(u32 ip, u16 port);
  void (*free)(data_channel_t *);
  void (*accept)(data_channel_t *);
  bool (*is_connected)(data_channel_t *);
  void (*send)(data_channel_t *);
  void (*recv)(data_channel_t *);
};

void dtp_poll(void);
int dtp_send_buffer(data_channel_t *channel, char *buffer);
void dtp_channel_init(client_t *client, u32 ip, u16 port);

extern data_interface_t passive;
extern data_interface_t active;
extern data_interface_t base;

#endif /* _DTP_H */
