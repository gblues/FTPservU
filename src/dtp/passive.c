#include <unistd.h>
#include <stdlib.h>
#include "sys/socket.h"

#include "iobuffer.h"
#include "passive.h"
#include "network.h"
#include "wiiu/ac.h"

static int pasv_port = 1024;

/*
 * This list is read-only.
 */
static passive_t *pasv_list = NULL;
/*
 * This list is write-only. Do not iterate over this list.
 */
static passive_t *next_pasv_list = NULL;

static int get_pasv_port(void)
{
  int result = pasv_port;

  pasv_port++;
  if(pasv_port > 65535)
    pasv_port = 1024;

  return result;
}

static void pasv_try_accept(passive_t *pasv)
{
}

static void pasv_send_data(passive_t *pasv)
{
}

static void pasv_recv_data(passive_t *pasv)
{
}

void passive_poll(void)
{
  passive_t *pasv = NULL;

  while(pasv_list != NULL)
  {
    pasv = pasv_list;
    pasv_list = pasv->next;

    switch(pasv->state)
    {
      case PASV_NONE:
        pasv_try_accept(pasv);
        break;
      case PASV_CTS:
        pasv_send_data(pasv);
        break;
      case PASV_RTS:
        pasv_recv_data(pasv);
        break;
    }

    if(pasv->state == PASV_FREE)
      free_passive(pasv);
    else
    {
      pasv->next = next_pasv_list;
      next_pasv_list = pasv;
    }
  }

  pasv_list = next_pasv_list;
  next_pasv_list = NULL;
}

passive_t *new_passive(void)
{
  passive_t *result = (passive_t *)malloc(sizeof(passive_t));
  if(result != NULL)
  {
    ACGetAssignedAddress(&result->ip);
    result->port = get_pasv_port();
    result->state = PASV_NONE;
    result->client_fd = -1;
    result->file_fd = -1;

    if(result->ip == 0)
      goto error;

    result->listen_fd = network_create_serversocket(result->port);
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
    free_passive(result);

  return NULL;
}

void free_passive(passive_t *passive)
{
  if(passive)
  {
    passive->next = NULL;

    if(passive->listen_fd >= 0)
    {
      socketclose(passive->listen_fd);
      passive->listen_fd = -1;
    }
    if(passive->client_fd >= 0)
    {
      socketclose(passive->client_fd);
      passive->client_fd = -1;
    }
    if(passive->file_fd)
    {
      close(passive->file_fd);
      passive->file_fd = -1;
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
