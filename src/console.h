#ifndef _CONSOLE_H
#define _CONSOLE_H

void console_init(void);
void console_deinit(void);
void console_printf(const char *fmt, ...);

#include <stdarg.h>
#include <stdio.h>

/*
 * the prototype in DevKitPro's stdio.h appears to be broken, but the function exists because the
 * linker doesn't complain. So, here's a manual function prototype.
 */
int vasprintf(char **, const char *, __VALIST);

#endif
