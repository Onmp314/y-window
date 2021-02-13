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
#include <Y/util/dbuffer.h>
#include <Y/util/log.h>

#include <Y/main/control.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <netinet/in.h>

#define MESSAGE_HEADER_SIZE (4 * 8)

struct UnixData
{
  char *path;
  int listeningFD;
};

struct UnixClient
{
  struct Client client;
  int fd;
};

static void unixClose (struct Client *c);
static void unixWriteData (struct Client *self_c, size_t len);

struct ClientClass unixClientClass =
{
  name: "Unix Domain Socket Client",
  writeData: unixWriteData,
  close: unixClose
};

static struct UnixClient *
castBack (struct Client *self_c)
{
  /* assert ( self_c -> c == &unixClientClass ); */
  return (struct UnixClient *)self_c;
}

static void
unixWriteData (struct Client *self_c, size_t len)
{
  struct UnixClient *self = castBack (self_c);
  if (dbuffer_len(self -> client.sendq) > 0)
    controlChangeFileDescriptorMask (self -> fd,
                                     CONTROL_WATCH_WRITE | CONTROL_WATCH_READ );
}

static void
unixClose (struct Client *self_c)
{
  struct UnixClient *self = castBack (self_c);
  controlUnregisterFileDescriptor (self -> fd);
  close (self -> fd);
  yfree (self);
}

static void
doRead(struct UnixClient *self)
{
  char buf[4096];

  ssize_t ret = read(self->fd, buf, sizeof(buf));
  if (ret < 0)
    {
      int e = errno;
      if (e == EINTR || e == EAGAIN)
        return;
      Y_TRACE ("Read error from client %d: %s (%d)", clientGetID(&self->client), strerror(e), ret);
      clientClose (&(self -> client));
      return;
    }

  if (ret == 0)
    {
      Y_TRACE ("Connection closed by client %d", clientGetID(&self->client));
      clientClose (&(self -> client));
      return;
    }

  dbuffer_add(self->client.recvq, buf, ret);
  clientReadData(&self->client);
}

static void
doWrite(struct UnixClient *self)
{
  char buf[4096];
  size_t len = dbuffer_get(self->client.sendq, buf, sizeof(buf));

  ssize_t ret = write(self->fd, buf, len);
  if (ret < 0)
    {
      int e = errno;
      if (e == EINTR || e == EAGAIN)
        return;
      Y_TRACE ("Write error from client %d: %s", clientGetID(&self->client), strerror(e));
      clientClose (&(self -> client));
      return;
    }

  dbuffer_remove(self->client.sendq, ret);

  if (dbuffer_len(self->client.sendq) == 0)
    controlChangeFileDescriptorMask (self -> fd, CONTROL_WATCH_READ);
}

static void
unixClientReady (int fd, int causeMask, void *data_v)
{
  struct UnixClient *self = data_v;
  if (causeMask & CONTROL_WATCH_READ)
    doRead(self);
  if (causeMask & CONTROL_WATCH_WRITE)
    doWrite(self);
}

static void
unixSocketReady (int fd, int causeMask, void *data_v)
{
  /* struct UnixData *data = data_v; */
  struct UnixClient *newClient;

  int new_fd = accept (fd, NULL, NULL);
  if (new_fd == -1)
    return;

  newClient = ymalloc (sizeof (struct UnixClient));
  newClient -> fd = new_fd;

  newClient -> client.c = &unixClientClass;

  clientRegister (&(newClient -> client));

  controlRegisterFileDescriptor (newClient -> fd, CONTROL_WATCH_READ,
                                 newClient, unixClientReady);
  
};

int
initialise (struct Module *self, const struct Tuple *args)
{
  struct UnixData *data = ymalloc (sizeof (struct UnixData));
  struct sockaddr_un sockaddr;

  static char moduleName[] = "IPC: UNIX Domain Sockets";
  self -> name = moduleName;
  self -> data = data;
  data -> path = NULL;

  for (uint32_t i = 0; i < args->count; ++i)
    {
      if (args->list[i].type != t_string)
        continue;

      const char *arg = args->list[i].string.data;
      if (strncmp (arg, "socket=", 7) == 0
          && strlen (arg + 7) > 0)
        data -> path = ystrdup (arg + 7);
    }

  pid_t pid = getpid();
  char path[64];
  snprintf(path, 64, "/tmp/.Y-unix/%lu", (long unsigned int)pid);
  if (data -> path == NULL)
    data -> path = ystrdup (path);
  sockaddr.sun_family = AF_UNIX;
  strncpy (sockaddr.sun_path, data->path, 100);

  data -> listeningFD = socket (PF_UNIX, SOCK_STREAM, 0);

  if (data -> listeningFD < 0)
    {
      /* clean up */
      /* write error message to Y log */
      return 0;
    }

  mode_t old_umask = umask(0);
  if (mkdir("/tmp/.Y-unix", 0777) == -1 && errno != EEXIST)
    {
      umask(old_umask);
      return 0;
    }
  umask(old_umask);
 
  unlink (data -> path);
  int r = bind (data->listeningFD, (struct sockaddr *)&sockaddr, sizeof (sockaddr));

  if (r < 0)
    {
      /* clean up */
      /* write error message to Y log */
      return 0;
    }

  r = chmod (data->path, 0777);

  if (r < 0)
    {
      /* clean up */
      /* write error message to Y log */
      return 0;
    }

  r = listen (data->listeningFD, 64);

  if (r < 0)
    {
      /* clean up */
      /* write error message to Y log */
      return 0;
    }
  
  controlRegisterFileDescriptor (data->listeningFD, CONTROL_WATCH_READ,
                                 data, unixSocketReady);

  return 0;
}

int
finalise (struct Module *self)
{
  struct UnixData *data = self -> data;
  unlink (data -> path);
  yfree (data -> path);
  yfree (data);
  return 0;
}

/* arch-tag: 2679bc05-8215-4840-a073-12c1c35c6a3e
 */
