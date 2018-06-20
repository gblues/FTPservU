#include "commands.h"
#include "ftp.h"

void do_cwd(client_t *client, char *args)
{
  vfs_path_t *parsed = new_vfs_path(args);
  vfs_path_t *result = vfs_path_merge(client->cwd, parsed);

  if(result != NULL && vfs_can_chdir(result)) {
    client->cwd = result;
    ftp_response(250, client, "CWD command successful.");
  } else {
    ftp_response(550, client, "That directory does not exist.");
  }
}
