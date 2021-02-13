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

#include <Y/y.h>
#include <Y/message/client_p.h>
#include <Y/object/object_p.h>
#include <Y/const.h>
#include <Y/object/class_p.h>
#include <Y/util/yutil.h>
#include <stdlib.h>
#include <string.h>

struct PropertyValue
{
  char *name;
  struct Value *v;
};

struct Signal
{
  char *name;
  struct Index *clients;
};

static uint32_t objectNextID = 1;
static struct Index *objectIndex = NULL;

DEFINE_CLASS(Object);
#include "Object.yc"

int
objectKeyFunction (const void *key_v, const void *obj_v)
{
  const uint32_t *key = key_v;
  const struct Object *obj = obj_v;
  if (*key == obj -> oid)
    return 0;
  else if (*key < obj -> oid)
    return -1;
  else
    return 1; 
}

int
objectComparisonFunction (const void *obj1_v, const void *obj2_v)
{
  const struct Object *obj1 = obj1_v;
  const struct Object *obj2 = obj2_v;
  if (obj1 -> oid == obj2 -> oid)
    return 0;
  else if (obj1 -> oid < obj2 -> oid)
    return -1;
  else
    return 1; 
}

static int
propertyKeyFunction (const void *key_v, const void *obj_v)
{
  const char *key = key_v;
  const struct PropertyValue *obj = obj_v;
  return strcmp (key, obj -> name);
}

static int
propertyComparisonFunction (const void *obj1_v, const void *obj2_v)
{
  const struct PropertyValue *obj1 = obj1_v;
  const struct PropertyValue *obj2 = obj2_v;
  return strcmp (obj1 -> name, obj2 -> name);
}

static void
propertyDestructorFunction (void *obj)
{
  struct PropertyValue *prop = obj;
  valueDestroy(prop->v);
  yfree(prop->name);
  yfree(prop);
}

static int
signalKeyFunction (const void *key_v, const void *obj_v)
{
  const char *key = key_v;
  const struct Signal *obj = obj_v;
  return strcmp (key, obj -> name);
}

static int
signalComparisonFunction (const void *obj1_v, const void *obj2_v)
{
  const struct Signal *obj1 = obj1_v;
  const struct Signal *obj2 = obj2_v;
  return strcmp (obj1 -> name, obj2 -> name);
}

static void
signalDestructorFunction (void *obj)
{
  struct Signal *sig = obj;
  indexDestroy(sig->clients, NULL);
  yfree(sig->name);
  yfree(sig);
}

static int
stringComparisonFunction (const void *str1, const void *str2)
{
  return strcmp(str1, str2);
}

void
objectInitialise (struct Object *o, struct Class *c)
{
  o -> oid = objectNextID++;
  o -> c = c;
  o -> client = getCurrentClient();
  o -> properties = indexCreate (propertyKeyFunction,
                                 propertyComparisonFunction);
  o -> signals = indexCreate (signalKeyFunction, signalComparisonFunction);
  if (objectIndex == NULL)
    objectIndex = indexCreate (objectKeyFunction, objectComparisonFunction);
  indexAdd (objectIndex, o);
  clientAddObject(getCurrentClient(), o);
}

void
objectFinalise (struct Object *o)
{
  indexRemove (objectIndex, &(o -> oid)); 
  indexDestroy (o -> properties, propertyDestructorFunction); 
  indexDestroy (o -> signals, signalDestructorFunction);
  if (o->client != NULL)
    clientRemoveObject (o -> client, o);
  if (indexCount (objectIndex) == 0)
    {
      indexDestroy (objectIndex, NULL);
      objectIndex = NULL;
    }
}

uint32_t
objectGetID (const struct Object *o)
{
  return o -> oid;
}

const struct Class *
objectClass (const struct Object *o)
{
  return o -> c;
}

struct Object *
objectFind (uint32_t oid)
{
  struct Object *obj = indexFind (objectIndex, &oid);
  return obj;
}

void
objectEmitSignal_(struct Object *o, const char *name, struct Tuple *args)
{
  struct Signal *sig = indexFind (o->signals, name);
  if (!sig)
    return;

  struct IndexIterator *i;
  for (i = indexGetStartIterator (sig->clients); indexiteratorHasValue(i); indexiteratorNext(i))
    {
      struct Client *client = indexiteratorGet(i);
      struct Message *m = messageCreate(YMO_EVENT);
      m->to = clientGetID(client);
      m->id = o->oid;
      m->tuple = tupleDup(args);
      messageDespatch (NULL, m);
    }
  indexiteratorDestroy(i);
  tupleDestroy(args);
}

