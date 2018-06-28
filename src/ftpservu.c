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
#include "network.h"
#include "ftp.h"
#include "input.h"
#include "console.h"
#include "logging.h"

#define TCP_PORT              21

static bool fs_mounted = false;
static int fsa_fd = -1;

extern int iosuhaxMount;

void __mount_filesystems()
{
  if(fs_mounted)
    return;

  if(iosuhaxMount)
  {
    vfs_fsa_init();
    vfs_mount("slccmpt01", "/dev/slccmpt01", "/vol/storage_slccmpt01");
    vfs_mount("storage_odd_tickets", "/dev/odd01", "/vol/storage_odd_tickets");
    vfs_mount("storage_odd_updates", "/dev/odd02", "/vol/storage_odd_updates");
    vfs_mount("storage_odd_content", "/dev/odd03", "/vol/storage_odd_content");
    vfs_mount("storage_odd_content2", "/dev/odd04", "/vol/storage_odd_content2");
    vfs_mount("storage_slc", NULL, "/vol/system");
    vfs_mount("storage_mlc", NULL, "/vol/storage_mlc01");
    vfs_mount("storage_usb", NULL, "/vol/storage_usb01");
  } else {
    vfs_mount_fat("sd");
  }

  fs_mounted = true;
}

void __unmount_filesystems()
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

void remount_filesystems()
{
  __unmount_filesystems();
  __mount_filesystems();
}

void ftpserver_main_loop(void)
{
  int serverSocket = 0;
  int cmd = 0;
  bool network_down = false;

  serverSocket = network_create_serversocket(TCP_PORT, FTP_SOCKET_BACKLOG);

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
  network_init();

  logging_init();
  VPADInit();

  console_printf("Starting FTPServU v%s", PACKAGE_VERSION);
  ftpserver_main_loop();
  console_printf("Shutting down.");

  ftp_deinit();
  console_deinit();
  logging_deinit();
}
