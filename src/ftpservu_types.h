#ifndef _FTPSERVU_TYPES_H
#define _FTPSERVU_TYPES_H

#include <netinet/in.h>
#include <sys/socket.h>

/*
 * Typedefs go in this file so they can be used anywhere to avoid
 * circular include dependencies.
 */

typedef struct data_channel data_channel_t;
typedef struct data_interface data_interface_t;
typedef struct client_struct client_t;
typedef struct command command_t;
typedef struct io_buffer io_buffer_t;
typedef struct vfs_node vfs_node_t;
typedef struct vfs_path vfs_path_t;
typedef struct vfs_pathent vfs_pathent_t;
typedef void (*accept_cb)(int fd, struct sockaddr_in *sock, socklen_t len, void *usrptr);

#endif /* _FTPSERVU_TYPES_H */
