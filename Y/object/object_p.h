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

#ifndef Y_OBJECT_OBJECT_P_H
#define Y_OBJECT_OBJECT_P_H

#include <Y/y.h>
#include <Y/object/object.h>
#include <Y/object/class.h>
#include <Y/util/index.h>

struct Object
{
  uint32_t oid;
  struct Client *client;
  struct Class *c;
  struct Index *properties;
  struct Index *signals;
};

void objectInitialise (struct Object *, struct Class *);
void objectFinalise (struct Object *);

#endif /* header guard */

/* arch-tag: 5ff71bea-d068-4221-b46f-36caec802414
 */
