#include "commands.h"
#include "ftp.h"

void do_noop(client_t *client, char *args)
{
  ftp_response(200, client, "Okey dokey artichokey!");
}
