/************************************************************************
 *   Copyright (C) Mark Thomas <markbt@efaref.net>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <Y/modules/module_interface.h>

#include <Y/message/message.h>
#include <Y/message/client.h>
#include <Y/message/client_p.h>

#include <Y/util/yutil.h>
#include <Y/util/index.h>

#include <Y/main/control.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <errno.h>

#define MESSAGE_HEADER_SIZE (4 * 8)

struct TcpData
{
  int port;
  int listeningFD;
  struct Index *tcpClients;
};

struct TcpClient
{
  struct Client client;
  int fd;
  struct TcpData *moduleData;
};

static void tcpWriteData (struct Client *self_c, size_t len);
static void tcpClose (struct Client *c);

struct ClientClass tcpClientClass =
{
  name: "TCP Socket Client",
  writeData: tcpWriteData,
  close: tcpClose
};

static struct TcpClient *
castBack (struct Client *self_c)
{
  /* assert ( self_c -> c == &tcpClientClass ); */
  return (struct TcpClient *)self_c;
}

static void
tcpWriteData (struct Client *self_c, size_t len)
{
  struct TcpClient *self = castBack (self_c);
  if (dbuffer_len(self -> client.sendq) > 0)
    controlChangeFileDescriptorMask (self -> fd,
                                     CONTROL_WATCH_WRITE | CONTROL_WATCH_READ );
}

static void
tcpClose (struct Client *self_c)
{
  struct TcpClient *self = castBack (self_c);
  indexRemove (self -> moduleData -> tcpClients, &(self -> client.id));
  controlUnregisterFileDescriptor (self -> fd);
  close (self -> fd);
  yfree (self);
}

static void
doRead(struct TcpClient *self)
{
  char buf[4096];
  ssize_t ret = read(self->fd, buf, sizeof(buf));

  if (ret < 0)
    {
      if (errno == EINTR || errno == EAGAIN)
        return;
      clientClose (&(self -> client));
      return;
    }

  if (ret == 0)
    {
      clientClose (&(self -> client));
      return;
    }

  dbuffer_add(self->client.recvq, buf, ret);
}

static void
doWrite(struct TcpClient *self)
{
  char buf[4096];
  size_t len = dbuffer_get(self->client.sendq, buf, sizeof(buf));
  ssize_t ret = write(self->fd, buf, len);

  if (ret < 0)
    {
      if (errno == EINTR || errno == EAGAIN)
        return;
      clientClose (&(self -> client));
      return;
    }

  dbuffer_remove(self->client.recvq, ret);

  if (dbuffer_len(self->client.sendq) == 0)
    controlChangeFileDescriptorMask (self -> fd, CONTROL_WATCH_READ);
}

static void
tcpClientReady (int fd, int causeMask, void *data_v)
{
  struct TcpClient *self = data_v;
  if (causeMask & CONTROL_WATCH_READ)
    doRead(self);
  if (causeMask & CONTROL_WATCH_WRITE)
    doWrite(self);
}

static void
tcpSocketReady (int fd, int causeMask, void *data_v)
{
  struct TcpData *data = data_v;
  struct TcpClient *newClient;

  int new_fd = accept (fd, NULL, NULL);
  if (new_fd == -1)
    return;

  newClient = ymalloc (sizeof (struct TcpClient));
  newClient -> fd = new_fd;
  newClient -> moduleData = data;

  newClient -> client.c = &tcpClientClass;

  indexAdd (data -> tcpClients, newClient);

  clientRegister (&(newClient -> client));

  controlRegisterFileDescriptor (newClient -> fd, CONTROL_WATCH_READ,
                                 newClient, tcpClientReady);
  
};

int
initialise (struct Module *self, const struct Tuple *args)
{
  struct TcpData *data = ymalloc (sizeof (struct TcpData));
  struct sockaddr_in sockaddr;

  static char moduleName[] = "IPC: TCP Sockets";
  self -> name = moduleName;
  self -> data = data;
  data -> port = 8900;

  for (uint32_t i = 0; i < args->count; ++i)
    {
      if (args->list[i].type != t_string)
        continue;

      const char *arg = args->list[i].string.data;
      if (strncmp (arg, "port=", 5) == 0
          && strlen (arg + 5) > 0)
        data -> port = strtoul (arg + 5, NULL, 10);
    }

  sockaddr.sin_family = AF_INET;
  sockaddr.sin_port = htons (data -> port);
  sockaddr.sin_addr.s_addr = INADDR_ANY;

  data -> listeningFD = socket (PF_INET, SOCK_STREAM, 0);

  if (data -> listeningFD < 0)
    {
      /* clean up */
      yfree (data);
      Y_ERROR ("Could not open socket: %s", strerror (errno));
      return 1;
    }
 
  int r = bind (data->listeningFD, (struct sockaddr *)&sockaddr, sizeof (sockaddr));

  if (r < 0)
    {
      /* clean up */
      close (data -> listeningFD);
      yfree (data);
      Y_ERROR ("Could not bind to socket: %s", strerror (errno));
      return 1;
    }

  r = listen (data->listeningFD, 64);

  if (r < 0)
    {
      /* clean up */
      close (data -> listeningFD);
      yfree (data);
      Y_ERROR ("Could not listen to socket: %s", strerror (errno));
      return 1;
    }

  data -> tcpClients = indexCreate (clientKeyFunction,
                                    clientComparisonFunction);
    
  controlRegisterFileDescriptor (data->listeningFD, CONTROL_WATCH_READ,
                                 data, tcpSocketReady);

  return 0;
}

int
finalise (struct Module *self)
{
  struct TcpData *data = self -> data;
  indexDestroy (data -> tcpClients, (void (*)(void *)) clientClose);
  controlUnregisterFileDescriptor (data -> listeningFD);
  close (data -> listeningFD);
  yfree (data);
  return 0;
}

/* arch-tag: 215b4026-8d84-4099-8d28-bcd810df0fe9
 */
