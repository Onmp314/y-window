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

#ifndef Y_THEMES_BASIC_WINDOW_H
#define Y_THEMES_BASIC_WINDOW_H

#include <Y/widget/window.h>
#include <Y/buffer/painter.h>

void basicWindowInit (struct Window *);
void basicWindowPaint (struct Window *, struct Painter *);
int  basicWindowGetRegion (struct Window *, int32_t, int32_t);
int  basicWindowPointerMotion (struct Window *, int32_t, int32_t, int32_t, int32_t);
int  basicWindowPointerButton (struct Window *, int32_t, int32_t, uint32_t, bool);
void basicWindowReconfigure (struct Window *, int32_t *, int32_t *, int32_t *, int32_t *,
                             int32_t *, int32_t *);
void basicWindowResize (struct Window *);

#endif

/* arch-tag: 0bdd3c50-3afa-4a15-9e62-b8eae4573a1b
 */
