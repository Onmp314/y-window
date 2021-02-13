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

#ifndef Y_UTIL_INDEX_H
#define Y_UTIL_INDEX_H

struct Index;
struct IndexIterator;

/*
 *  Creates a new index
 *   keyFunction:         a function which return value is:
 *                             < 0    iff  key <  obj.key
 *                             = 0    iff  key == obj.key
 *                             > 0    iff  key  > obj.key
 *   comparisonFunction:  a function whose return value is:
 *                             < 0    iff  obj1.key <  obj2.key
 *                             = 0    iff  obj1.key == obj2.key
 *                             > 0    iff  obj1.key  > obj2.key
 */
struct Index *indexCreate  (int (*keyFunction)(const void *key, const void *obj),
                            int (*comparisonFunction)(const void *obj1, const void *obj2));

/*
 *  Destroys an index
 *   destructorFunction:  if non-null, this is called on all objects
 *                        in the index
 */
void          indexDestroy (struct Index *,
                            void (destructorFunction)(void *obj));

/* adds an object to the index */
void          indexAdd     (struct Index *, void *obj);

/* finds an object in the index */
void *        indexFind    (const struct Index *, const void *key);

/* removes an object from the index. DOES NOT free() THE OBJECT */
void *        indexRemove  (struct Index *, const void *key);

/* returns the number of items in the index */
int           indexCount   (const struct Index *);

/* interates over all items in the index */
void          indexIterate (struct Index *, void *userData,
                            void (*iterationFunction)(const void * /*obj*/, void *));

/* returns an iterator object pointing at the start of the index */
struct IndexIterator *indexGetStartIterator (struct Index *);

/* returns an iterator object pointing at the end of the index */
struct IndexIterator *indexGetEndIterator (struct Index *);

/* destroy an iterator */
void  indexiteratorDestroy  (struct IndexIterator *);
int   indexiteratorHasValue (struct IndexIterator *);
void *indexiteratorGet      (struct IndexIterator *);
void  indexiteratorNext     (struct IndexIterator *);
void  indexiteratorPrevious (struct IndexIterator *);

int  indexVerifyConstraints (struct Index *);

#endif

/* arch-tag: 1e934096-3f79-4ebc-bf43-59ec68407f27
 */
