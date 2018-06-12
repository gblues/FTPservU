#include "commands.h"
#include "ftp.h"

void do_pass(client_t *client, char *args)
{
  switch(client->state)
  {
    /* no user given yet */
    case STATE_NONE:
      ftp_response(332, client, "Need account for login.");
      break;
    /* user has been given */
    case STATE_USER:
      client->state = STATE_AUTH;
      ftp_response(230, client, "User logged in successfully.");
      break;
    /* user is already authenticated */
    default:
      ftp_response(503, client, "Bad sequence of commands.");
      break;
  }
}
