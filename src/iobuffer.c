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

int iobuffer_append(io_buffer_t *buffer, void *data, int len)
{
  if(data == NULL)
  {
    printf("[iobuffer]: cannot append NULL data\n");
    return -1;
  }

  if(buffer->head + len > buffer->size)
  {
    printf("[iobuffer]: append would overflow buffer\n");
    return -1;
  }

  memcpy(buffer->buffer+buffer->head, data, len);
  buffer->head += len;

  return 0;
}

static int find_eol(uint8_t *data, int limit)
{
  for(int i = 0; i < limit; i++)
    if(data[i] == '\n')
      return i;

  return -1;
}

static int calculate_bytes_to_copy(int eol_offset, bool has_cr, bool preserve_newlines)
{
  int result = 0;
  if(preserve_newlines)
  {
    result = eol_offset+1;
  } else {
    result = (has_cr) ? eol_offset-1 : eol_offset;
  }

  return result;
}

static uint8_t *__iobuffer_next_line(io_buffer_t *buffer, bool preserve_newline)
{
  int eol = find_eol(buffer->buffer, buffer->head);

  if(eol < 0)
    return NULL;

  int bytesToCopy = calculate_bytes_to_copy(eol, (buffer->buffer[eol-1] == '\r'), preserve_newline);
  int newBufferStart = eol+1;

  memcpy(buffer->line, buffer->buffer, bytesToCopy);
  buffer->line[bytesToCopy] = '\0';

  memmove(buffer->buffer, buffer->buffer+newBufferStart, (buffer->size - newBufferStart));
  buffer->head -= newBufferStart;
  memset(buffer->buffer+buffer->head, 0, (buffer->size - buffer->head));

  return buffer->line;
}

/*
 * Read the next line out of the buffer. This version preserves EOL
 * characters.
 */
uint8_t *iobuffer_next_line_eol(io_buffer_t *buffer)
{
  return __iobuffer_next_line(buffer, true);
}

/*
 * Read the next line out of the buffer. This version removes the EOL
 * character(s).
 */
uint8_t *iobuffer_next_line(io_buffer_t *buffer)
{
  return __iobuffer_next_line(buffer, false);
}
