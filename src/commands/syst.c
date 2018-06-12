#include "commands.h"
#include "ftp.h"

void do_system(client_t *client, char *args)
{
  ftp_response(215, client, "UNIX Type: L8 Version: FTPservU");
}
