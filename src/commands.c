#include "commands.h"

/*
 * Commands that can be done on any open connection.
 */
static const unsigned char *public_commands[] = {
	"USER", // provide username
	"PASS", // provide password
	"QUIT", // terminate the connection
	"REIN", // Reinitialize (basically, logout w/o closing connection)
	"NOOP", // No-op to verify server is alive, respond w/ "OK"
	NULL
};

/*
 * Commands that require authentication (not that we actually do
 * authentication, but...
 */
static const unsigned char *auth_commands[] = {
  "USER", "PASS", "QUIT", "REIN", "NOOP", // see above
  "LIST", // list files in human-friendly format (`ls`)
  "PWD", // print working directory
  "CWD", // change working directory
  "CDUP", // go to parent directory
  "SIZE", // get file size in bytes
  "PASV", // switch to passive data mode
  "PORT", // establish a data connection(??)
  "TYPE", // set to either ASCII or binary mode
  "SYST", // get the type of operating system (e.g. uname)
  "MODE", // set transfer mode (stream, block, or compressed)
  "RETR", // retrieve a file
  "STOR", // upload a file
  "APPE", // append to a file
  "REST", // part of resume protocol; this tells the server where to start
  "DELE", // delete a file
  "MKD",  // create directory
  "RMD",  // remove directory
  "RNFR", // rename from - what file are we renaming?
  "RNTO", // rename to - must come immediately after RNFR
  "NLST", // list of filenames in current directory (eq. to `ls -1`)
  "SITE", // site-specific command interface
  "ALLO", // Allocate -- pre-allocate space for file to be uploaded
  NULL
};

void unauthenticated_command_processor(client_t *client, uint8_t *command)
{
}

void authenticated_command_processor(client_t *client, uint8_t *command)
{
}
