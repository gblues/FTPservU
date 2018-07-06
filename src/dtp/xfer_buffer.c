#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "xfer.h"
#include "iobuffer.h"

typedef struct xfer_buffer xfer_buffer_t;

struct xfer_buffer {
  char *buffer;
  int head;
};

xfer_t *new_xfer_buffer(char *buffer)
{
  xfer_t *result = (xfer_t *)malloc(sizeof(xfer_t));
  if(result == NULL)
    goto error;

  memset(result, 0, sizeof(xfer_t));
  xfer_buffer_t *data = (xfer_buffer_t *)malloc(sizeof(xfer_buffer_t));
  if(data == NULL)
    goto error;

  data->buffer = buffer;
  data->head = 0;
  result->data = data;
  result->iface = &xfer_buffer;

  return result;

  error:
    if(result != NULL) {
      free(result);
    }
    return NULL;
}

void free_xfer_buffer(xfer_t *xfer) {
  xfer_buffer_t *data = (xfer_buffer_t *)xfer->data;
  if(data != NULL) {
    free(data);
    xfer->data = NULL;
  }
  free(xfer);
}

static void xfer_buffer_fill(io_buffer_t *buffer, void *src_data)
{
  if(buffer == NULL || src_data == NULL) {
    printf("[dtp]: xfer_buffer_fill: null parameters\n");
    return;
  }

  xfer_buffer_t *src = (xfer_buffer_t *)src_data;

  int remaining_to_copy = strlen(src->buffer+src->head);
  int space_in_buffer = iobuffer_remaining(buffer);

  if(remaining_to_copy == 0)
    return;

  int toCopy = (remaining_to_copy < space_in_buffer) ? remaining_to_copy :
        space_in_buffer;

  if(toCopy == 0) {
    printf("[dtp]: xfer_buffer_fill: no room in buffer.\n");
  }

  memcpy(iobuffer_head(buffer), src->buffer+src->head, toCopy);
  buffer->head += toCopy;
  src->head += toCopy;
}

xfer_interface_t xfer_buffer = {
  xfer_buffer_fill,
  NULL,
  free_xfer_buffer
};
