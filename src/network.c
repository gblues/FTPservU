#include <string.h>

#include "network.h"

static int setup_server_socket(void)
{
  u32 so_reuseaddr  = 1;
  u32 so_nonblock = 1;
  /* get the socket from the OS */
  int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if(sock < 0)
    return -1;

  /* Set to non-blocking I/O */
  setsockopt(sock, SOL_SOCKET, SO_NONBLOCK, &so_nonblock, sizeof(so_nonblock));
  /* Allow address re-use */
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof(so_reuseaddr));

  return sock;
}

static int bind_to_port(int socket, int port)
{
  struct sockaddr_in bindAddress;

  memset(&bindAddress, 0, sizeof(bindAddress));
  bindAddress.sin_family = AF_INET;
  bindAddress.sin_port = htons(port);
  bindAddress.sin_addr.s_addr = htonl(INADDR_ANY);

  return bind(socket, (struct sockaddr *)&bindAddress, sizeof(bindAddress));
}

int network_create_serversocket(int port)
{
  int socket = 0;

  socket = setup_server_socket();

  if( (socket < 0) ||
      (bind_to_port(socket,port) < 0) ||
      (listen(socket, FTP_SOCKET_BACKLOG) < 0) )
  {
    if(socket >= 0)
      socketclose(socket);
    return -1;
  }

  return socket;
}

int network_accept_poll(int socket, accept_cb callback)
{
  struct sockaddr_in client;
  socklen_t len;

  bool socketqueue_exhausted = false;
  int errno;

  if(callback == NULL)
    return -1;

  do {
    memset(&client, 0, sizeof(client));
    int fd = accept(socket, (struct sockaddr *)&client, &len);
    if(fd < 0)
    {
      errno = socketlasterr();
      if( errno == EAGAIN || errno == EWOULDBLOCK )
        socketqueue_exhausted = true;
      else
        return -1;
    } else {
      callback(fd, &client, len);
    }
  } while(!socketqueue_exhausted);
}

s32 network_close(int socket)
{
  if(socket < 0)
    return socket;

  return socketclose(socket);
}

int network_read_buffer(int socket, io_buffer_t *buffer)
{
  if(socket < 0)
    return socket;

  int nread;
  int errno;

  nread = recv(socket, iobuffer_head(buffer), iobuffer_remaining(buffer), 0);
  if(nread < 0)
  {
    errno = socketlasterr();
    return -errno;
  }

  buffer->head += nread;
  memset(iobuffer_head(buffer), 0, iobuffer_remaining(buffer));

  return nread;
}
