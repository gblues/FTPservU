#include <string.h>
#include <malloc.h>

#include "hbl.h"
#include "fs/fs_utils.h"
#include "fs/sd_fat_devoptab.h"
#include "system/dynamic.h"
#include "system/exception_handler.h"
#include "wiiu/types.h"
#include "wiiu/os/screen.h"
#include "wiiu/vpad.h"

#include "network.h"
#include "ftp.h"
#include "input.h"
#include "console.h"

#define TCP_PORT              21

void mount_filesystems()
{
}

void unmount_filesystems()
{
}

void remount_filesystems()
{
  unmount_filesystems();
  mount_filesystems();
}

void screen_init()
{
  int bufsize = 0;

  OSScreenInit();
  bufsize = OSScreenGetBufferSizeEx(0);
  OSScreenSetBufferEx(0, (void *)0xf4000000);
  OSScreenSetBufferEx(1, (void *)(0xf4000000 + bufsize));

  OSScreenEnableEx(0, 1);
  OSScreenEnableEx(1, 1);

  OSScreenFlipBuffersEx(0);
  OSScreenFlipBuffersEx(1);
}

void ftpserver_main_loop(void)
{
  int serverSocket = 0;
  int cmd = 0;
  bool network_down = false;

  serverSocket = network_create_serversocket(TCP_PORT);

  while( serverSocket >= 0 && !network_down )
  {
    network_down = ftp_network_handler(serverSocket);
    cmd = input_handler();
    switch(cmd)
    {
      case VPAD_CMD_EXIT:
        network_down = true;
        break;
      case VPAD_CMD_REMOUNT:
        remount_filesystems();
        break;
    }
  }
}

int main(int argc, char **argv)
{
  (int) argc;
  (char **)argv;

  mount_filesystems();
  console_init();
  VPADInit();
  screen_init();

  ftpserver_main_loop();

  ftp_deinit();
  console_deinit();
  unmount_filesystems();
}
