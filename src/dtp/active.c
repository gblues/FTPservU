#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "dtp.h"
#include "iobuffer.h"

static data_channel_t *new_active_channel(u32 ip, u16 port)
{
  return base.new(ip, port);
}

static void free_active_channel(data_channel_t *channel)
{
  base.free(channel);
}

static void active_try_connect(data_channel_t *channel)
{
}

static void active_send_data(data_channel_t *channel)
{
  base.send(channel);
}

static void active_receive_data(data_channel_t *channel)
{
  base.recv(channel);
}

static bool active_is_connected(data_channel_t *channel)
{
  if(channel->state = DTP_ESTABLISHED && channel->remote_fd >= 0)
    return true;

  /* TODO: attempt to connect to channel->ip on channel->port) */
  return false;
}

data_interface_t active = {
  new_active_channel,
  free_active_channel,
  active_try_connect,
  active_is_connected,
  active_send_data,
  active_receive_data
};
