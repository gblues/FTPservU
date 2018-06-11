#ifndef _NETWORK_H
#define _NETWORK_H

#define FTP_SOCKET_BACKLOG 10

#include "netinet/in.h"
#include "sys/socket.h"

#include "ftpservu_types.h"

void network_init(void);
int network_create_serversocket(int port, int backlog);
int network_accept_poll(int socket, accept_cb callback, void *userptr);
s32 network_close(int socket);
int network_read_buffer(int socket, io_buffer_t *buffer);
int network_writeln(int socket, char *line);
u32 network_get_host_ip(void);
u16 network_get_ephermal_port(void);

#endif /* _NETWORK_H */
