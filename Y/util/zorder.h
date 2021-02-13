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

#ifndef Y_UTIL_ZORDER_H
#define Y_UTIL_ZORDER_H

struct ZOrder;
struct ZOrderIterator;

#include <Y/y.h>

struct ZOrder *zorderCreate    (int (*keyFunction)(const void *, const void *),
                                int (*comparisonFunction)(const void *, const void *));
void           zorderDestroy   (struct ZOrder *,
                                void (*destructorFunction)(void *));

void           zorderAddAtTop            (struct ZOrder *, void *);
void           zorderRemove              (struct ZOrder *, void *);
void           zorderMoveToTop           (struct ZOrder *, void *);
void           zorderMoveUp              (struct ZOrder *, void *);
void           zorderMoveDown            (struct ZOrder *, void *);
void           zorderMoveToBottom        (struct ZOrder *, void *);
void *         zorderGetTop              (struct ZOrder *);
void *         zorderGetBottom           (struct ZOrder *);

struct ZOrderIterator *zorderGetTopIterator (struct ZOrder *);
struct ZOrderIterator *zorderGetBottomIterator (struct ZOrder *);

void           zorderiteratorDestroy   (struct ZOrderIterator *);
int            zorderiteratorHasValue  (struct ZOrderIterator *);
void *         zorderiteratorGet       (struct ZOrderIterator *);
void           zorderiteratorMoveUp    (struct ZOrderIterator *);
void           zorderiteratorMoveDown  (struct ZOrderIterator *);

#endif

/* arch-tag: 97afbc1e-f573-4938-965c-32f8cb4ba4d7
 */
