#ifndef _XFER_H
#define _XFER_H

#include "ftpservu_types.h"

typedef struct xfer xfer_t;
typedef struct xfer_interface xfer_interface_t;

struct xfer {
  void *data;
  xfer_interface_t *iface;
};

struct xfer_interface {
   void (*fill_buffer)(io_buffer_t *buffer, void *input);
   int (*write_buffer)(io_buffer_t *buffer, void *output, int packetSize);
   void (*free)(xfer_t *xfer);
};

xfer_t *new_xfer_buffer(char *buffer);
void free_xfer_buffer(xfer_t *xfer);

xfer_t *new_xfer_fd(int fd);
void free_xfer_fd(xfer_t *xfer);

xfer_t *new_xfer_socket(int socket);
void free_xfer_socket(xfer_t *xfer);

extern xfer_interface_t xfer_buffer;
extern xfer_interface_t xfer_socket;
extern xfer_interface_t xfer_fd;

#endif /* _XFER_H */
