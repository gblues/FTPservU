#ifndef _LOGGING_H
#define _LOGGING_H

void logging_init(void);
void logging_deinit(void);

int log_printf(const char *fmt, ...);

#endif /* _LOGGING_H */
