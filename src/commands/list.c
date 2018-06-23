#include <stdio.h>

#include "commands.h"
#include "ftp.h"

void do_list(client_t *client, char *args)
{
  char *dir_listing = vfs_list_directory(client->cwd);
  printf("DIRECTORY LISTING:\n%s", dir_listing);

  ftp_data_send_buffer(client, dir_listing);
}
