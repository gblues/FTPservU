#define _GNU_SOURCE
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>

#include <fat.h>
#include <iosuhax.h>
#include <iosuhax_devoptab.h>

#include "fs/fs_utils.h"
#include "fs/sd_fat_devoptab.h"
#include "vfs.h"


static char *vfs_list_root_directory(void);
static char *vfs_list_virtual_directory(char *virtual_path);

static int get_buffer_size(DIR *dir)
{
  if(dir == NULL)
    return 0;

  int size = 0;
  struct dirent *dirent;

  while( (dirent = readdir(dir)) != NULL )
  {
    // each line will be 40 characters + filename + \r\n
    size += strlen(dirent->d_name) + 42;
  }

  rewinddir(dir);
  return size;
}

static int strnumlen(long number)
{
  long n = (number > 0) ? number : -number;
  int len = 1;

  while(n > 10) {
    len++;
    n /= 10;
  }

  if(len > 3)
    len += (len / 3);

  return (number > 0) ? len : len+1;
}

static void format_filesize(char *buffer, off_t filesize)
{
  char *head = buffer + strlen(buffer);
  int width = strnumlen(filesize);
  int padding = 14 - width;

  if(padding < 0) padding = 0;

  int count = 0;

  for(int i = 0; i < padding; i++)
    head[i] = ' ';

  for(int i = 13; i >= padding; i--)
  {
    if(count == 3) {
      head[i] = ',';
      count = 0;
    } else {
      int digit = filesize % 10;
      head[i] = '0' + digit;
      filesize -= digit;
      filesize /= 10;
      count++;
    }
  }
  head[14] = ' ';
}

static void format_modifytime(char *buffer, time_t modified)
{
  char *tail = buffer + strlen(buffer);
  int result;

  // "MMM dd YYYY HH:MM " + null terminator = 20
  result = strftime(tail, 20, "%b %d %Y %H:%M ", localtime(&modified));

  if(result < 0)
    memset(tail, ' ', 19);
}

static char *build_directory_list(DIR *dir, char *parent, int size)
{
  char *buffer = (char *)malloc(size);
  char *abs_path;
  struct dirent *dirent;
  struct stat statbuf;

  if(buffer == NULL) {
    printf("[vfs]: failed to allocate %d bytes\n", size);
    return NULL;
  }

  while( (dirent = readdir(dir)) != NULL ) {
    printf("[vfs]: processing dirent: %s\n", dirent->d_name);
    asprintf(&abs_path, "%s/%s", parent, dirent->d_name);
    if(abs_path != NULL) {
      printf("[vfs]: attempting to stat %s\n", abs_path);
      memset(&statbuf, 0, sizeof(statbuf));
      if(stat(abs_path, &statbuf) < 0) {
        printf("[vfs]: failed to stat '%s'\n", abs_path);
        continue;
      }
      // directory marker
      if( statbuf.st_mode & S_IFMT == S_IFDIR ) {
        strcat(buffer, "<DIR> ");
      } else {
        strcat(buffer, "      ");
      }
      // file size, formatted with commas
      format_filesize(buffer, statbuf.st_size);
      // modify timestamp in MMM dd YYYY HH:MM format
      format_modifytime(buffer, statbuf.st_mtime);
      strcat(buffer, dirent->d_name);
      strcat(buffer, "\r\n");
    }
  }
  rewinddir(dir);
  printf("[vfs]: done building directory list:\n%s", buffer);
  return buffer;
}

char *vfs_list_directory(vfs_path_t *v_dir)
{
  char *parent = vfs_path_acquire(v_dir);
  char *result = NULL;

  printf("Attempting to build directory for path: %s\n", parent);

  if(!strcmp(parent, "/"))
    result = vfs_list_root_directory();
  else
    result = vfs_list_virtual_directory(parent);

  free(parent);
  return result;
}

