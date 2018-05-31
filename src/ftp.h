#ifndef _FTP_H
#define _FTP_H

#include "wiiu/types.h"
#include "iobuffer.h"
#include "commands.h"
#include "ftpservu_types.h"

struct client_struct {
  s32 fd;
  io_buffer_t *input_buffer;
  bool authenticated;
  void *data_connection;
};

int ftp_network_handler(int socket);
void ftp_deinit(void);

#endif /* _FTP_H */
