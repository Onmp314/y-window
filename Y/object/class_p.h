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

#ifndef Y_OBJECT_CLASS_P_H
#define Y_OBJECT_CLASS_P_H

#include <Y/y.h>
#include <Y/object/class.h>
#include <Y/object/object.h>
#include <Y/message/message.h>

struct Class;

#if 0
struct Method
{
  const char *name;
  struct Tuple *(*m)(struct Object *, const struct Tuple *);
};

struct Class
{
  const char *       name;
  int                id;
  struct Class *     super;
  struct Object *  (*instantiate)(const struct Tuple *);
  void             (*destroy)(struct Object *);
/*   void             (*despatch)(struct Object *, struct Message *); */
  int                methodCount;
#if __GNUC__ >= 3
  struct Method      methods[]; 
#else
# ifndef CLASS_MAX_METHODS
#  define CLASS_MAX_METHODS 64
# endif
  struct Method      methods[CLASS_MAX_METHODS];
#endif
};
#endif

/* NOTA BENE:  method names must be sorted in     *
 *             strcmp order, as they will be      *
 *             searched using a bsearch call.     */

#endif


/* arch-tag: 704424e8-5652-46b0-b91f-0a921253d6f8
 */