static char *vfs_list_root_directory(void)
{
  vfs_volume_t *volumes = vfs_volume_get_list();
  int size = 0;

  if(volumes == NULL)
    return strdup("No volumes are mounted.\r\n");

  for(vfs_volume_t *v = volumes; v != NULL; v = v->next) {
    // each line will be 40 characters + filename + \r\n
    size += strlen(v->name) + 42;
  }

  char *result = (char *)malloc(size);
  if(result == NULL)
    return NULL;

  memset(result, 0, size);
  for(vfs_volume_t *v = volumes; v != NULL; v = v->next) {
    strcat(result, "<DIR>");
    memset(result+5, ' ', 35);
    strcat(result, v->name);
    strcat(result, "\r\n");
  }

  return result;
}

/**
 * Given a path, split out the volume name from the file path.
 * The volume name is the child of the virtual root directory,
 * so e.g. given the path /foo/bar then the volume is "foo" and the path
 * is "/bar."
 *
 * This is not even remotely thread safe.
 */
int vfs_path_split(char *virtual_path, char **volptr, char **pathptr)
{
  static char volume[256];
  static char path[PATH_MAX];

  if(virtual_path == NULL || volptr == NULL || pathptr == NULL) {
    printf("[vfs]: null pointers passed in to vfs_path_split\n");
    return -1;
  }

  if( !strcmp(virtual_path, "/")) {
    printf("[vfs]: cannot split root directory.\n");
    return -1;
  }

  if( virtual_path[0] != '/' ) {
    printf("[vfs]: cannot split relative path '%s'\n", virtual_path);
    return -1;
  }

  int i = 0;
  char *start = virtual_path + 1;

  for(i = 0; start[i] != '\0'; i++)
    if(start[i] == '/')
      break;

  if(i >= 256)
  {
    printf("[vfs]: expecting volume name < 256, got %d\n", i);
    return -1;
  }
  if( start[i] == '\0' ) {
    strcpy(volume, start);
    strcpy(path, "/");
  } else {
    int pathlen = strlen(&start[i+1]);
    if(pathlen >= PATH_MAX) {
      printf("[vfs]: path too long (expected < %d (PATH_MAX), got %d\n", PATH_MAX, pathlen);
      return -1;
    }
    if(pathlen == 0)
      strcpy(path, "/");
    else {
      strncpy(path, &start[i+1], pathlen);
      path[pathlen] = '\0';
    }
  }
  *volptr = volume;
  *pathptr = path;

  return 0;
}

static char *vfs_list_virtual_directory(char *virtual_path)
{
  char *real_path;
  char *volume = NULL;
  char *path = NULL;
  char *result = NULL;

  if( vfs_path_split(virtual_path, &volume, &path) < 0 )
    return NULL;

  asprintf(&real_path, "%s:%s", volume, path);
  if(real_path == NULL)
    return NULL;

  DIR *dir = opendir(real_path);
  if(dir != NULL) {
    printf("Opened directory sucessfully.\n");
    result = build_directory_list(dir, real_path, get_buffer_size(dir));
    closedir(dir);
  }

  free(real_path);
  return result;
}

vfs_node_t *new_vfs_node(void)
{
  vfs_node_t *result = (vfs_node_t *)malloc(sizeof(vfs_node_t));
  if(result != NULL) {
    memset(result, 0, sizeof(vfs_node_t));
  }

  return result;
}

/*
 * next and children pointers must be nullified before calling!
 */
int free_vfs_node(vfs_node_t *node)
{
  if(node == NULL)
    return 0;

  if(node->next || node->children)
    return -1;

  if(node->name)
  {
    free(node->name);
    node->name = NULL;
  }

  memset(node, 0, sizeof(vfs_node_t));
  free(node);
}

static int fsa_fd = -1;

void vfs_fsa_init(void) {
  if( vfs_volume_register("sd") )
    fatInitDefault();

  fsa_fd = IOSUHAX_FSA_Open();
}

void vfs_mount(const char *name, const char *device, const char *volume)
{
  if(fsa_fd < 0)
  {
    printf("[vfs]: cannot mount '%s': IOSUHAX is not initialized\n", name);
    return;
  }

  if( vfs_volume_register(name) )
    mount_fs(name, fsa_fd, device, volume);
}

void vfs_mount_fat(const char *name)
{
  if( vfs_volume_register(name) )
    mount_sd_fat(name);
}
