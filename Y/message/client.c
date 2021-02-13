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

#include <Y/message/client.h>
#include <Y/message/client_p.h>
#include <Y/message/message.h>
#include <Y/util/index.h>
#include <Y/util/yutil.h>

#include <Y/object/class.h>

#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <assert.h>

struct SignalSubscription
{
  char *name;
  struct Object *obj;
};

static struct Client *currentClient = NULL;
static struct Index *clients = NULL;
static int clientNextID = 1;

int
clientKeyFunction (const void *key_v, const void *obj_v)
{
  const int *key_p = key_v;
  const struct Client *obj = obj_v;
  if (*key_p == obj->id)
    return 0;
  if (*key_p < obj->id)
    return -1;
  else
    return 1;
}

int
clientComparisonFunction (const void *obj1_v, const void *obj2_v)
{
  const struct Client *obj1 = obj1_v;
  const struct Client *obj2 = obj2_v;
  if (obj1->id == obj2->id)
    return 0;
  if (obj1->id < obj2->id)
    return -1;
  else
    return 1;
}

static int
signalsubscriptionComparisonFunction (const void *obj1_v, const void *obj2_v)
{
  const struct SignalSubscription *obj1 = obj1_v;
  const struct SignalSubscription *obj2 = obj2_v;
  if (obj1->obj != obj2->obj)
    return objectGetID(obj2->obj) - objectGetID(obj1->obj);
  else
    return strcmp (obj1 -> name, obj2 -> name);
}

static void
signalsubscriptionDestructorFunction (void *obj)
{
  struct SignalSubscription *sig = obj;
  if (!sig)
    return;
  yfree(sig->name);
  yfree(sig);
}

static void
clientDestructorFunction (void *obj)
{
  struct Client *c = obj;
  indexDestroy (c -> objects, (void(*)(void*))objectDestroy);
  indexDestroy (c -> signals, signalsubscriptionDestructorFunction);
  free_dbuffer(c -> recvq);
  free_dbuffer(c -> sendq);
  c -> c -> close (c);
}

void
clientInitialise (void)
{
  clients = indexCreate (clientKeyFunction, clientComparisonFunction);
  clientNextID = 1;
}

void
clientFinalise (void)
{
  indexDestroy(clients, clientDestructorFunction);
}

void
clientRegister (struct Client *c)
{
  Y_TRACE ("Registering client with ID: %d", clientNextID);
  c -> id = clientNextID ++;
  c -> objects = indexCreate (objectKeyFunction, objectComparisonFunction);
  c -> signals = indexCreate (signalsubscriptionComparisonFunction, signalsubscriptionComparisonFunction);
  c -> recvq = new_dbuffer();
  c -> sendq = new_dbuffer();
  indexAdd (clients, c);
}

void
clientClose (struct Client *c)
{
  Y_TRACE ("Closing client %d", c->id);

  struct IndexIterator *i;
  for (i = indexGetStartIterator (c->signals); indexiteratorHasValue(i); indexiteratorNext(i))
    {
      struct SignalSubscription *sub = indexiteratorGet(i);
      objectUnsubscribeSignal(c, sub->obj, sub->name);
    }
  indexiteratorDestroy(i);

  indexRemove (clients, &(c->id));
  clientDestructorFunction(c);
}

int
clientGetID (const struct Client *c)
{
  if (c == NULL)
    return 0;
  return c -> id;
}

struct Client *
clientFind (int id)
{
  if (clients == NULL)
    return NULL;
  return indexFind (clients, &id);
}

void
clientSendMessage (struct Client *c, struct Message *m)
{
  if (c == NULL)
    return;

  char *buf;
  size_t len;
  messageToString(m, &buf, &len);
  uint32_t nlen = htonl(len);
  dbuffer_add(c->sendq, (char *)&nlen, sizeof(nlen));
  dbuffer_add(c->sendq, buf, len);
  yfree(buf);
  c -> c -> writeData (c, len);
}

void
clientReadData (struct Client *c)
{
  uint32_t packet_len;
  while (dbuffer_len(c->recvq) >= sizeof(packet_len))
    {
      dbuffer_get(c->recvq, (char *)&packet_len, sizeof(packet_len));
      packet_len = ntohl(packet_len);

      if (dbuffer_len(c->recvq) < (sizeof(packet_len) + packet_len))
        break;

      char buf[packet_len];
      dbuffer_remove(c->recvq, sizeof(packet_len));
      dbuffer_extract(c->recvq, buf, packet_len);

      struct Message *m;
      if (!messageFromString(buf, packet_len, &m))
        {
          /* Protocol error */
          /* FIXME: this causes re-entrancy problems in the IPC
           * drivers, which will be fixed when they're refactored
           */
          Y_TRACE ("Protocol error from client %d (packet_len == %lu)", c->id, (long unsigned int)packet_len);
          clientClose(c);
          return;
        }
      messageDespatch(c, m);
    }
}

void
clientAddObject (struct Client *c, struct Object *o)
{
  if (c == NULL)
    return;
  Y_TRACE ("Adding object %d [%s] to client %d",
          objectGetID (o), classGetName (objectClass (o)), c->id);
  indexAdd (c -> objects, o);
}

void
clientRemoveObject (struct Client *c, struct Object *o)
{
  int oid = objectGetID (o);
  indexRemove (c -> objects, &oid);
}

void
clientSubscribedSignal (struct Client *c, struct Object *o, const char *name)
{
  /* We don't need to know about signals for objects belonging to us;
   * they will be destroyed when the connection is torn down anyway
   */
  uint32_t oid = objectGetID(o);
  if (indexFind(c->objects, &oid))
    return;

  struct SignalSubscription *sub = ymalloc(sizeof(*sub));
  sub->name = ystrdup(name);
  sub->obj = o;
  indexAdd (c->signals, sub);
}

void
clientUnsubscribedSignal (struct Client *c, struct Object *o, const char *name)
{
  /* Usual const-ness problem */
  size_t len = strlen(name);
  char vname[len + 1];
  memcpy(vname, name, len);
  vname[len] = '\0';

  const struct SignalSubscription vsub = {.name = vname, .obj = o};
  /* If this is one of our objects, then it won't be in the index, so
   * indexRemove will return NULL, and nothing will happen
   */
  signalsubscriptionDestructorFunction(indexRemove(c->signals, &vsub));
}

struct Client *
getCurrentClient(void)
{
  return currentClient;
}

void
setCurrentClient (struct Client *client)
{
  currentClient = client;
}

/* arch-tag: 8eb95cd6-369e-4879-961a-45aaef4d33c7
 */
