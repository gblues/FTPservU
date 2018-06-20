#include <stdlib.h>

#include "commands.h"
#include "ftp.h"
#include "vfs/vfs.h"

void do_pwd(client_t *client, char *args)
{
  char *pwd = vfs_path_acquire(client->cwd);
  ftp_responsef(257, client, "\"%s\" is current directory.", pwd);
  free(pwd);
}
