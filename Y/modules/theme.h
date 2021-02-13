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

#ifndef Y_MODULES_THEME_H
#define Y_MODULES_THEME_H

#include <stdbool.h>
#include <Y/y.h>
#include <Y/text/font.h>
#include <Y/modules/theme_interface.h>

void themeDrawBackgroundPane   (struct Painter *, int32_t x, int32_t y, int32_t w, int32_t h);
void themeDrawButtonPane       (struct Painter *, int32_t x, int32_t y, int32_t w, int32_t h,
                                enum WidgetState state);

void themeDrawCheckBox         (struct Painter *, struct CheckBox *);
void themeDrawLabel            (struct Painter *, struct Label *);
void themeDrawButton           (struct Painter *, struct Button *);

struct Font *
     themeGetDefaultFont       (void);

void themeWindowInit           (struct Window *);
void themeWindowPaint          (struct Window *, struct Painter *);
int  themeWindowPointerMotion  (struct Window *, int32_t, int32_t, int32_t, int32_t);
int  themeWindowPointerButton  (struct Window *, int32_t, int32_t, uint32_t, bool);
void themeWindowReconfigure    (struct Window *, int32_t *, int32_t *, int32_t *, int32_t *,
                                int32_t *, int32_t *);
void themeWindowResize         (struct Window *);

#endif /* header guard */

/* arch-tag: c84878a9-90e5-43f8-87cf-b8008be413dd
 */
