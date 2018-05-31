#ifndef _COMMANDS_H
#define _COMMANDS_H

#include "wiiu/types.h"
#include "ftpservu_types.h"

struct command {
  uint8_t *mnemonic;
  bool authentication;
  void (*func)(client_t *client, uint8_t *params);
};

void do_user(client_t *client, uint8_t *command);
void do_pass(client_t *client, uint8_t *command);
void do_quit(client_t *client, uint8_t *command);
void do_reinitialize(client_t *client, uint8_t *command);
void do_noop(client_t *client, uint8_t *command);
void do_list(client_t *client, uint8_t *command);
void do_pwd(client_t *client, uint8_t *command);
void do_cwd(client_t *client, uint8_t *command);
void do_cdup(client_t *client, uint8_t *command);
void do_size(client_t *client, uint8_t *command);
void do_passive(client_t *client, uint8_t *command);
void do_port(client_t *client, uint8_t *command);
void do_type(client_t *client, uint8_t *command);
void do_system(client_t *client, uint8_t *command);
void do_mode(client_t *client, uint8_t *command);
void do_retrieve(client_t *client, uint8_t *command);
void do_store(client_t *client, uint8_t *command);
void do_append(client_t *client, uint8_t *command);
void do_restart(client_t *client, uint8_t *command);
void do_delete(client_t *client, uint8_t *command);
void do_mkdir(client_t *client, uint8_t *command);
void do_rmdir(client_t *client, uint8_t *command);
void do_rename_from(client_t *client, uint8_t *command);
void do_rename_to(client_t *client, uint8_t *command);
void do_namelist(client_t *client, uint8_t *command);
void do_site(client_t *client, uint8_t *command);
void do_allocate(client_t *client, uint8_t *command);

#endif /* _COMMANDS_H */
