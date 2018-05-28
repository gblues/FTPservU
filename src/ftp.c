#include "wiiu/types.h"
#include "malloc.h"

#include "network.h"
#include "ftp.h"

#define MAX_CLIENTS 25
#define FTP_BUFFER 1024

client_t *clients[MAX_CLIENTS];

client_t *new_client(void);
void free_client(client_t *client);


static void handle_data_events(client_t *client)
{
}

static void ftp_client_cleanup(client_t *client)
{
}

static void handle_control_events(client_t *client)
{
  int nread;

  if(client->data_connection)
    return;

  nread = network_read_buffer(client->fd, client->input_buffer);

  /* client has no more bytes to read */
  if(nread == -EAGAIN)
    return;
  /* something went wrong with the connection */
  if(nread <= 0)
  {
    ftp_client_cleanup(client);
    return;
  }
}

static void handle_client(client_t *client)
{
  if(client == NULL)
    return;

  if(client->data_connection)
    handle_data_events(client);
  else
    handle_control_events(client);
}

static int find_open_client_slot(void)
{
  for( int i = 0; i < MAX_CLIENTS; i++ )
    if(clients[i] == NULL)
      return i;

  return -1;
}

/**
 * Will get invoked for each successfully established connection
 * via network_accept_poll
 */
void ftp_accept_handler(int fd, struct sockaddr_in *sockaddr, socklen_t size)
{
  // console_printf("Accepted connection from %s!\n", ...
  int slot = find_open_client_slot();
  if(slot < 0)
  {
    // Maximum client list reached
    network_close(fd);
    return;
  }

  client_t *client = new_client();
  if(!client)
  {
    network_close(fd);
    return;
  }

  client->fd = fd;
  clients[slot] = client;
}

void ftp_deinit(void)
{
  for(int i = 0; i < MAX_CLIENTS; i++)
  {
    if(clients[i] != NULL)
    {
      free_client(clients[i]);
      clients[i] = NULL;
    }
  }
}

int ftp_network_handler(int socket)
{
  int i;

  if( network_accept_poll(socket, ftp_accept_handler) < 0 )
    return -1;

  for(i = 0; i < MAX_CLIENTS; i++)
    handle_client(clients[i]);

  return 0;
}

client_t *new_client(void)
{
  client_t *client = malloc(sizeof(client_t));

  if(client == NULL)
    goto error;

  client->input_buffer = new_buffer(FTP_BUFFER);

  if(client->input_buffer == NULL)
    goto error;

  return client;

  error:
    free_client(client);
    return NULL;
}

void free_client(client_t *client)
{
  if(client) {
    if(client->input_buffer) {
      free_buffer(client->input_buffer);
     client->input_buffer = NULL;
    }
    free(client);
  }
}
