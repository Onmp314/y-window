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

#ifndef Y_WIDGET_LABEL_H
#define Y_WIDGET_LABEL_H

struct Label;

#include <Y/y.h>
#include <Y/widget/widget.h>

struct Widget * labelToWidget  (struct Label *);
struct Object * labelToObject  (struct Label *);

#endif /* Y_WIDGET_LABEL_H */

/* arch-tag: d563ea8e-f926-4c2e-abb0-56a09917fb8d
 */
