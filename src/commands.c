#define _GNU_SOURCE

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "console.h"
#include "commands.h"
#include "ftp.h"
#include "network.h"

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
    printf("[commands]: got unknown/unimplemented mnemonic '%s'\n", mnemonic);
    if(IS_AUTHENTICATED(client))
    {
      ftp_response(502, client, "Command not implemented.");
      return;
    }
  }

  if( !cmd || (cmd->authentication && !IS_AUTHENTICATED(client)) )
  {
    ftp_response(530, client, "Please login with USER and PASS.");
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

/*
 * Takes a string in the form n1,n2,n3,n4,n5,n6
 * and parses out n1-n6, then maps them to IP/port.
 */
static void parse_port_args(char *str, u32 *ip, u16 *port)
{
  int bytes[6] = { 0, 0, 0, 0, 0, 0 };
  int segment = 0;
  int digit = 0;
  int tmp;

  for(int i = 0; str[i] != '\0' && digit < 6; i++)
  {
    if(isspace(str[i])) continue;
    if(!isdigit(str[i]) && str[i] != ',')
      return;

    if(isdigit(str[i]))
    {
      tmp = str[i] - '0';
      segment = (segment * 10) + tmp;
    }

    if(str[i] == ',')
    {
      if(segment > 255)
        return;

      bytes[digit] = segment;
      digit++;
      segment = 0;
    }
  }
  bytes[digit] = segment;

  // client didn't pass enough comma-separated values
  if(digit != 5)
    return;

  // client sent invalid byte sequences
  for(int i = 0; i < 6; i++)
    if(bytes[i] < 0 || bytes[i] > 255)
      return;
  // n1-n4 = client IP, n4-n5 = port
  *ip = (bytes[0] << 24 | bytes[1] << 16 | bytes[2] << 8 | bytes[3]);
  *port = (bytes[4] << 8 | bytes[5]);
}

void do_passive(client_t *client, char *command)
{
  if(client->data != NULL) {
    passive.free(client->data);
    client->data = NULL;
  }

  client->data = passive.new(network_get_host_ip(), network_get_ephermal_port());

  char *msg = NULL;

  asprintf(&msg, "Entering passive mode (%d,%d,%d,%d,%d,%d)",
    (client->data->ip & 0xff000000) >> 24,
    (client->data->ip & 0x00ff0000) >> 16,
    (client->data->ip & 0x0000ff00) >> 8,
    (client->data->ip & 0x000000ff),
    (client->data->port & 0xff00) >> 8,
    (client->data->port & 0x00ff));

  if(msg != NULL)
  {
    ftp_response(227, client, msg);
    free(msg);
  }
}

void do_port(client_t *client, char *command)
{
  u32 remote_ip = 0;
  u16 remote_port = 0;

  parse_port_args(command, &remote_ip, &remote_port);
  if( remote_ip == 0 || remote_port == 0 )
  {
    ftp_response(500, client, "Failed to parse parameters.");
    return;
  }

  client->data = active.new(remote_ip, remote_port);
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

