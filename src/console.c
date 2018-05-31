#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "wiiu/os/screen.h"
#include "console.h"

#define MAX_CONSOLE_LINES_TV  27
#define MAX_CONSOLE_LINES_PAD 18

static char *tvConsole[MAX_CONSOLE_LINES_TV];
static char *padConsole[MAX_CONSOLE_LINES_PAD];

void console_screen_draw(int screen, char **term, int term_size)
{
  int i;
  OSScreenClearBufferEx(screen, 0);
  for(i = 0; i < term_size; i++)
    if(term[i])
      OSScreenPutFontEx(screen, 0, i, term[i]);

  OSScreenFlipBuffersEx(screen);
}

void console_draw()
{
  console_screen_draw(0, tvConsole, MAX_CONSOLE_LINES_TV);
  console_screen_draw(1, padConsole, MAX_CONSOLE_LINES_PAD);
}

static void shift_rows(char **rows, int count)
{
  int i;

  if(rows[0])
    free(rows[0]);
  for(i = 1; i < count; i++)
    rows[i-1] = rows[i];
}

/*
 * This implementation is pretty broken. Any line longer than 80 characters gets
 * truncated, and special characters like newlines and tabs aren't handled.
 */
void console_printf(const char *format, ...)
{
  char *formatted = NULL;
  int len;

  va_list va;
  va_start(va, format);
  len = vasprintf(&formatted, format, va);
  if( len >= 0 )
  {
    shift_rows(tvConsole, MAX_CONSOLE_LINES_TV);
    shift_rows(padConsole, MAX_CONSOLE_LINES_PAD);

    if(len > 79)
      formatted[79] = '\0';

    tvConsole[MAX_CONSOLE_LINES_TV-1] = strdup(formatted);
    padConsole[MAX_CONSOLE_LINES_PAD-1] = formatted;
  }
  va_end(va);
  console_draw();
}

void console_init()
{
  int i;

  for(i = 0; i < MAX_CONSOLE_LINES_TV; i++)
    tvConsole[i] = NULL;
  for(i = 0; i < MAX_CONSOLE_LINES_PAD; i++)
    padConsole[i] = NULL;
}

void console_deinit()
{
  int i;

  for(i = 0; i < MAX_CONSOLE_LINES_TV; i++)
    if(tvConsole[i]) free(tvConsole[i]);
  for(i = 0; i < MAX_CONSOLE_LINES_PAD; i++)
    if(padConsole[i]) free(padConsole[i]);
}
