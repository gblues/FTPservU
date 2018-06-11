#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys/socket.h"

#include "console.h"
#include "iobuffer.h"
#include "passive.h"
#include "dtp.h"
#include "network.h"
#include "wiiu/ac.h"

static int pasv_port = 1024;

static void free_passive_channel(data_channel_t *channel);

/*
 * This list is read-only.
 */
static data_channel_t *pasv_list = NULL;
/*
 * This list is write-only. Do not iterate over this list.
 */
static data_channel_t *next_pasv_list = NULL;

static int get_pasv_port(void)
{
  int result = pasv_port;

  pasv_port++;
  if(pasv_port > 65535)
    pasv_port = 1024;

  return result;
}

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

  pasv->remote_fd = fd;
  pasv->state = DTP_ESTABLISHED;
}

static void pasv_try_accept(data_channel_t *channel)
{
  if(!channel || channel->state != DTP_PENDING)
    return;

  if( network_accept_poll(channel->listen_fd, pasv_accept_handler, channel) < 0 )
    channel->state = DTP_ERROR;
}

static int write_buffered_data_to_fd(int fd, io_buffer_t *buffer, int packetSize)
{
  int nwritten = 0;

  if(fd < 0 || buffer == NULL || packetSize < 0)
    return -1;

  if(packetSize > buffer->head)
    packetSize = buffer->head;

  nwritten = write(fd, buffer->buffer, packetSize);

  if(nwritten < 0)
  {
    int errno = socketlasterr();
    if(errno != EAGAIN)
    {
      printf("[passive]: error writing data to client: %d\n", errno);
      return -errno;
    }
    return 0;
  }

  memmove(buffer->buffer, buffer->buffer+nwritten, buffer->size - nwritten);
  buffer->head -= nwritten;

  return nwritten;
}

int fill_buffer_from_fd(io_buffer_t *buffer, int fd)
{
  if(buffer->head > 0)
    return buffer->head;

  int nread = read(fd, buffer->buffer, buffer->size);
  if(nread >= 0)
    buffer->head = nread;

  return nread;
}

static void copy_data(int *to, int *from, data_channel_t *channel, int size)
{
  int nwritten = 0;
  int nread = 0;


  if(to     == NULL ||
       from == NULL ||
    channel == NULL ||
            *to < 0 ||
          *from < 0)
    return;

  nwritten = write_buffered_data_to_fd(*to, channel->buffer, size);
  if(nwritten < 0)
    goto error;

  nread = fill_buffer_from_fd(channel->buffer, *from);

  if(nread < 0)
    goto error;

  if(nread == 0)
  {
    close(*to);
    close(*from);
    *to = -1;
    *from = -1;
    channel->state = DTP_CLOSED;
  }

  return;

  error:
  channel->state = DTP_ERROR;
}

static void pasv_send_data(data_channel_t *channel)
{
  if(!channel || !channel->buffer)
    return;

  copy_data(&channel->remote_fd, &channel->local_fd, channel, 1500);
}

static void pasv_recv_data(data_channel_t *channel)
{
  if(!channel || !channel->buffer)
    return;

  copy_data(&channel->local_fd, &channel->remote_fd, channel, 512);
}

void passive_poll(void)
{
  data_channel_t *pasv = NULL;

  while(pasv_list != NULL)
  {
    pasv = pasv_list;
    pasv_list = pasv->next;

    switch(pasv->state)
    {
      case DTP_PENDING:
        pasv_try_accept(pasv);
        break;
      case DTP_XMIT:
        pasv_send_data(pasv);
        break;
      case DTP_RECV:
        pasv_recv_data(pasv);
        break;
    }

    if(pasv->state == DTP_FREE)
      free_passive_channel(pasv);
    else
    {
      pasv->next = next_pasv_list;
      next_pasv_list = pasv;
    }
  }

  pasv_list = next_pasv_list;
  next_pasv_list = NULL;
}

static data_channel_t *new_passive_channel(u32 ip, u16 port)
{
  data_channel_t *result = (data_channel_t *)malloc(sizeof(data_channel_t));
  if(result != NULL)
  {
    result->ip = ip;
    result->port = port;
    result->state = DTP_PENDING;
    result->local_fd = -1;
    result->remote_fd = -1;

    if(result->ip == 0)
      goto error;

    result->listen_fd = network_create_serversocket(result->port, 1);
    if(result->listen_fd < 0)
      goto error;
    /* 192K is 128 1500-byte packets, and also 375 512-byte blocks */
    result->buffer = new_buffer(192000);
    if(result->buffer == NULL)
      goto error;

    result->next = next_pasv_list;
    next_pasv_list = result;
  }
  return result;

  error:
  if(result)
    free_passive_channel(result);

  return NULL;
}

static void free_passive_channel(data_channel_t *passive)
{
  if(passive)
  {
    passive->next = NULL;

    if(passive->listen_fd >= 0)
    {
      socketclose(passive->listen_fd);
      passive->listen_fd = -1;
    }
    if(passive->remote_fd >= 0)
    {
      socketclose(passive->remote_fd);
      passive->remote_fd = -1;
    }
    if(passive->local_fd)
    {
      close(passive->local_fd);
      passive->local_fd = -1;
    }
    if(passive->buffer)
    {
      free_buffer(passive->buffer);
      passive->buffer = NULL;
    }
    passive->port = 0;
    passive->ip = 0;
    free(passive);
  }
}

data_interface_t passive = {
  new_passive_channel,
  free_passive_channel
};
