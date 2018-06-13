#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include "vfs.h"

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
