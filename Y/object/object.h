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

#ifndef Y_OBJECT_OBJECT_H
#define Y_OBJECT_OBJECT_H

struct Object;

#include <Y/y.h>
#include <Y/message/client.h>
#include <Y/message/message.h>

typedef struct Tuple *InstanceMethod(struct Object *, struct Client *, const struct Tuple *);

uint32_t            objectGetID (const struct Object *);
const struct Class *objectClass (const struct Object *);

struct Object *objectFind (uint32_t oid);

/* for putting objects in indices */
int objectComparisonFunction (const void *, const void *);
int objectKeyFunction (const void *, const void *);

const struct Value *objectGetProperty (struct Object *, const char *);
bool objectSetProperty(struct Object *, const char *, const struct Value *);

void         objectEmitSignal_(struct Object *, const char *, struct Tuple *);
#define      objectEmitSignal(obj, name, ...) \
  objectEmitSignal_(obj, name, tupleBuild(tb_string(name), ##__VA_ARGS__))

bool         objectSubscribeSignal (struct Client *, struct Object *, const char *);
void         objectUnsubscribeSignal (struct Client *, struct Object *, const char *);

void         objectDestroy (struct Object *);

#endif

/* arch-tag: 0fed6be1-efef-42f0-9cc8-e016a4fd181f
 */
