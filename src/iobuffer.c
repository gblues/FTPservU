#include "iobuffer.h"

io_buffer_t *new_buffer(int size)
{
  io_buffer_t *result;

  result = (io_buffer_t *)malloc(sizeof io_buffer_t);
  if(result == NULL)
    goto error;

  memset(result, 0, sizeof(io_buffer_t);

  result->buffer = (uint8_t *)malloc(size);
  if(result->buffer == NULL)
    goto error;

  memset(result->buffer, 0, size);

  result->size = size;
  return result;

  error:
    if(result) {
      if(result->buffer) {
        free(result->buffer);
        result->buffer = NULL;
      }

      free(result);
    }
    return NULL;
}

void free_buffer(io_buffer_t *buffer)
{
  if(buffer) {
    if(buffer->buffer) {
      free(buffer->buffer);
      buffer->buffer = NULL;
    }

    free(buffer);
  }
}
