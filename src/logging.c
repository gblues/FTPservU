#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/iosupport.h>
#include "wiiu/os.h"
#include "wiiu/ac.h"
#include "network.h"
#include "logging.h"
#include "console.h"

#define LOG_UDP_PORT 4405
#define DGRAM_SIZE 576

static int udp_fd = -1;
static volatile int log_lock = 0;
static struct sockaddr_in broadcast;

static ssize_t logging_write(struct _reent *r, void *fd, const char *ptr, size_t len);

static devoptab_t dotab_stdout =
{
  "stdout_net",
  0,
  NULL,
  NULL,
  logging_write,
  NULL,
};

int get_broadcast_address(ACIpAddress *broadcast)
{
  ACIpAddress myIp, mySubnet;
  ACResult result;

  if(broadcast == NULL)
    return -1;

  result = ACGetAssignedAddress(&myIp);
  if(result < 0)
    return -1;

  result = ACGetAssignedSubnet(&mySubnet);
  if(result < 0)
    return -1;

  *broadcast = myIp | (~mySubnet);
  return 0;
}

static int init_broadcast(int port)
{
  ACIpAddress broadcast_ip;
  if(get_broadcast_address(&broadcast_ip) < 0)
    return -1;

  memset(&broadcast, 0, sizeof(broadcast));
  broadcast.sin_family = AF_INET;
  broadcast.sin_port = htons(port);
  broadcast.sin_addr.s_addr = htonl(broadcast_ip);

  return 0;
}

static void init_udp_broadcast(int port)
{
  if(udp_fd >= 0)
    return;

  init_broadcast(port);

  udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if(udp_fd < 0)
  {
    console_printf("[log]: Error when trying to create UDP socket: %d", socketlasterr());

    return;
  }

  struct sockaddr_in connect_addr;
  memset(&connect_addr, 0, sizeof(connect_addr));
  connect_addr.sin_family = AF_INET;
  connect_addr.sin_port = 0;
  connect_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if( bind(udp_fd, (struct sockaddr *)&connect_addr, sizeof(connect_addr)) < 0)
  {
    console_printf("[log]: Error when binding local port: %d", socketlasterr());
    socketclose(udp_fd);
    udp_fd = -1;
    return;
  }

  console_printf("Initialized UDP logging on socket %d", udp_fd);
}

static void deinit_udp_broadcast()
{
  if(udp_fd >= 0)
  {
    socketclose(udp_fd);
    udp_fd = -1;
  }
}

void logging_init(void)
{
  console_printf("Starting up logging service..");
  init_udp_broadcast(LOG_UDP_PORT);
  devoptab_list[STD_OUT] = &dotab_stdout;
  devoptab_list[STD_ERR] = &dotab_stdout;
}

void logging_deinit(void)
{
  fflush(stdout);
  fflush(stderr);

  deinit_udp_broadcast();
}

static ssize_t logging_write(struct _reent *r, void *fd, const char *ptr, size_t len)
{
  if(udp_fd < 0)
    return len;


  while(log_lock)
    OSSleepTicks(((248625000 / 4)) / 1000);

  log_lock = 1;

  int sent;
  int remaining = len;

  while(remaining > 0)
  {
    int block = (remaining < DGRAM_SIZE) ? remaining : DGRAM_SIZE;
    sent = sendto(udp_fd, ptr, block, 0, (struct sockaddr *)&broadcast, sizeof(broadcast));
    if( sent < 0)
      break;

    remaining -= sent;
    ptr       += sent;
  }

  log_lock = 0;

  return len;
}

int log_printf(const char *fmt, ...)
{

  int result;
  va_list va;

  va_start(va, fmt);
  result = vfprintf(stderr, fmt, va);
  va_end(va);

  return 0;
}
