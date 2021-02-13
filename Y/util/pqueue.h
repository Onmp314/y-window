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

#ifndef Y_UTIL_PQUEUE_H
#define Y_UTIL_PQUEUE_H

struct PQueue;

struct PQueue *pqueueCreate (int (*comparisonFunction)(const void *obj1, const void *obj2));

void          pqueueDestroy (struct PQueue *,
                             void (*destructorFunction)(void *obj));

void          pqueueInsert (struct PQueue *, void *obj);

void *        pqueueGetNext (struct PQueue *);
void *        pqueuePeekNext (const struct PQueue *);

int           pqueueLength (const struct PQueue *);

void          pqueueRemove (struct PQueue *, void *userData, int (*testFunction)(void *obj, void *data));

#endif

/* arch-tag: ef9aff25-3610-4d7a-a59a-b6307ec11010
 */
