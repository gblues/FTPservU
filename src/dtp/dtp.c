#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys/socket.h"

#include "console.h"
#include "iobuffer.h"
#include "ftp.h"
#include "dtp.h"
#include "network.h"
#include "wiiu/ac.h"

static data_channel_t *dtp_list = NULL;
static data_channel_t *next_dtp_list = NULL;

static void dtp_shutdown(data_channel_t *channel, int state)
{
  if( channel->local != NULL) {
    channel->local->iface->free(channel->local);
    channel->local = NULL;
  }

  if(channel->remote != NULL) {
    channel->remote->iface->free(channel->remote);
    channel->remote = NULL;
  }

  SET_STATE(channel, state);
}

static int dtp_fill_buffer(io_buffer_t *buffer, xfer_t *src)
{
  if(buffer == NULL || src == NULL)
    return -1;

  src->iface->fill_buffer(buffer, src->data);
  return 0;
}

/*
 * Copy up to size bytes from the input source ('from') to the output ('to')
 */
static void dtp_copy_data(xfer_t **to, xfer_t **from, data_channel_t *channel, int size)
{
  if(    to == NULL ||
        *to == NULL ||
       from == NULL ||
      *from == NULL ||
    channel == NULL ||
       size <= 0)
    return;

  dtp_fill_buffer(channel->buffer, *from);

  if(channel->buffer->head == 0) {
    dtp_shutdown(channel, DTP_CLOSED);
    return;
  }

  int nwritten = (*to)->iface->write_buffer(channel->buffer, (*to)->data, size);

  if(nwritten < 0)
    goto error;

  return;

  error:
    dtp_shutdown(channel, DTP_ERROR);
}

static void dtp_send_data(data_channel_t *channel)
{
  if(!channel || !channel->buffer)
    return;

  dtp_copy_data(&channel->remote, &channel->local, channel, 1500);
}

static void dtp_receive_data(data_channel_t *channel)
{
  if(!channel || !channel->buffer)
    return;

//  dtp_copy_data(&channel->local.fd, &channel->remote, channel, 512);
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
  if( GET_STATE(channel) != DTP_ESTABLISHED ) {
    printf("[dtp]: check_ready: NOT READY, connection is not established.\n");
    return;
  }

  if(channel->client == NULL) {
    printf("[dtp]: check_ready: NOT READY, null client\n");
    return;
  }

  if( (channel->state & (DTP_RECV|DTP_XMIT)) )
    ftp_response(150, channel->client, "Beginning data transfer.");
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

  dtp_list = next_dtp_list;
  next_dtp_list = NULL;
}

/*
 * Used to initialize a channel pointer if it's not already set.
 */
void dtp_channel_init(client_t *client, u32 ip, u16 port)
{
  if(client == NULL)
    return;

  if(client->data == NULL) {
    client->data = active.new(ip, port);
    client->data->client = client;
  }
}

data_channel_t *new_data_channel(u32 ip, u16 port)
{
  data_channel_t *result = (data_channel_t *)malloc(sizeof(data_channel_t));
  if(result != NULL)
  {
    result->ip = ip;
    result->port = port;
    result->state = DTP_PENDING;
    result->local = NULL;
    result->remote = NULL;

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

    if(channel->local != NULL) {
      channel->local->iface->free(channel->local);
      channel->local = NULL;
    }
    if(channel->remote != NULL) {
      channel->remote->iface->free(channel->remote);
      channel->remote = NULL;
    }

    if(channel->listen_fd >= 0)
    {
      socketclose(channel->listen_fd);
      channel->listen_fd = -1;
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

/*
 * Queue a null-terminated buffer to be sent to the client's data
 * connection. Returns 0 on success, -1 on failure.
 */
int dtp_send_buffer(data_channel_t *channel, char *buffer)
{
  if(channel->local == NULL) {
    channel->local = new_xfer_buffer(buffer);
    channel->state |= DTP_XMIT;

    return (channel->local == NULL) ? -1 : 0;
  }

  return -1;
}

int dtp_send_file(data_channel_t *channel, int fd)
{
  if(channel->local == NULL) {
    channel->local = new_xfer_fd(fd);
    channel->state |= DTP_XMIT;
    return (channel->local == NULL) ? -1 : 0;
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
