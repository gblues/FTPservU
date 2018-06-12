#include "commands.h"
#include "ftp.h"

void do_user(client_t *client, char *args)
{
  client->state = STATE_USER;
  ftp_response(331, client, "Username OK. Ready for password.");
}
