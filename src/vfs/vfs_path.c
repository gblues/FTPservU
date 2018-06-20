#include <limits.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "vfs.h"

static vfs_path_t *new_vfs_path_internal(void)
{
  vfs_path_t *result = (vfs_path_t *)malloc(sizeof(vfs_path_t));
  if(result != NULL)
    memset(result, 0, sizeof(vfs_path_t));

  return result;
}

vfs_path_t *new_vfs_path(char *path)
{
  if(path == NULL || path[0] == '\0')
    return NULL;

  vfs_path_t *result = new_vfs_path_internal();
  if(result == NULL)
    return NULL;

  if(path[0] == '/')
    result->absolute = true;

  result->entities = vfs_pathent_parse(path);
  vfs_path_resolve(result);

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

/*
 * Transforms from struct back to a string form. Caller is responsible
 * for free()ing.
 */
char *vfs_path_acquire(vfs_path_t *path)
{
  char *buffer = (char *)malloc(PATH_MAX);
  if(buffer == NULL)
    return NULL;

  memset(buffer, 0, PATH_MAX);
  if(path->absolute)
    buffer[0] = '/';

  if(path->entities == NULL)
    return buffer;

  for(vfs_pathent_t *ent = path->entities; ent != NULL; ent = ent->next)
  {
    // existing + next segment + null terminator
    int len = strlen(buffer) + strlen(ent->name) + 1;
    // account for path separator too
    if(ent->next != NULL) len++;
    if(len >= PATH_MAX)
    {
      free(buffer);
      return NULL;
    }
    strcat(buffer, ent->name);
    if(ent->next != NULL)
      strcat(buffer, "/");
  }
  return buffer;
}

/*
 * Resolve a path containing indirect links.
 *
 * i.e. /usr/local/../bin => /usr/bin
 * or:  /usr/local/./bin => /usr/local/bin
 */
void vfs_path_resolve(vfs_path_t *path)
{
  if(path == NULL || path->entities == NULL)
    return;

  vfs_pathent_t *next_ent;
  vfs_pathent_t *stack = NULL;

  /* start by pushing the directories onto a stack. */
  for(vfs_pathent_t *ent = path->entities; ent != NULL; ent = next_ent) {
    next_ent = ent->next;

    /* if the "previous directory" reference is found, discard the top
     * of the stack */
    if( !strcmp(ent->name, "..") ) {
      if(stack != NULL) {
        vfs_pathent_t *top = stack;
        stack = top->next;
        top->next = NULL;
        free_vfs_pathent(top);
      }
    /* if the "current directory" reference is found, just discard it */
    } else if( !strcmp(ent->name, ".") ) {
      ent->next = NULL;
      free_vfs_pathent(ent);
    /* otherwise it's a path segment, push it onto the stack */
    } else {
      ent->next = stack;
      stack = ent;
    }
  }
  path->entities = NULL;
  /* now we just pop everything off the stack */
  for(vfs_pathent_t *ent = stack; ent != NULL; ent = next_ent) {
    next_ent = ent->next;
    ent->next = path->entities;
    path->entities = ent;
  }
}

vfs_path_t *vfs_path_clone(vfs_path_t *path)
{
  if(path == NULL)
    return NULL;

  vfs_path_t *result = new_vfs_path_internal();
  vfs_pathent_t *head = NULL;

  if(result != NULL) {
    result->absolute = path->absolute;
    for(vfs_pathent_t *ent = path->entities; ent != NULL; ent = ent->next)
    {
      vfs_pathent_t *copy = new_vfs_pathent(ent->name);
      if(copy == NULL)
      {
        free_vfs_path(result);
        return NULL;
      }

      if(result->entities == NULL) {
        result->entities = copy;
        head = copy;
      } else {
        head->next = copy;
        head = copy;
      }
    }
  }

  return result;
}

vfs_path_t *vfs_path_merge(vfs_path_t *dest, vfs_path_t *src)
{
  if(dest == NULL || src == NULL) {
    printf("vfs_path_merge: null parameters\n");
    return NULL;
  }
  if(!(dest->absolute)) {
    printf("vfs_path_merge: dest must be an absolute path\n");
    return NULL;
  }

  /* if src is absolute, it just overrides dest */
  if(src->absolute) {
    return vfs_path_clone(src);
  }

  /* nothing to do, just return dest */
  if(src->entities == NULL) {
    return vfs_path_clone(dest);
  }

  vfs_path_t *result = vfs_path_clone(dest);
  vfs_path_t *tmp = vfs_path_clone(src);

  if(result->entities == NULL)
    result->entities = tmp->entities;
  else {
    vfs_pathent_t *tail;
    for(tail = result->entities; tail->next != NULL; tail = tail->next);
    tail->next = tmp->entities;
  }

  tmp->entities = NULL;
  free_vfs_path(tmp);

  vfs_path_resolve(result);

  return result;
}

/*
 * Returns true if "path" refers to a directory.
 * Returns false if not a directory or doesn't exist
 */
bool vfs_can_chdir(vfs_path_t *path)
{
  int ret = 0;

  if(!path->absolute)
    return false;
  /* root directory */
  if(path->absolute && path->entities == NULL)
    return true;

  struct stat file_status;
  char *pathstr = vfs_path_acquire(path);
  if(pathstr == NULL)
    return false;
  ret = stat(pathstr, &file_status);
  free(pathstr);

  if( ret < 0)
    return false;

  return S_ISDIR(file_status.st_mode);
}
