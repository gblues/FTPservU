#include <malloc.h>
#include <stdlib.h>
#include "vfs.h"

vfs_path_t *new_vfs_path(char *path)
{
  if(path == NULL || path[0] == '\0')
    return NULL;

  vfs_path_t *result = (vfs_path_t *)malloc(sizeof(vfs_path_t));

  if(path[0] == '/')
    result->absolute = true;

  result->entities = vfs_pathent_parse(path);

  return result;
}

void free_vfs_path(vfs_path_t *path)
{
  if(path == NULL)
    return;

  if(path->entities != NULL)
  {
    free_vfs_pathent(path->entities);
    path->entities = NULL;
  }

  free(path);
}
