#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include "commands.h"
#include "ftp.h"
#include "network.h"

void do_passive(client_t *client, char *args)
{
  if(client->data != NULL) {
    client->data->iface->free(client->data);
    client->data = NULL;
  }

  client->data = passive.new(
    network_get_host_ip(),
    network_get_ephermal_port());

  client->data->client = client;
  char *msg = NULL;

  asprintf(&msg, "Entering passive mode (%d,%d,%d,%d,%d,%d)",
    (client->data->ip & 0xff000000) >> 24,
    (client->data->ip & 0x00ff0000) >> 16,
    (client->data->ip & 0x0000ff00) >> 8,
    (client->data->ip & 0x000000ff),
    (client->data->port & 0xff00) >> 8,
    (client->data->port & 0x00ff));

  if(msg != NULL)
  {
    ftp_response(227, client, msg);
    free(msg);
  }
}
