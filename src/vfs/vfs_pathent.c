#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "vfs.h"

static char *skip_separator(char *path)
{
  if(path == NULL)
    return NULL;

  while(path[0] == '/')
    path++;

  return path;
}

static vfs_pathent_t *pathent_list_append(vfs_pathent_t *list, vfs_pathent_t *pathent)
{
  if(list == NULL)
    return pathent;

  vfs_pathent_t *head = list;

  while(list->next)
    list = list->next;
  list->next = pathent;

  return head;
}

/**
 * Transforms a user-specified path into a linked-list of potential directories.
 * Returns NULL if the path is invalid or empty.
 */

vfs_pathent_t *vfs_pathent_parse(char *path)
{
  if(path == NULL || path[0] == '\0')
    return NULL;

  char *entity;
  vfs_pathent_t *pathent_list = NULL;
  vfs_pathent_t *pathent = NULL;

  path = skip_separator(path);
  entity = path;

  for(int i = 0; path[i] != '\0'; i++)
  {
    if(!isgraph(path[i]) && path[i] != ' ')
      goto error;

    if(path[i] == '/')
    {
      path[i] = '\0';
      pathent = new_vfs_pathent(entity);

      if(pathent == NULL)
        goto error;

      pathent_list = pathent_list_append(pathent_list, pathent);
      path[i] = '/';

      while(path[i] == '/')
        i++;

      entity = &path[i];
    }
  }

  if(entity[0] != '\0') {
    pathent = new_vfs_pathent(entity);

    if(pathent == NULL)
      goto error;

    pathent_list = pathent_list_append(pathent_list, pathent);
  }

  return pathent_list;

  error:
    printf("Could not parse '%s' into a path\n", path);
    free_vfs_pathent(pathent_list);
    return NULL;
}

vfs_pathent_t *new_vfs_pathent(char *entity)
{
  if(entity == NULL)
    return NULL;

  vfs_pathent_t *result = (vfs_pathent_t *)malloc(sizeof(vfs_pathent_t));
  if(result != NULL) {
    memset(result, 0, sizeof(vfs_pathent_t));
    result->name = strdup(entity);
  }

  return result;
}

void free_vfs_pathent(vfs_pathent_t *entity)
{
  vfs_pathent_t *next;

  while(entity != NULL)
  {
    next = entity->next;
    entity->next = NULL;
    free(entity);
    entity = next;
  }
}
