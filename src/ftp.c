#define _GNU_SOURCE
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "wiiu/types.h"
#include "malloc.h"

#include "logging.h"
#include "console.h"
#include "commands.h"
#include "network.h"
#include "ftp.h"
#include "dtp/passive.h"

#define MAX_CLIENTS 25
#define FTP_BUFFER 1024

client_t *clients[MAX_CLIENTS];

client_t *new_client(void);
void free_client(client_t *client);

static void handle_data_events(client_t *client)
{
  if(!client || !(client->state & STATE_DATA))
    return;

  if(client->data == NULL)
  {
    client->state &= ~STATE_DATA;
    return;
  }

  if(client->data->state >= DTP_CLOSED)
  {
    switch(client->data->state)
    {
      case DTP_CLOSED:
        ftp_response(226, client, "Transfer complete.");
        break;
      default:
        ftp_response(420, client, "Transfer failed due to a network error.");
        break;
    }

    client->data->state = DTP_FREE;
    client->data = NULL;
    client->state &= ~STATE_DATA;
  }
}

static void ftp_client_cleanup(client_t *client)
{
  for(int i = 0; i < MAX_CLIENTS; i++)
    if(clients[i] == client)
      clients[i] = NULL;

  free_client(client);
}

char *get_mnemonic(uint8_t **buffer)
{
  char *mnemonic = NULL;

  while(isspace(**buffer))
    (*buffer)++;

  if(**buffer == '\0')
    return NULL;

  mnemonic = *buffer;

  while(**buffer != '\0' && !isspace(**buffer))
  {
    if(!isalpha(**buffer))
      return NULL;
    (*buffer)++;
  }

  /* if there's something after the mnemonic, fast-forward the buffer to the first non-whitespace
   * character. */
  if(**buffer != '\0')
  {
    **buffer = '\0';
    (*buffer)++;
    while(isspace(**buffer))
      (*buffer)++;
  }

  return mnemonic;
}

/*
 * The pointer passed into this function can be freely modified, because it's anchored in
 * the network buffer and will get cleaned up.
 */
static void ftp_process_command(client_t *client, uint8_t *command)
{
  int i;
  char *mnemonic = NULL;
  char *parameters = NULL;

  console_printf("Client sent: %s", command);
  /* get_mnemonic automatically fast-forwards command */
  mnemonic = get_mnemonic(&command);
  parameters = command;

  if(mnemonic != NULL)
  {
    for(int i = 0; mnemonic[i] != '\0'; i++)
      mnemonic[i] = toupper(mnemonic[i]);

    printf("mnemonic: %s\n", mnemonic);
    printf("parameters: %s\n", ((parameters == NULL) ? "(null)" : parameters));
    command_invoke(client, mnemonic, parameters);
  }
}

static void handle_control_events(client_t *client)
{
  int nread;

  if(client->state & STATE_DATA)
    return;

  nread = network_read_buffer(client->fd, client->input_buffer);

  /* client has no more bytes to read */
  if(nread == -EAGAIN)
    return;

  /* something went wrong with the connection */
  if(nread <= 0)
  {
    if(nread == 0)
      log_printf("[ftp]: EOF when reading from client.\n");
    else
      log_printf("[ftp]: Error when reading: %d\n", nread);

    ftp_client_cleanup(client);
    return;
  }
  log_printf("[ftp]: read %d bytes\n", nread);

  uint8_t *line;

  while( (line = iobuffer_next_line(client->input_buffer)) != NULL )
    ftp_process_command(client, line);
}

static void handle_output(client_t *client)
{
  if(client == NULL)
  {
    printf("[ftp]: handle_output: null client\n");
    return;
  }

  /* nothing to do */
  if(client->output_buffer->head == 0)
    return;

  char *line = NULL;

  while( (line = iobuffer_next_line_eol(client->output_buffer)) != NULL )
    network_writeln(client->fd, line);
}

static void handle_client(client_t *client)
{
  if(client == NULL)
    return;

  if(client->state & STATE_DATA)
    handle_data_events(client);

  if(!(client->state & STATE_DATA))
    handle_control_events(client);

  handle_output(client);
  if(client->state & STATE_DISCONN)
    ftp_client_cleanup(client);
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
void ftp_accept_handler(int fd, struct sockaddr_in *sockaddr, socklen_t size, void *unused)
{
  if(sockaddr == NULL)
  {
    printf("[ftp]: accept handler received null sockaddr\n");
    return;
  }

  console_printf("Accepted connection from %d.%d.%d.%d:%d!",
    (sockaddr->sin_addr.s_addr & 0xff000000) >> 24,
    (sockaddr->sin_addr.s_addr & 0x00ff0000) >> 16,
    (sockaddr->sin_addr.s_addr & 0x0000ff00) >> 8,
    (sockaddr->sin_addr.s_addr & 0x000000ff),
    sockaddr->sin_port );
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

  ftp_response(220, client, "FTPservU, here to serve you!");

  client->fd = fd;
  client->ip = sockaddr->sin_addr.s_addr;
  client->port = sockaddr->sin_port;
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

  if( network_accept_poll(socket, ftp_accept_handler, NULL) < 0 )
    return -1;

  passive_poll();

  for(i = 0; i < MAX_CLIENTS; i++)
    handle_client(clients[i]);

  return 0;
}

client_t *new_client(void)
{
  client_t *client = malloc(sizeof(client_t));

  if(client == NULL)
    goto error;

  memset(client, 0, sizeof(client_t));

  client->input_buffer = new_buffer(FTP_BUFFER);
  client->output_buffer = new_buffer(FTP_BUFFER);
  client->cwd[0] = '/';

  if(client->input_buffer == NULL || client->output_buffer == NULL)
    goto error;

  return client;

  error:
    free_client(client);
    return NULL;
}

void free_client(client_t *client)
{
  if(client) {
    client->state = STATE_NONE;

    if(client->fd >= 0) {
      socketclose(client->fd);
      client->fd = -1;
    }

    if(client->input_buffer) {
      free_buffer(client->input_buffer);
      client->input_buffer = NULL;
    }

    if(client->output_buffer) {
      free_buffer(client->output_buffer);
      client->output_buffer = NULL;
    }

    free(client);
  }
}

void ftp_response(int code, client_t *client, const char *msg)
{
  char *response = NULL;
  asprintf(&response, "%d %s\r\n", code, msg);
  if(response != NULL) {
    iobuffer_append(client->output_buffer, response, strlen(response));
    free(response);
  }
}
