#include <string.h>

#include "console.h"
#include "network.h"
#include "iobuffer.h"
#include "wiiu/ac.h"

static unsigned next_ephermal_port = 1024;

/*
 * Tell the Wii U to connect to its default network profile, then init the socket library.
 */
void network_init(void)
{
  ACInitialize();
  ACConnect();

  socket_lib_init();
}

u32 network_get_host_ip(void)
{
  u32 ip = 0;

  ACGetAssignedAddress(&ip);
  return ip;
}

u16 network_get_ephermal_port(void)
{
  u16 result = next_ephermal_port;

  next_ephermal_port++;
  if(next_ephermal_port > 65535)
    next_ephermal_port = 1024;

  printf("[network]: returned ephermal port %d\n", result);

  return result;
}

static int setup_server_socket(void)
{
  int errno = 0;
  u32 so_reuseaddr  = 1;
  u32 so_nonblock   = 1;

  /* get the socket from the OS */
  int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if(sock < 0)
  {
    errno = socketlasterr();
    console_printf("FATAL: failed to create socket: %d", errno);
    return -1;
  }

  /* Set to non-blocking I/O */
  setsockopt(sock, SOL_SOCKET, SO_NONBLOCK, &so_nonblock, sizeof(so_nonblock));
  /* Allow address re-use */
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof(so_reuseaddr));

  return sock;
}

static int bind_to_port(int socket, int port)
{
  struct sockaddr_in bindAddress;
  int result = -1;
  int errno = 0;

  memset(&bindAddress, 0, sizeof(bindAddress));
  bindAddress.sin_family = AF_INET;
  bindAddress.sin_port = htons(port);
  bindAddress.sin_addr.s_addr = htonl(INADDR_ANY);

  result = bind(socket, (struct sockaddr *)&bindAddress, sizeof(bindAddress));
  if(result < 0)
  {
    errno = socketlasterr();

    console_printf("FATAL: Failed to bind socket to port %d: %d", port, errno);
  }

  return result;
}

int network_create_serversocket(int port, int backlog)
{
  int socket = 0;
  int result = -1;
  int errno = 0;

  printf("[network]: attempting to create server socket for port %d with connect backlog of %d\n", port, backlog);

  socket = setup_server_socket();
  if(socket < 0) return -1;

  if(bind_to_port(socket, port) < 0)
  {
    socketclose(socket);
    return -1;
  }

  result = listen(socket, backlog);
  if(result < 0)
  {
    errno = socketlasterr();
    console_printf("FATAL: failed to listen to port %d with backlog %d: %d",
      port, FTP_SOCKET_BACKLOG, errno);
    socketclose(socket);
    return -1;
  }

  return socket;
}

int network_accept_poll(int socket, accept_cb callback, void *userptr)
{
  struct sockaddr_in client;
  socklen_t len = sizeof(client);
  bool socketqueue_exhausted = false;
  int errno;

  if(callback == NULL)
  {
    console_printf("FATAL: no callback given to network_accept_poll");
    return -1;
  }

  printf("[network] accept poll\n");

  memset(&client, 0, sizeof(client));
  printf("socket: %d, len: %d\n", socket, len);
  int fd = accept(socket, (struct sockaddr *)&client, &len);
  if(fd < 0)
  {
    errno = socketlasterr();
    if( errno == EAGAIN || errno == EWOULDBLOCK )
      return 0;
    else
    {
      console_printf("FATAL: accept failed (%d)", errno);
      return -1;
    }
  }

  callback(fd, &client, len, userptr);

  printf("[network] done network polling\n");
  return 0;
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

  if( iobuffer_remaining(buffer) == 0)
    return -ENOBUFS;

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

int network_writeln(int socket, char *line)
{
  if(socket < 0)
    return -1;

  int len = strlen(line);

  if(len == 0)
    return 0;

  int nwritten = send(socket, line, len, 0);

  if(nwritten < 0)
    nwritten = -socketlasterr();

  return nwritten;
}

s32 network_connect(u32 remote_ip, u16 remote_port)
{
  return -1;
}
