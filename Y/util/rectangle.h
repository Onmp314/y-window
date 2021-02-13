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

struct Rectangle;

#ifndef Y_UTIL_RECTANGLE_H
#define Y_UTIL_RECTANGLE_H

#include <inttypes.h>
#include <stdbool.h>

struct Rectangle
{
  int32_t x, y;
  int32_t w, h;
};

#include <Y/util/llist.h>

struct Rectangle *rectangleCreate (uint32_t x, uint32_t y, uint32_t w, uint32_t h);
struct Rectangle *rectangleDuplicate (const struct Rectangle *);
void              rectangleDestroy (struct Rectangle *);

/* dest may equal src1 or src2, e.g.
 *         rectangleUnion (r1, r1, r2)  <->  r1 = r1 U r2 */

/* returns true if Union reduces total area */
bool rectangleUnion     (struct Rectangle *dest,
                        const struct Rectangle *src1, const struct Rectangle *src2);

/* returns true if they intersected at all */
bool rectangleIntersect (struct Rectangle *dest,
                        const struct Rectangle *src1, const struct Rectangle *src2);

/* unions rectangles if they overlap */
void  rectanglelistUnionOverlaps (struct llist *);

/* get a list of intersecting rectangles */
struct llist *rectanglelistIntersectWith (struct llist *src1, struct llist *src);

#endif


/* arch-tag: a8754b11-0bb6-4acb-bec6-9d11ca967910
 */
