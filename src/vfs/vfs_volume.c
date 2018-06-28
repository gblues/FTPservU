#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "vfs.h"

vfs_volume_t *mounted_volumes;

vfs_volume_t *vfs_volume_get_list(void) {
  return mounted_volumes;
}

vfs_volume_t *new_vfs_volume(const char *name) {
  vfs_volume_t *volume = (vfs_volume_t *)malloc(sizeof(vfs_volume_t));
  if(volume != NULL)
    volume->name = strdup(name);

  return volume;
}

void free_vfs_volume(vfs_volume_t *volume) {
  if(volume == NULL)
    return;

  volume->next = NULL;

  if(volume->name)
    free(volume->name);

  free(volume);
}

vfs_volume_t *vfs_find_volume(const char *name)
{
  if(name == NULL || mounted_volumes == NULL)
    return NULL;

  for(vfs_volume_t *mount = mounted_volumes; mount != NULL; mount = mount->next)
    if(!strcmp(name, mount->name))
      return mount;

  printf("[vfs]: Failed to find mounted volume '%s'", name);
  return NULL;
}

bool vfs_volume_register(const char *name)
{
  vfs_volume_t *mount = new_vfs_volume(name);
  if(mount == NULL)
    return false;

  if( vfs_find_volume(name) != NULL ) {
    printf("[vfs]: volume '%s' is already registered.\n", name);
    free_vfs_volume(mount);
    return false;
  }

  mount->next = mounted_volumes;
  mounted_volumes = mount;
  return true;
}

bool vfs_volume_unregister(char *name) {
  if(name == NULL || mounted_volumes == NULL)
    return false;

  if(!strcmp(name, mounted_volumes->name)) {
    vfs_volume_t *head = mounted_volumes->next;
    free_vfs_volume(mounted_volumes);
    mounted_volumes = head;
    return true;
  }

  vfs_volume_t *prev = mounted_volumes;
  for(vfs_volume_t *volume = mounted_volumes->next; volume != NULL; volume = volume->next) {
    if(!strcmp(name, volume->name)) {
      prev->next = volume->next;
      free_vfs_volume(volume);
      return true;
    }
    prev = volume;
  }

  return false;
}
