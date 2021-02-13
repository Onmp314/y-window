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
#include <Y/object/class_p.h>
#include <Y/object/object_p.h>
#include <Y/util/yutil.h>
#include <Y/util/index.h>
#include <string.h>
#include <assert.h>

static struct Index *classNameIndex = NULL;
static struct Index *classIDIndex = NULL;
static int classNextID = 1;
static bool preinitDone = false;

struct Class
{
  char *name;
  uint32_t id;
  uint32_t superCount;
  char **superNameList;
  struct Class **superList;
  struct Index *classMethods;
  struct Index *instanceMethods;
  struct Index *properties;
};

struct Method
{
  char *name;
  /* Precisely one of these fields will be NULL and the other will be used */
  ClassMethod *classFunc;
  InstanceMethod *instanceFunc;
};

struct Property
{
  char *name;
  enum Type type;
  PropertyHook *hook;
};

static int
classNameKeyFunction (const void *key_v, const void *obj_v)
{
  const char *key = key_v;
  const struct Class *obj = obj_v;
  return strcmp (key, classGetName (obj));
}

static int
classNameComparisonFunction (const void *obj1_v, const void *obj2_v)
{
  const struct Class *obj1 = obj1_v;
  const struct Class *obj2 = obj2_v;
  return strcmp (classGetName (obj1), classGetName (obj2));
}

static int
methodKeyFunction (const void *key_v, const void *obj_v)
{
  const char *key = key_v;
  const struct Method *obj = obj_v;
  return strcmp (key, obj->name);
}

static int
methodComparisonFunction (const void *obj1_v, const void *obj2_v)
{
  const struct Method *obj1 = obj1_v;
  const struct Method *obj2 = obj2_v;
  return strcmp (obj1->name, obj2->name);
}

static void
methodDestructorFunction (void *obj_v)
{
  struct Method *obj = obj_v;
  if (!obj)
    return;
  yfree(obj->name);
  yfree(obj);
}

static int
propertyKeyFunction (const void *key_v, const void *obj_v)
{
  const char *key = key_v;
  const struct Property *obj = obj_v;
  return strcmp (key, obj->name);
}

static int
propertyComparisonFunction (const void *obj1_v, const void *obj2_v)
{
  const struct Property *obj1 = obj1_v;
  const struct Property *obj2 = obj2_v;
  return strcmp (obj1->name, obj2->name);
}

static void
propertyDestructorFunction (void *obj_v)
{
  struct Property *obj = obj_v;
  if (!obj)
    return;
  yfree(obj->name);
  yfree(obj);
}

static int
classIDKeyFunction (const void *key_v, const void *obj_v)
{
  const int *key_p = key_v;
  const struct Class *obj = obj_v;
  int obj_id = classGetID (obj);
  if (*key_p == obj_id)
    return 0;
  return (*key_p < obj_id) ? -1 : 1;
}

static int
classIDComparisonFunction (const void *obj1_v, const void *obj2_v)
{
  const struct Class *obj1 = obj1_v;
  int obj1_id = classGetID (obj1);
  const struct Class *obj2 = obj2_v;
  int obj2_id = classGetID (obj2);
  if (obj1_id == obj2_id)
    return 0;
  return (obj1_id < obj2_id) ? -1 : 1;
}

static void
classDestructorFunction (void *obj_v)
{
  struct Class *c = obj_v;
  if (!c)
    return;
  yfree(c->name);
  for (uint32_t i = 0; i < c->superCount; i++)
    yfree(c->superNameList[i]);
  yfree(c->superNameList);
  yfree(c->superList);
  indexDestroy(c->classMethods, methodDestructorFunction);
  indexDestroy(c->instanceMethods, methodDestructorFunction);
  indexDestroy(c->properties, propertyDestructorFunction);
  yfree(c);
}

static void
classPreInitialise(void)
{
  if (classNameIndex == NULL)
    classNameIndex = indexCreate (classNameKeyFunction,
                                  classNameComparisonFunction);
  if (classIDIndex == NULL)
    classIDIndex = indexCreate (classIDKeyFunction, classIDComparisonFunction);
}

static void
classSetup(struct Class *c)
{
  if (!preinitDone)
    return;
  if (c->superList != NULL)
    return;
  c->superList = ymalloc(sizeof(c->superList[0]) * c->superCount);
  for (uint32_t i = 0; i < c->superCount; i++)
    {
      c->superList[i] = indexFind (classNameIndex, c->superNameList[i]);
      /* This will have to be revisited later */
      assert(c->superList[i]);
    }
}

void
classInitialise (void)
{
  classPreInitialise();
  /* By the time we get to this point, all preinit tasks have
   * completed - all static classes have been registered
   */
  preinitDone = true;
  struct IndexIterator *i = indexGetStartIterator(classNameIndex);
  while (indexiteratorHasValue(i))
    {
      struct Class *c = indexiteratorGet(i);
      classSetup(c);
      indexiteratorNext(i);
    }
  indexiteratorDestroy(i);
}

void
classFinalise (void)
{
  indexDestroy (classNameIndex, NULL);
  indexDestroy (classIDIndex, classDestructorFunction);
}

