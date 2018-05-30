#ifndef _COMMANDS_H
#define _COMMANDS_H

#include "wiiu/types.h"
#include "ftpservu_types.h"

void unauthenticated_command_processor(client_t *client, uint8_t *command);
void authenticated_command_processor(client_t *client, uint8_t *command);

#endif /* _COMMANDS_H */
