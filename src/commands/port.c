#include <ctype.h>
#include "commands.h"
#include "ftp.h"

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

  if(digit != 5)
    return;
  for(int i = 0; i < 6; i++)
    if(bytes[i] < 0 || bytes[i] > 255)
      return;

  *ip = (bytes[0] << 24 | bytes[1] << 16 | bytes[2] << 8 | bytes[3]);
  *port = (bytes[4] << 8 | bytes[5]);
}

void do_port(client_t *client, char *args)
{
  u32 remote_ip = 0;
  u16 remote_port = 0;

  parse_port_args(args, &remote_ip, &remote_port);
  if(remote_ip == 0 || remote_port == 0)
  {
    ftp_response(500, client, "Failed to parse parameters.");
    return;
  }

  client->data = active.new(remote_ip, remote_port);
  client->data->client = client;
}
