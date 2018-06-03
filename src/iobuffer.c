#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iobuffer.h"

io_buffer_t *new_buffer(int size)
{
  io_buffer_t *result;

  result = (io_buffer_t *)malloc(sizeof(io_buffer_t));
  if(result == NULL)
    goto error;

  memset(result, 0, sizeof(io_buffer_t));

  result->buffer = (uint8_t *)malloc(size);
  result->line = (uint8_t *)malloc(size);
  if(result->buffer == NULL || result->line == NULL)
    goto error;

  memset(result->buffer, 0, size);
  memset(result->line, 0, size);

  result->size = size;
  return result;

  error:
    free_buffer(result);
    return NULL;
}

void free_buffer(io_buffer_t *buffer)
{
  if(buffer) {
    if(buffer->buffer) {
      free(buffer->buffer);
      buffer->buffer = NULL;
    }
    if(buffer->line) {
      free(buffer->line);
      buffer->line = NULL;
    }

    free(buffer);
  }
}

int iobuffer_remaining(io_buffer_t *buffer)
{
  return buffer->size - 1 - buffer->head;
}

uint8_t *iobuffer_head(io_buffer_t *buffer)
{
  if(buffer == NULL || buffer->buffer == NULL || buffer->head > buffer->size)
    return NULL;

  return buffer->buffer + buffer->head;
}

uint8_t *iobuffer_next_line(io_buffer_t *buffer)
{
  int i;
  int eol = -1;
  int start = -1;
  int bytesToCopy = 0;

  for(i = 0; i < buffer->head; i++)
  {
    if( buffer->buffer[i] == '\n' )
    {
      eol = i;
      break;
    }
  }


  if(eol < 0)
    return NULL;
  start = eol+1;

  /* copy string up to (but not including) EOL characters */
  bytesToCopy = (eol > 0 && buffer->buffer[eol-1] == '\r') ? (eol-1) : eol;
  memcpy(buffer->line, buffer->buffer, bytesToCopy);
  buffer->line[bytesToCopy] = '\0';
  /* move memory forward, update the head, and zero out the empty part of the
   * buffer */
  memmove(buffer->buffer, buffer->buffer+start, (buffer->size - start));
  buffer->head -= start;
  memset(buffer->buffer+buffer->head, 0, (buffer->size - buffer->head));

  return buffer->line;
}
