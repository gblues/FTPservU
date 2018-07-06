#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "xfer.h"
#include "iobuffer.h"

typedef struct xfer_fd xfer_fd_t;

struct xfer_fd {
  int fd;
};

xfer_t *new_xfer_fd(int fd)
{
  xfer_t *result = (xfer_t *)malloc(sizeof(xfer_t));
  if(result == NULL)
    goto error;

  memset(result, 0, sizeof(xfer_t));
  xfer_fd_t *data = (xfer_fd_t *)malloc(sizeof(xfer_fd_t));
  if(data == NULL)
    goto error;

  data->fd = fd;
  result->data = data;
  result->iface = &xfer_fd;

  return result;

  error:
    if(result != NULL) {
      free(result);
    }
    return NULL;
}

void free_xfer_fd(xfer_t *xfer) {
  xfer_fd_t *data = (xfer_fd_t *)xfer->data;
  if(data != NULL) {
    free(data);
    xfer->data = NULL;
  }
  free(xfer);
}

static void xfer_fd_fill(io_buffer_t *buffer, void *src_data)
{
  if(buffer == NULL || src_data == NULL) {
    printf("[dtp]: xfer_fd_fill: null parameters\n");
    return;
  }

  xfer_fd_t *src = (xfer_fd_t *)src_data;

  if(src->fd < 0 || iobuffer_remaining(buffer) == 0)
    return;

  int nread = read(src->fd, iobuffer_head(buffer), iobuffer_remaining(buffer));
  if(nread <= 0) {
    close(src->fd);
    src->fd = -1;
    return;
  }
  buffer->head += nread;
}

static int xfer_fd_write(io_buffer_t *buffer, void *output, int packetSize)
{
  xfer_fd_t *data = (xfer_fd_t *)output;

  if(data == NULL || data->fd < 0)
    return -1;

  int toSend = (packetSize > buffer->head) ? buffer->head : packetSize;
  if(toSend == 0)
    return 0;

  int nwritten = write(data->fd, buffer->buffer, toSend);

  if(nwritten > 0) {
    memmove(buffer->buffer, buffer->buffer+nwritten, buffer->size - nwritten);
    buffer->head -= nwritten;
  }

  return nwritten;
}

xfer_interface_t xfer_fd = {
  xfer_fd_fill,
  xfer_fd_write,
  free_xfer_fd
};
