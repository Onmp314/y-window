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

#ifndef Y_THEMES_BASIC_DRAW_H
#define Y_THEMES_BASIC_DRAW_H

#include <Y/buffer/painter.h>
#include <Y/widget/widget.h>

void basicDrawBackgroundPane (struct Painter *, int32_t, int32_t, int32_t, int32_t);
void basicDrawButtonPane (struct Painter *, int32_t, int32_t, int32_t, int32_t,
                          enum WidgetState);
void basicDrawCheckBox (struct Painter *, struct CheckBox *);
void basicDrawButton (struct Painter *, struct Button *);
void basicDrawLabel (struct Painter *, struct Label *);

#endif

/* arch-tag: b94dee59-2250-4c8b-9bdf-f470f7c12615
 */
