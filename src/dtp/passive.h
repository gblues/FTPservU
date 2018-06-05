#ifndef _PASSIVE_H
#define _PASSIVE_H

#include "wiiu/types.h"
#include "iobuffer.h"

typedef struct passive passive_t;

#define PASV_NONE 0x00 // not connected
#define PASV_CONN 0x01 // TCP connection established
#define PASV_CTS  0x02 // connection is sending data to client
#define PASV_RTS  0x04 // connection is receiving data from client
#define PASV_DONE 0x08 // transfer is done
#define PASV_FREE 0x10 // ready for GC (detached from client)

struct passive {
  passive_t *next;
  u8 state;
  s32 listen_fd; // fd for the socket we are listening on
  s32 client_fd; // fd for the established client connection
  s32 file_fd;  // fd for the file being written to/from
  io_buffer_t *buffer; // buffer to use for data transfer
  u32 ip;
  u16 port;
};


passive_t *new_passive(void);
void free_passive(passive_t *passive);
void passive_poll(void);

#endif /* _PASSIVE_H */
