#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <iosuhax.h>
#include <iosuhax_devoptab.h>
#include <fat.h>

#include "hbl.h"
#include "fs/fs_utils.h"
#include "fs/sd_fat_devoptab.h"
#include "system/dynamic.h"
#include "system/exception_handler.h"
#include "wiiu/types.h"
#include "wiiu/os/screen.h"
#include "wiiu/vpad.h"

#include "version.h"
#include "main.h"
#include "network.h"
#include "ftp.h"
#include "input.h"
#include "console.h"

#define TCP_PORT              21

static bool fs_mounted = false;
static int fsa_fd = -1;

void mount_filesystems()
{
  if(fs_mounted)
    return;

  if(iosuhaxMount)
  {
    fatInitDefault();
    fsa_fd = IOSUHAX_FSA_Open();

    if(fsa_fd >= 0)
    {
      mount_fs("slccmpt01", fsa_fd, "/dev/slccmpt01", "/vol/storage_slccmpt01");
      mount_fs("storage_odd_tickets", fsa_fd, "/dev/odd01", "/vol/storage_odd_tickets");
      mount_fs("storage_odd_updates", fsa_fd, "/dev/odd02", "/vol/storage_odd_updates");
      mount_fs("storage_odd_content", fsa_fd, "/dev/odd03", "/vol/storage_odd_content");
      mount_fs("storage_odd_content2", fsa_fd, "/dev/odd04", "/vol/storage_odd_content2");
      mount_fs("storage_slc", fsa_fd, NULL, "/vol/system");
      mount_fs("storage_mlc", fsa_fd, NULL, "/vol/storage_mlc01");
      mount_fs("storage_usb", fsa_fd, NULL, "/vol/storage_usb01");
    }
  } else {
    mount_sd_fat("sd");
  }

  fs_mounted = true;
}

void unmount_filesystems()
{
  if(!fs_mounted)
    return;

  if(iosuhaxMount)
  {
    fatUnmount("sd");
    fatUnmount("usb");
    if(fsa_fd >= 0)
    {
      unmount_fs("slccmpt01");
      unmount_fs("storage_odd_tickets");
      unmount_fs("storage_odd_updates");
      unmount_fs("storage_odd_content");
      unmount_fs("storage_odd_content2");
      unmount_fs("storage_slc");
      unmount_fs("storage_mlc");
      unmount_fs("storage_usb");

      IOSUHAX_FSA_Close(fsa_fd);
      fsa_fd = -1;
    }
  } else {
    unmount_sd_fat("sd");
  }

  fs_mounted = false;
}

hooks_t hooks = {
  mount_filesystems,
  unmount_filesystems
};

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

  if(serverSocket >= 0)
    console_printf("Listening on port %d", TCP_PORT);
  else
  {
    console_printf("Failed to create server socket (%d), terminating...", serverSocket);
  }

  while( serverSocket >= 0 && !network_down )
  {
    network_down = ftp_network_handler(serverSocket);
    if(network_down)
      console_printf("encountered a fatal network error, terminating...");

    cmd = input_handler();
    switch(cmd)
    {
      case VPAD_CMD_EXIT:
        console_printf("Executing shutdown command");
        network_down = true;
        break;
      case VPAD_CMD_REMOUNT:
        console_printf("Remounting filesystems");
        remount_filesystems();
        break;
    }
  }
}

int main(int argc, char **argv)
{
  (int) argc;
  (char **)argv;

  console_init();
  VPADInit();
  screen_init();
  network_init();

  console_printf("Starting FTPServU v%s", PACKAGE_VERSION);
  ftpserver_main_loop();
  console_printf("Shutting down.");

  ftp_deinit();
  console_deinit();
}
