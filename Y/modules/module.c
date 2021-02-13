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
#include <Y/object/class.h>
#include <Y/modules/module.h>
#include <Y/util/yutil.h>
#include <Y/util/index.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dlfcn.h>
#include <assert.h>

DEFINE_CLASS(Module);
#include "Module.yc"

/* Modules should be changed to a ZOrder, as this will
 * effectively simulate dependencies
 */

struct Module
{
  char *moduleName;
  char *name;
  void *handle;
  void *data;
};

static void moduleUnloadImpl (const char *, struct Module *);

static int
moduleComparisonFunction (const void *m1_v, const void *m2_v)
{
  const struct Module *m1 = m1_v, *m2 = m2_v;
  return strcmp (m1 -> moduleName, m2 -> moduleName);
}

static int
moduleKeyFunction (const void *name_v, const void *m_v)
{
  const char *name = name_v;
  const struct Module *m = m_v;
  return strcmp (name, m -> moduleName);
}

static void
moduleDestructorFunction (void *m_v)
{
  struct Module *m = m_v;
  moduleUnloadImpl (m->moduleName, m);
}

static struct Index *moduleIndex = NULL;

void
moduleInitialise (struct Config *serverConfig)
{
  moduleIndex = indexCreate (moduleKeyFunction, moduleComparisonFunction);

  struct ConfigKeyIterator *i = configGetKeyIterator(serverConfig, "modules");
  if (!i)
    /* There is no modules group in the config file */
    return;

  /* No type constraints on module arguments */
  struct TupleType argsType = {.count = 1, .list = (enum Type[]) {t_list}};
  for (; configKeyIteratorHasValue(i); configKeyIteratorNext(i))
    {
      const char *name = configKeyIteratorName(i);
      struct Tuple *args = configKeyIteratorValue(i, &argsType);
      assert(args != NULL);
      assert(!args->error);

      moduleLoad(name, args);
      tupleDestroy(args);
    }

  configKeyIteratorDestroy(i);
}

void
moduleFinalise (void)
{
  indexDestroy (moduleIndex, moduleDestructorFunction);
}

void
moduleLoad (const char *moduleName, const struct Tuple *args)
{
  struct Module *module;
  int (*moduleInitialiseFunction)(struct Module *, const struct Tuple *);
  char *moduleFilename;
  int moduleFilenameLength;

  module = indexFind (moduleIndex, moduleName);
  if (module != NULL)
    {
      Y_ERROR ("Module '%s' already loaded.", moduleName);
      return;
    }

  module = ymalloc (sizeof (struct Module));

  moduleFilenameLength = strlen (yModuleDir) + strlen (moduleName) + 5;
                                                              /* / .so\0 */
  moduleFilename = ymalloc (moduleFilenameLength);
  snprintf (moduleFilename, moduleFilenameLength, "%s/%s.so", yModuleDir,
            moduleName);

  module->moduleName = ystrdup (moduleName);
  module->handle = dlopen (moduleFilename, RTLD_NOW);
  if (module->handle == NULL)
    {
      Y_ERROR ("Module: Failed to load %s: %s\n",
               moduleName, dlerror ());
      abort ();
    }

  yfree (moduleFilename);

  moduleInitialiseFunction = (int (*)(struct Module *, const struct Tuple *))
                             dlsym (module->handle, "initialise");
  if (moduleInitialiseFunction == NULL)
    {
      Y_ERROR ("Module: Failed to find initialise for %s: %s\n",
               moduleName, dlerror ());
      yfree (module);
      return;
    }

  if (moduleInitialiseFunction (module, args) != 0)
    {
      yfree (module);
      return;
    }
  indexAdd (moduleIndex, module);
}

void
moduleUnload (const char *name)
{
  struct Module *module;
  module = indexRemove (moduleIndex, name);
  moduleUnloadImpl (name, module);
}

void
moduleUnloadImpl (const char *name, struct Module *module)
{
  int (*moduleFinaliseFunction)(struct Module *);
  Y_TRACE ("Unload Module %s", name);
  if (module == NULL)
    return;
  moduleFinaliseFunction = dlsym(module -> handle, "finalise");
  if (moduleFinaliseFunction != NULL)
    {
      moduleFinaliseFunction (module);
    }
  dlclose (module -> handle);
  yfree (module -> moduleName);
  yfree (module);
}

/* METHOD
 * list :: () -> (...)
 */
struct Tuple *
moduleCList (const struct Tuple *args)
{
  struct Tuple *ret = tupleCreate(indexCount(moduleIndex));
  int i = 0;
  struct IndexIterator *iter = indexGetStartIterator (moduleIndex);
  while (indexiteratorHasValue (iter))
    {
      struct Module *module = indexiteratorGet (iter);
      char buffer[1024];
      snprintf (buffer, sizeof(buffer), "%s\t%s", module -> moduleName, module -> name);
      ret->list[i++] = tb_string(buffer);
      indexiteratorNext (iter);
    }
  indexiteratorDestroy (iter);
  assert(i == indexCount(moduleIndex));
  return ret;
}

/* METHOD
 * load :: (string, ...) -> ()
 */
void
moduleCLoad (const struct Tuple *args)
{
  const char *name = args->list[0].string.data;

  struct Tuple vargs = {.count = args->count - 1, .list = args->list + 1};

  moduleLoad (name, &vargs);
}

/* METHOD
 * unload :: (string) -> ()
 */
void
moduleCUnload (const struct Tuple *args)
{
  const char *name = args->list[0].string.data;
  moduleUnload (name);
}

/* arch-tag: 0644c645-a2c2-4680-b044-3090714ed36c
 */
