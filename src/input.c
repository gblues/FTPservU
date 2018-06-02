#include <string.h>

#include "input.h"
#include "wiiu/vpad.h"

static int buttons = 0;

static int get_command(int held)
{
  int command = 0;

  if( (held & VPAD_BUTTON_HOME) && !(buttons & VPAD_BUTTON_HOME) )
    command = VPAD_CMD_EXIT;
  else if( (held & VPAD_BUTTON_Y) && !(buttons & VPAD_BUTTON_Y) )
    command = VPAD_CMD_REMOUNT;

  buttons = held;
  return command;
}

int input_handler(void)
{
  VPADStatus vpad;
  VPADReadError vpadError;

  memset(&vpad, 0, sizeof(vpad));
  VPADRead(0, &vpad, 1, &vpadError);

  if(!vpadError)
    return get_command(vpad.hold);

  return 0;
}
