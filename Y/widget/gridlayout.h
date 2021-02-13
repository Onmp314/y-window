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

#ifndef Y_WIDGET_GRIDLAYOUT_H
#define Y_WIDGET_GRIDLAYOUT_H

struct GridLayout;

#include <Y/y.h>
#include <Y/widget/widget.h>

struct Widget *      gridlayoutToWidget  (struct GridLayout *);
struct Object *      gridlayoutToObject  (struct GridLayout *);

void gridlayoutAddWidget (struct GridLayout *, struct Object *, uint32_t, uint32_t, uint32_t, uint32_t);
void gridlayoutRemoveWidget (struct GridLayout *, struct Object *);

#endif /* Y_WIDGET_GRIDLAYOUT_H */

/* arch-tag: 3fbc3e6e-b55d-4ed3-a7bc-12dcc54ea929
 */
