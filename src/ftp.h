#ifndef _FTP_H
#define _FTP_H

#include "wiiu/types.h"
#include "iobuffer.h"

typedef struct buffer buffer_t;

struct client_struct {
  s32 fd;
  io_buffer_t *input_buffer;
  void *data_connection;
};

typedef struct client_struct client_t;

int ftp_network_handler(int socket);
void ftp_deinit(void);

#endif /* _FTP_H */
