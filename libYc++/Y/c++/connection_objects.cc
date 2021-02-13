/************************************************************************
 *   Copyright (C) Andrew Suffield <asuffield@debian.org>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <Y/c++/connection.h>
#include <Y/c++/class.h>
#include <Y/c++/object.h>

#include "thread_support.h"

Y::Class *
Y::Connection::findClass (std::string className)
{
  Class *c = NULL;
  int oldtype;
  lock_mutex(classes_mutex, oldtype);
  std::map<std::string, Class *>::iterator i = classes.find(className);
  if (i == classes.end())
    c = NULL;
  else
    c = i->second;
  unlock_mutex(oldtype);

  if (!c)
    {
      c = new Class(this, className);
      classes[className] = c;
    }

  return c;
}

void
Y::Connection::createdObject (Object *obj)
{
  int oldtype;
  lock_mutex(objects_mutex, oldtype);
  objects[obj->id()] = obj;
  unlock_mutex(oldtype);
}

Y::Object *
Y::Connection::findObject (uint32_t oid)
{
  Object *obj = NULL;
  int oldtype;
  lock_mutex(objects_mutex, oldtype);
  std::map<uint32_t, Object *>::iterator i = objects.find(oid);
  if (i == objects.end())
    obj = NULL;
  else
    obj = i->second;
  unlock_mutex(oldtype);
  return obj;
}

void
Y::Connection::destroyObject (uint32_t oid)
{
  int oldtype;
  lock_mutex(objects_mutex, oldtype);
  std::map<uint32_t, Object *>::iterator i = objects.find(oid);
  if (i == objects.end())
    objects.erase(i);
  unlock_mutex(oldtype);
}

/* arch-tag: e3ec0e65-db55-4346-99c7-b6d4fc59dbcd
 */
