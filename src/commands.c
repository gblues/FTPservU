#include <string.h>

#include "console.h"
#include "commands.h"
#include "ftp.h"

static const command_t commands[] = {
  { "USER", false, do_user },
  { "PASS", false, do_pass },
  { "QUIT", false, do_quit },
  { "REIN", false, do_reinitialize },
  { "NOOP", false, do_noop },
  { "LIST", true, do_list },        // list files in human-friendly format (`ls`)
  { "PWD",  true, do_pwd },         // print working directory
  { "CWD",  true, do_cwd },         // change working directory
  { "CDUP", true, do_cdup },        // go to parent directory
  { "SIZE", true, do_size },        // get file size in bytes
  { "PASV", true, do_passive },     // switch to passive data mode
  { "PORT", true, do_port },        // establish a data connection(??)
  { "TYPE", true, do_type },        // set to either ASCII or binary mode
  { "SYST", true, do_system },      // get the type of operating system (e.g. uname)
  { "MODE", true, do_mode },        // set transfer mode (stream, block, or compressed)
  { "RETR", true, do_retrieve },    // retrieve a file
  { "STOR", true, do_store },       // upload a file
  { "APPE", true, do_append },      // append to a file
  { "REST", true, do_restart },     // part of resume protocol; this tells the server where to start
  { "DELE", true, do_delete },      // delete a file
  { "MKD",  true, do_mkdir },       // create directory
  { "RMD",  true, do_rmdir },       // remove directory
  { "RNFR", true, do_rename_from }, // rename from - what file are we renaming?
  { "RNTO", true, do_rename_to },   // rename to - must come immediately after RNFR
  { "NLST", true, do_namelist },    // list of filenames in current directory (eq. to `ls -1`)
  { "SITE", true, do_site },        // site-specific command interface
  { "ALLO", true, do_allocate },    // Allocate -- pre-allocate space for file to be uploaded
  { NULL, false, NULL }
};

static const command_t *find_command(char *mnemonic)
{
  for(int i = 0; commands[i].mnemonic != NULL; i++)
    if(!strcmp(mnemonic, commands[i].mnemonic))
      return &commands[i];

  return NULL;
}

void command_invoke(client_t *client, char *mnemonic, char *parameter)
{
  const command_t *cmd = find_command(mnemonic);
  if(!cmd)
  {
    console_printf("ERROR: failed to find a command for mnemonic: '%s'", mnemonic);
    return;
  }

  if(cmd->authentication && !IS_AUTHENTICATED(client))
  {
    console_printf("ERROR: cannot use '%s' without being authenticated.", mnemonic);
    return;
  }

  cmd->func(client, parameter);
}

/*
 * According to RFC, a user may use the USER command to switch accounts
 * at any time. Specifying the USER zaps all previous authentication info.
 */
void do_user(client_t *client, char *command)
{
  client->state = STATE_USER;
  ftp_response(331, client, "Username OK. Ready for password.");
}

/**
 * PASS must come immediately after USER. Any other usage is an error.
 * If PASS is given first, we provide a nice "need account" response.
 * Otherwise a meaner "wrong order, dummy" message.
 */
void do_pass(client_t *client, char *command)
{
  switch(client->state)
  {
    /* no user given yet */
    case STATE_NONE:
    ftp_response(332, client, "Need account for login.");
    break;
    case STATE_USER:
    /* user has been given */
    client->state = STATE_AUTH;
    ftp_response(230, client, "User logged in successfully.");
    break;
    /* user is already authenticated */
    default:
    ftp_response(503, client, "Bad sequence of commands.");
    break;
  }
}

void do_quit(client_t *client, char *command)
{
  client->state |= STATE_DISCONN;
  ftp_response(221, client, "Service closing control connection.");
}

void do_reinitialize(client_t *client, char *command)
{
  client->state = STATE_NONE;
  ftp_response(220, client, "Service ready for new user.");
}

void do_noop(client_t *client, char *command)
{
  ftp_response(200, client, "Okey dokey artichokey!");
}

void do_list(client_t *client, char *command)
{
}

void do_pwd(client_t *client, char *command)
{
}

void do_cwd(client_t *client, char *command)
{
}

void do_cdup(client_t *client, char *command)
{
}

void do_size(client_t *client, char *command)
{
}

void do_passive(client_t *client, char *command)
{
}

void do_port(client_t *client, char *command)
{
}

void do_type(client_t *client, char *command)
{
}

void do_system(client_t *client, char *command)
{
  ftp_response(215, client, "UNIX Type: L8 Version: FTPservU");
}

void do_mode(client_t *client, char *command)
{
}

void do_retrieve(client_t *client, char *command)
{
}

void do_store(client_t *client, char *command)
{
}

void do_append(client_t *client, char *command)
{
}

void do_restart(client_t *client, char *command)
{
}

void do_delete(client_t *client, char *command)
{
}

void do_mkdir(client_t *client, char *command)
{
}

void do_rmdir(client_t *client, char *command)
{
}

void do_rename_from(client_t *client, char *command)
{
}

void do_rename_to(client_t *client, char *command)
{
}

void do_namelist(client_t *client, char *command)
{
}

void do_site(client_t *client, char *command)
{
}

void do_allocate(client_t *client, char *command)
{
}

