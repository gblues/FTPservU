#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys/socket.h"

#include "console.h"
#include "iobuffer.h"
#include "dtp.h"
#include "network.h"
#include "wiiu/ac.h"

static data_channel_t *dtp_list = NULL;
static data_channel_t *next_dtp_list = NULL;

static int dtp_write_buffered_data_to_fd(int fd, io_buffer_t *buffer, int packetSize)
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

static int dtp_fill_buffer_from_fd(io_buffer_t *buffer, int fd)
{
  if(buffer->head > 0)
    return buffer->head;

  int nread = read(fd, buffer->buffer, buffer->size);
  if(nread >= 0)
    buffer->head = nread;

  return nread;
}

static void dtp_copy_data(int *to, int *from, data_channel_t *channel, int size)
{
  int nwritten = 0;
  int nread = 0;


  if(to     == NULL ||
       from == NULL ||
    channel == NULL ||
            *to < 0 ||
          *from < 0)
    return;

  nwritten = dtp_write_buffered_data_to_fd(*to, channel->buffer, size);
  if(nwritten < 0)
    goto error;

  nread = dtp_fill_buffer_from_fd(channel->buffer, *from);

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

static void dtp_send_data(data_channel_t *channel)
{
  if(!channel || !channel->buffer)
    return;

  dtp_copy_data(&channel->remote_fd, &channel->local_fd, channel, 1500);
}

static void dtp_receive_data(data_channel_t *channel)
{
  if(!channel || !channel->buffer)
    return;

  dtp_copy_data(&channel->local_fd, &channel->remote_fd, channel, 512);
}

static void dtp_try_accept(data_channel_t *channel)
{
  (void *)channel;
}

void dtp_poll(void)
{
  data_channel_t *dtp = NULL;

  while(dtp_list != NULL)
  {
    dtp = dtp_list;
    dtp_list = dtp->next;

    switch(dtp->state)
    {
      case DTP_PENDING:
        dtp->iface->accept(dtp);
        break;
      case DTP_XMIT:
        dtp->iface->send(dtp);
        break;
      case DTP_RECV:
        dtp->iface->recv(dtp);
        break;
    }

    if(dtp->state == DTP_FREE)
      dtp->iface->free(dtp);
    else
    {
      dtp->next = next_dtp_list;
      next_dtp_list = dtp;
    }
  }

  dtp_list = next_dtp_list;
  next_dtp_list = NULL;
}

data_channel_t *new_data_channel(u32 ip, u16 port)
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

    /* 192K is 128 1500-byte packets, and also 375 512-byte blocks */
    result->buffer = new_buffer(192000);
    if(result->buffer == NULL)
      goto error;

    result->next = next_dtp_list;
    next_dtp_list = result;
  }
  return result;

  error:
  if(result)
    base.free(result);

  return NULL;
}

void free_data_channel(data_channel_t *channel)
{
  if(channel)
  {
    channel->next = NULL;

    if(channel->listen_fd >= 0)
    {
      socketclose(channel->listen_fd);
      channel->listen_fd = -1;
    }
    if(channel->remote_fd >= 0)
    {
      socketclose(channel->remote_fd);
      channel->remote_fd = -1;
    }
    if(channel->local_fd)
    {
      close(channel->local_fd);
      channel->local_fd = -1;
    }
    if(channel->buffer)
    {
      free_buffer(channel->buffer);
      channel->buffer = NULL;
    }
    channel->port = 0;
    channel->ip = 0;
    free(channel);
  }
}

data_interface_t base = {
  new_data_channel,
  free_data_channel,
  dtp_try_accept,
  NULL,
  dtp_send_data,
  dtp_receive_data
};
