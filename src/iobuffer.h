#ifndef _IOBUFFER_H
#define _IOBUFFER_H

#include "wiiu/types.h"
#include "ftpservu_types.h"

struct io_buffer {
  uint8_t *buffer;
  uint8_t *line;
  int size;
  int head;
};

io_buffer_t *new_buffer(int size);
void free_buffer(io_buffer_t *buffer);
int iobuffer_remaining(io_buffer_t *buffer);
int iobuffer_append(io_buffer_t *buffer, void *data, int len);
uint8_t *iobuffer_head(io_buffer_t *buffer);
uint8_t *iobuffer_next_line(io_buffer_t *buffer);
uint8_t *iobuffer_next_line_eol(io_buffer_t *buffer);

#endif /* _IOBUFFER_H */