bool
objectSubscribeSignal (struct Client *c, struct Object *o, const char *name)
{
  struct Signal *sig = indexFind (o->signals, name);
  if (!sig)
    {
      sig = ymalloc (sizeof(*sig));
      sig->name = ystrdup (name);
      sig->clients = indexCreate (clientComparisonFunction, clientComparisonFunction);
      indexAdd (o->signals, sig);
    }

  if (indexFind (sig->clients, c) == NULL)
    {
      indexAdd (sig->clients, c);
      clientSubscribedSignal(c, o, name);
      return true;
    }
  else
    return false;
}

void
objectUnsubscribeSignal (struct Client *c, struct Object *o, const char *name)
{
  /* Find the signal object */
  struct Signal *sig = indexFind (o->signals, name);
  /* (or not) */
  if (!sig)
    return;
  /* Unsubscribe the client */
  if (indexRemove (sig->clients, name))
    clientUnsubscribedSignal(c, o, name);
  /* And remove the signal object if it's now unused */
  if (indexCount (sig->clients) == 0)
    signalDestructorFunction (indexRemove (o->signals, sig));
}

const struct Value *
objectGetProperty (struct Object *o, const char *name)
{
  struct PropertyValue *p = indexFind (o->properties, name);
  if (!p)
    return NULL;
  return p->v;
}

bool
objectSetProperty(struct Object *o, const char *name, const struct Value *v_in)
{
  /* First we check the type and try to cast */
  enum Type type = classGetProperty (objectClass(o), name);

  if (type == t_undef)
    {
      /* No such property */
      return false;
    }

  /* NULL or t_undef means "unset" */
  if (!v_in || v_in->type == t_undef)
    {
      struct PropertyValue *p = indexRemove (o->properties, name);
      classCallPropertyHook(o, name, p->v, NULL);
      propertyDestructorFunction(p);
      return true;
    }

  /* valueStaticCast needs a writeable copy of the value to work on */
  struct Value *v = valueDup(v_in);
  if (!valueStaticCast(v, v, type))
    return false;

  /* Now, is the property already set? */
  struct PropertyValue *p = indexFind (o->properties, name);
  struct Value *old = NULL;
  if (!p)
    {
      /* No, need to create one */
      p = ymalloc(sizeof(*p));
      p->name = ystrdup(name);
      indexAdd(o->properties, p);
    }
  else
    old = p->v;

  /* Store the value */
  p->v = v;
  classCallPropertyHook(o, name, v, old);
  valueDestroy(old);
  return true;
}

void
objectDestroy (struct Object *o)
{
  /* Unsubscribe all clients from this object */
  struct IndexIterator *i;
  for (i = indexGetStartIterator (o->signals); indexiteratorHasValue(i); indexiteratorNext(i))
    {
      struct Signal *sig = indexiteratorGet(i);
      struct IndexIterator *j;
      for (j = indexGetStartIterator (sig->clients); indexiteratorHasValue(j); indexiteratorNext(j))
        {
          struct Client *client = indexiteratorGet(j);
          clientUnsubscribedSignal(client, o, sig->name);
        }
      indexiteratorDestroy(j);
    }
  indexiteratorDestroy(i);

  classInvokeInstanceMethod(o, NULL, "DESTROY", NULL);
}

/* METHOD
 * subscribeSignal :: (string) -> ()
 */
void
objectCSubscribeSignal(struct Object *o, struct Client *from, const struct Tuple *args)
{
  const char *name = args->list[0].string.data;
  objectSubscribeSignal(from, o, name);
}

/* METHOD
 * unsubscribeSignal :: (string) -> ()
 */
void
objectCUnsubscribeSignal(struct Object *o, struct Client *from, const struct Tuple *args)
{
  const char *name = args->list[0].string.data;
  objectUnsubscribeSignal(from, o, name);
}

/* METHOD
 * getProperty :: (string) -> (any)
 */
struct Tuple *
objectCGetProperty(struct Object *o, const struct Tuple *args)
{
  const char *name = args->list[0].string.data;

  if (classGetProperty(objectClass(o), name) == t_undef)
    return tupleBuildError(tb_string("Property not found"));

  const struct Value *property = objectGetProperty (o, name);
  if (property)
    return tupleBuild(tb_value(*property));
  else
    return tupleBuild();
}

/* METHOD
 * setProperty :: (string, any) -> ()
 */
struct Tuple *
objectCSetProperty(struct Object *o, const struct Tuple *args)
{
  const char *name = args->list[0].string.data;

  if (classGetProperty(objectClass(o), name) == t_undef)
    return tupleBuildError(tb_string("Property not found"));

  const struct Value *v = &args->list[1];

  if (!objectSetProperty(o, name, v))
    return tupleBuildError(tb_string("Type mismatch"));

  return NULL;
}

/* arch-tag: c4962764-6b04-48f7-95bc-91d371fe3bb7
 */
