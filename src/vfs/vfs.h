#ifndef _VFS_H
#define _VFS_H

#include "ftpservu_types.h"
#include "wiiu/types.h"

/**
 * A representation of the actual filesystem data.
 */
struct vfs_node {
  char *name;
  vfs_node_t *next;
  vfs_node_t *children;
  size_t size;
  u32 created;
  u32 modified;
  u8 attrib;
};

/*
 * One element in a user-specified path. May or may not exist.
 */
struct vfs_pathent {
  vfs_pathent_t *next;
  char *name;
};

/**
 * This is a user-specified path normalized into a linked list of
 * directory names, so e.g. /usr/local/bin becomes usr -> local -> bin
 */
struct vfs_path {
  vfs_pathent_t *entities;
  bool absolute;
};

vfs_pathent_t *vfs_pathent_parse(char *path);
void vfs_pathent_free(vfs_pathent_t *entity);
vfs_node_t *new_vfs_node(void);
int free_vfs_node(vfs_node_t *node);
vfs_path_t *new_vfs_path(char *path);
void free_vfs_path(vfs_path_t *path);
void free_vfs_pathent(vfs_pathent_t *entity);

#endif /* _VFS_H */
