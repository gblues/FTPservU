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
    SET_STATE(channel, DTP_CLOSED);
  }

  return;

  error:
  SET_STATE(channel, DTP_ERROR);
}

static void dtp_send_data(data_channel_t *channel)
{
  if(!channel || !channel->buffer)
    return;

  dtp_copy_data(&channel->remote_fd, &channel->local.fd, channel, 1500);
}

static void dtp_receive_data(data_channel_t *channel)
{
  if(!channel || !channel->buffer)
    return;

  dtp_copy_data(&channel->local.fd, &channel->remote_fd, channel, 512);
}

static void dtp_try_accept(data_channel_t *channel)
{
  (void *)channel;
}

/*
 * Determine if we're ready to start the data transfer.
 * I really hate how stateful FTP is.
 */
static void dtp_check_ready(data_channel_t *channel)
{
  if( GET_STATE(channel) != DTP_ESTABLISHED )
    return;

  if( (channel->state & DTP_RECV) )
    SET_STATE(channel, DTP_RECVING);
  if( (channel->state & DTP_XMIT) )
    SET_STATE(channel, DTP_SENDING);
}

void dtp_poll(void)
{
  data_channel_t *dtp = NULL;
  int count = 0;

  while(dtp_list != NULL)
  {
    count++;
    dtp = dtp_list;
    dtp_list = dtp->next;
    dtp->next = NULL;
    printf("[dtp]: processing data connection %d\n", count);

    switch(GET_STATE(dtp))
    {
      case DTP_PENDING:
        printf("Data connection is PENDING, trying accept\n");
        dtp->iface->accept(dtp);
        break;
      case DTP_ESTABLISHED:
        printf("Data connection is ESTABLISHED, checking if we're ready to start data transfer\n");
        dtp_check_ready(dtp);
        break;
      case DTP_SENDING:
        printf("Data connection is SENDING, sending a packet\n");
        dtp->iface->send(dtp);
        break;
      case DTP_RECVING:
        printf("Data connection is RECVING, receiving a packet\n");
        dtp->iface->recv(dtp);
        break;
      default:
        printf("Data connection is %d, no-op\n", GET_STATE(dtp));
        break;
    }

    if(GET_STATE(dtp) == DTP_FREE)
    {
      printf("Data connection is ready to be freed. Be free!\n");
      dtp->iface->free(dtp);
    } else {
      dtp->next = next_dtp_list;
      next_dtp_list = dtp;
    }
  }

  printf("[dtp]: finished polling %d data channels\n", count);
  dtp_list = next_dtp_list;
  next_dtp_list = NULL;
}

/*
 * Used to initialize a channel pointer if it's not already set.
 */
void dtp_channel_init(data_channel_t **pchannel, u32 ip, u16 port)
{
  if(pchannel == NULL)
    return;

  if(*pchannel == NULL)
    *pchannel = active.new(ip, port);
}

data_channel_t *new_data_channel(u32 ip, u16 port)
{
  data_channel_t *result = (data_channel_t *)malloc(sizeof(data_channel_t));
  if(result != NULL)
  {
    result->ip = ip;
    result->port = port;
    result->state = DTP_PENDING;
    result->local.fd = -1;
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

    if(channel->state & DTP_LOCAL_BUF)
    {
      if(channel->local.buffer) {
        free(channel->local.buffer);
        channel->local.buffer = NULL;
      }
    } else {
      if(channel->local.fd >= 0) {
        close(channel->local.fd);
        channel->local.fd = -1;
      }
    }
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

int dtp_send_buffer(data_channel_t *channel, char *buffer)
{
  if(channel->local.fd == -1)
  {
    channel->local.buffer = buffer;
    channel->state |= (DTP_LOCAL_BUF | DTP_XMIT);
    return 0;
  }

  return -1;
}

int dtp_send_file(data_channel_t *channel, int fd)
{
  if(channel->local.fd == -1)
  {
    channel->local.fd = fd;
    channel->state &= ~DTP_LOCAL_BUF;
    channel->state |= DTP_XMIT;
    return 0;
  }

  return -1;
}

data_interface_t base = {
  new_data_channel,
  free_data_channel,
  dtp_try_accept,
  NULL,
  dtp_send_data,
  dtp_receive_data
};
