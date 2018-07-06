#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys/socket.h"

#include "console.h"
#include "iobuffer.h"
#include "dtp.h"
#include "network.h"

static void pasv_accept_handler(int fd, struct sockaddr_in *sockaddr, socklen_t size, void *data)
{
  data_channel_t *pasv = (data_channel_t *)data;

  if(sockaddr == NULL)
  {
    printf("[pasv]: accept handler received null sockaddr\n");
    return;
  }

  console_printf("Accepted PASV data connection from %d.%d.%d.%d!",
    (sockaddr->sin_addr.s_addr & 0xff000000) >> 24,
    (sockaddr->sin_addr.s_addr & 0x00ff0000) >> 16,
    (sockaddr->sin_addr.s_addr & 0x0000ff00) >> 8,
    (sockaddr->sin_addr.s_addr & 0x000000ff) );

  network_close(pasv->listen_fd);
  pasv->listen_fd = -1;

  pasv->remote = new_xfer_socket(fd);
  SET_STATE(pasv, DTP_ESTABLISHED);
}

static void pasv_try_accept(data_channel_t *channel)
{
  if(!channel || GET_STATE(channel) != DTP_PENDING)
    return;

  if( network_accept_poll(channel->listen_fd, pasv_accept_handler, channel) < 0 )
    SET_STATE(channel, DTP_ERROR);
}

static data_channel_t *new_passive_channel(u32 ip, u16 port)
{
  printf("[passive] new_passive_channel\n");
  data_channel_t *result = base.new(ip, port);
  if(result != NULL)
  {
    result->listen_fd = network_create_serversocket(result->port, 1);
    if(result->listen_fd < 0)
      goto error;
  }
  result->iface = &passive;
  return result;

  error:
  if(result)
    base.free(result);

  return NULL;
}

static void pasv_send_data(data_channel_t *channel) {
  base.send(channel);
}

static void pasv_receive_data(data_channel_t *channel) {
  base.recv(channel);
}

static void free_passive_channel(data_channel_t *channel)
{
  base.free(channel);
}

/*
 * We always return true here because this is primarily used
 * by data commands to detect if a 4xx response needs to be sent--
 * which is generally only in active mode.
 *
 * In a real FTP server, it will wait forever for something to
 * connect to the PASV port.
 */
static bool pasv_is_connected(data_channel_t *channel)
{
  return true;
}

data_interface_t passive = {
  new_passive_channel,
  free_passive_channel,
  pasv_try_accept,
  pasv_is_connected,
  pasv_send_data,
  pasv_receive_data
};
