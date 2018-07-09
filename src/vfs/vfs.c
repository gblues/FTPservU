#define _GNU_SOURCE
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>

#include "vfs.h"

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

  if(buffer == NULL)
    return NULL;

  while( (dirent = readdir(dir)) != NULL ) {
    asprintf(&abs_path, "%s/%s", parent, dirent->d_name);
    if(abs_path != NULL) {
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
  return buffer;
}

char *vfs_list_directory(vfs_path_t *v_dir)
{
  char *parent = vfs_path_acquire(v_dir);
  char *result = NULL;

  DIR *dir = opendir(parent);
  if(dir != NULL) {
    result = build_directory_list(dir, parent, get_buffer_size(dir));
    closedir(dir);
  }

  free(parent);
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
