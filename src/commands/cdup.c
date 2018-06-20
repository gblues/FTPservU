#include "commands.h"
#include "ftp.h"
#include "vfs/vfs.h"

static void detach_last_pathent(vfs_path_t *cwd)
{
  if(cwd == NULL || cwd->entities == NULL)
    return;

  if(cwd->entities->next == NULL) {
    free_vfs_pathent(cwd->entities);
    cwd->entities = NULL;
  } else {
    vfs_pathent_t *parent;
    for(vfs_pathent_t *ent = cwd->entities; ent != NULL; ent = ent->next)
    {
      if(ent->next != NULL)
        parent = ent;
    }
    if(parent != NULL)
    {
      free_vfs_pathent(parent->next);
      parent->next = NULL;
    }
  }
}

/*
 * Go up one directory level.
 * This command never fails. It always gives the 250 response,
 * even if already at the root directory.
 */
void do_cdup(client_t *client, char *args)
{
  detach_last_pathent(client->cwd);
  ftp_response(250, client, "CWD command successful.");
}
