#include "commands.h"
#include "ftp.h"

void do_reinitialize(client_t *client, char *args)
{
  client->state = STATE_NONE;
  ftp_response(220, client, "Service ready for new user.");
}
