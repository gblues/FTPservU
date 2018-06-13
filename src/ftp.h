#ifndef _FTP_H
#define _FTP_H

#include <limits.h>
#include "wiiu/types.h"
#include "iobuffer.h"
#include "commands.h"
#include "dtp/dtp.h"
#include "vfs/vfs.h"
#include "ftpservu_types.h"

#define STATE_NONE 0x00000000 /* initial state */
#define STATE_USER 0x00000001 /* USER has been provided */
#define STATE_AUTH 0x00000002 /* client is authenticated */
#define STATE_RNFR 0x00000004 /* client sent RNFR */
#define STATE_DATA 0x00000008 /* data transfer in progress */
#define STATE_DISCONN 0x0000010 /* client logged out */

#define IS_AUTHENTICATED(client) (client->state >= STATE_AUTH)

struct client_struct {
  s32 fd;
  u32 ip;
  u16 port;
  u32 state;
  io_buffer_t *input_buffer;
  io_buffer_t *output_buffer;
  data_channel_t *data;

  vfs_path_t *cwd;
};

int ftp_network_handler(int socket);
void ftp_deinit(void);

void ftp_response(int code, client_t *client, const char *msg);

#endif /* _FTP_H */
