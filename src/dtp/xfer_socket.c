#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "xfer.h"
#include "iobuffer.h"

typedef struct xfer_socket xfer_socket_t;

struct xfer_socket {
  int socket;
};

xfer_t *new_xfer_socket(int socket)
{
  if(socket < 0)
    goto error;

  xfer_t *result = (xfer_t *)malloc(sizeof(xfer_t));
  if(result == NULL)
    goto error;

  memset(result, 0, sizeof(xfer_t));
  xfer_socket_t *data = (xfer_socket_t *)malloc(sizeof(xfer_socket_t));
  if(data == NULL)
    goto error;

  data->socket = socket;

  result->data = data;
  result->iface = &xfer_buffer;

  return result;

  error:
    if(result != NULL) {
      free(result);
    }
    return NULL;
}

void free_xfer_socket(xfer_t *xfer) {
  xfer_socket_t *data = (xfer_socket_t *)xfer->data;
  if(data != NULL) {
    free(data);
    xfer->data = NULL;
  }
  free(xfer);
}

static void xfer_socket_fill(io_buffer_t *buffer, void *src_data)
{
  if(buffer == NULL || src_data == NULL) {
    printf("[dtp]: xfer_socket_fill: null parameters\n");
    return;
  }

  xfer_socket_t *src = (xfer_socket_t *)src_data;

  if(src->socket < 0 || iobuffer_remaining(buffer) == 0)
    return;

  int nread = recv(src->socket, iobuffer_head(buffer), iobuffer_remaining(buffer), 0);
  if(nread <= 0) {
    int errno = socketlasterr();

    if(errno != EAGAIN) {
      socketclose(src->socket);
      src->socket = -1;
    }

    return;
  }

  buffer->head += nread;
}

int xfer_socket_write(io_buffer_t *buffer, void *output, int packetSize)
{
  xfer_socket_t *data = (xfer_socket_t *)output;

  if(data == NULL)
    return -1;

  int toSend = (packetSize > buffer->head) ? buffer->head : packetSize;

  if(toSend == 0)
    return 0;

  int nwritten = send(data->socket, buffer->buffer, toSend, 0);

  if(nwritten < 0) {
    int errno = socketlasterr();

    if(errno != EAGAIN)
      return -errno;

    return 0;
  }

  memmove(buffer->buffer, buffer->buffer+nwritten, buffer->size - nwritten);
  buffer->head -= nwritten;

  return nwritten;
}

xfer_interface_t xfer_socket = {
  xfer_socket_fill,
  xfer_socket_write
};
