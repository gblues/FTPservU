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

