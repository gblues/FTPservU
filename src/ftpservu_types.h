#ifndef _FTPSERVU_TYPES_H
#define _FTPSERVU_TYPES_H

#include <netinet/in.h>
#include <sys/socket.h>

/*
 * Typedefs go in this file so they can be used anywhere to avoid
 * circular include dependencies.
 */

typedef struct client_struct client_t;
typedef struct io_buffer io_buffer_t;
typedef void (*accept_cb)(int fd, struct sockaddr_in *sock, socklen_t len);

#endif /* _FTPSERVU_TYPES_H */
