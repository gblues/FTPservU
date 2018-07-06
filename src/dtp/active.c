#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "dtp.h"
#include "iobuffer.h"
#include "network.h"

static data_channel_t *new_active_channel(u32 ip, u16 port)
{
  data_channel_t *result = base.new(ip, port);
  result->iface = &active;

  return result;
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
  if(GET_STATE(channel) == DTP_ESTABLISHED && channel->remote != NULL)
    return true;

  channel->remote = new_xfer_socket(network_connect(channel->ip, channel->port));
  if(channel->remote != NULL)
  {
    SET_STATE(channel, DTP_ESTABLISHED);
    return true;
  }

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
