#ifndef _FTP_H
#define _FTP_H

#include <limits.h>
#include "wiiu/types.h"
#include "iobuffer.h"
#include "commands.h"
#include "dtp/passive.h"
#include "ftpservu_types.h"

#define STATE_NONE 0x00000000 /* initial state */
#define STATE_USER 0x00000001 /* USER has been provided */
#define STATE_AUTH 0x00000002 /* client is authenticated */
#define STATE_RNFR 0x00000004 /* client sent RNFR */
#define STATE_DISCONN 0x0000008 /* client logged out */

#define IS_AUTHENTICATED(client) (client->state >= STATE_AUTH)

struct client_struct {
  s32 fd;
  u32 state;
  io_buffer_t *input_buffer;
  io_buffer_t *output_buffer;
  passive_t *passive;

  uint8_t cwd[PATH_MAX];

  void *data_connection;
};

int ftp_network_handler(int socket);
void ftp_deinit(void);

void ftp_response(int code, client_t *client, const char *msg);

#endif /* _FTP_H */
