#ifndef _IOBUFFER_H
#define _IOBUFFER_H

#include "wiiu/types.h"

struct io_buffer {
  uint8_t *buffer;
  int size;
  int head;
};

typedef struct io_buffer io_buffer_t;

io_buffer_t *new_buffer(int size);
void free_buffer(io_buffer_t *buffer);


#endif /* _IOBUFFER_H */