const char *
classGetName (const struct Class *c)
{
  return c -> name;
}

int
classGetID (const struct Class *c)
{
  return c -> id;
}

static int
classCompareMethod (const void *key_v, const void *memb_v)
{
  const char *key = key_v;
  const struct Method *memb = memb_v;
  return strcmp (key, memb->name);
}

static const struct Method *
classFindClassMethod (const struct Class *c, const char *method)
{
  const struct Method *m = indexFind(c->classMethods, method);

  if (m)
    return m;

  for (uint32_t i = 0; i < c->superCount; i++)
    {
      m = classFindClassMethod(c->superList[i], method);
      if (m)
        return m;
    }

  return NULL;
}

struct Tuple *
classInvokeClassMethod (const struct Class *c, struct Client *from, const char *method,
                        const struct Tuple *args)
{
  const struct Method *m = classFindClassMethod(c, method);

  if (m)
    return m->classFunc(from, args);
  else
    return tupleBuildError(tb_string("Class method not found"), tb_string(classGetName(c)), tb_string(method));
}

static const struct Method *
classFindInstanceMethod (const struct Class *c, const char *method)
{
  const struct Method *m = indexFind(c->instanceMethods, method);

  if (m)
    return m;

  for (uint32_t i = 0; i < c->superCount; i++)
    {
      m = classFindInstanceMethod(c->superList[i], method);
      if (m)
        return m;
    }

  return NULL;
}

struct Tuple *
classInvokeInstanceMethod (struct Object *o, struct Client *from, const char *method,
                           const struct Tuple *args)
{
  const struct Method *m = classFindInstanceMethod(objectClass(o), method);

  if (m)
    return m->instanceFunc(o, from, args);
  else
    return tupleBuildError(tb_string("Instance method not found"),
                           tb_string(classGetName(objectClass(o))),
                           tb_string(method),
                           tb_uint32(objectGetID(o)));
}

bool
classInherits (const struct Class *c, const struct Class *super)
{
  /* A class inherits from another class if they're the same... */
  if (c == super)
    return true;

  /* ...or if one of its superclasses inherits */
  for (uint32_t i = 0; i < c->superCount; i++)
    {
      if (classInherits(c->superList[i], super))
        return true;
    }

  return false;
}

struct Class *
classFindByName (const char *name)
{
  struct Class *c;
  if (classNameIndex == NULL)
    return NULL;
  c = indexFind (classNameIndex, name);
  return c;
}

struct Class *
classFindByID (int id)
{
  struct Class *c;
  if (classIDIndex == NULL)
    return NULL;
  c = indexFind (classIDIndex, &id);
  return c;
}

struct Class *
classCreate(const char *name, uint32_t superc, const char **superv)
{
  classPreInitialise();
  struct Class *c = ymalloc(sizeof(*c));
  c->name = ystrdup(name);
  c->superCount = superc;
  c->superNameList = ymalloc(sizeof(c->superNameList[0]) * superc);
  for (uint32_t i = 0; i < superc; i++)
    c->superNameList[i] = ystrdup(superv[i]);
  c->superList = NULL;
  c->classMethods = indexCreate (methodKeyFunction, methodComparisonFunction);
  c->instanceMethods = indexCreate (methodKeyFunction, methodComparisonFunction);
  c->properties = indexCreate(propertyKeyFunction, propertyComparisonFunction);
  c->id = classNextID++;
  indexAdd (classNameIndex, c);
  indexAdd (classIDIndex, c);
  classSetup(c);
  return c;
}

void
classAddInstanceMethod(struct Class *class, const char *name, InstanceMethod *method)
{
  assert(class);
  assert(name);
  assert(method);
  struct Method *m = ymalloc(sizeof(*m));
  m->name = ystrdup(name);
  m->instanceFunc = method;
  indexAdd (class->instanceMethods, m);
}

void
classAddClassMethod(struct Class *class, const char *name, ClassMethod *method)
{
  assert(class);
  assert(name);
  assert(method);
  struct Method *m = ymalloc(sizeof(*m));
  m->name = ystrdup(name);
  m->classFunc = method;
  indexAdd (class->classMethods, m);
}

void
classAddProperty(struct Class *class, const char *name, enum Type type, PropertyHook *hook)
{
  assert(class);
  assert(name);

  struct Property *p = ymalloc(sizeof(*p));
  p->name = ystrdup(name);
  p->type = type;
  p->hook = hook;
  indexAdd (class->properties, p);
}

enum Type
classGetProperty(const struct Class *class, const char *name)
{
  assert(class);
  assert(name);

  struct Property *p = indexFind (class->properties, name);
  if (!p)
    return t_undef;
  return p->type;
}

void
classCallPropertyHook(struct Object *o, const char *name, const struct Value *old, const struct Value *new)
{
  const struct Class *c = objectClass(o);
  const struct Property *p = indexFind (c->properties, name);
  if (!p)
    return;
  if (!p->hook)
    return;
  (*p->hook)(o, old, new);
}

/* arch-tag: 8c8cb7e2-7d3d-4178-82cc-ab6db357bc36
 */
