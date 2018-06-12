#include "commands.h"
#include "ftp.h"

void do_quit(client_t *client, char *args)
{
  client->state |= STATE_DISCONN;
  ftp_response(221, client, "Service closing control connection.");
}
