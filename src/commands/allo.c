#include "commands.h"
#include "ftp.h"

/*
 * "This command may be required by some servers to reserve sufficent
 *  storage to accommodate the new file to be transferred."
 *  - RFC 959 4.1.3
 *
 * The Wii U is not such a server, so as per the RFC, we make it a
 * no-op.
 */
void do_allocate(client_t *client, char *args)
{
  ftp_response(202, client, "ALLO command ignored.");
}
