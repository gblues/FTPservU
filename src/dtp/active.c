#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "dtp.h"
#include "iobuffer.h"

static void free_active_channel(data_channel_t *active);
static data_channel_t *new_active_channel(u32 ip, u16 port);

static data_channel_t *new_active_channel(u32 ip, u16 port)
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

    result->listen_fd = -1;
    /* 192K is 128 1500-byte packets, and also 375 512-byte blocks */
    result->buffer = new_buffer(192000);
    if(result->buffer == NULL)
      goto error;
  }
  return result;

  error:
  if(result)
    free_active_channel(result);

  return NULL;
}

static void free_active_channel(data_channel_t *active)
{
  if(active)
  {
    active->next = NULL;

    if(active->local_fd >= 0)
    {
      socketclose(active->local_fd);
      active->local_fd = -1;
    }
    if(active->remote_fd)
    {
      close(active->remote_fd);
      active->remote_fd = -1;
    }
    if(active->buffer)
    {
      free_buffer(active->buffer);
      active->buffer = NULL;
    }
    active->port = 0;
    active->ip = 0;
    free(active);
  }
}

data_interface_t active = {
  new_active_channel,
  free_active_channel
};
