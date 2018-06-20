#include "commands.h"
#include "ftp.h"

void do_list(client_t *client, char *args)
{
  client->state |= STATE_DATA;
  char *buffer = vfs_list_directory(client->cwd);
  /* TODO: write directory listing to temp file */
  /* TODO: open tempfile and set local_fd */
  if(!client->data->iface->is_connected(client->data))
    ftp_response(425, client, "Failed to connect to client");

  if( dtp_send_buffer(client->data, buffer) < 0 )
    ftp_response(425, client, "Failed to send directory listing");
}
