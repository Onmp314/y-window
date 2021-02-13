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

#ifndef Y_WIDGET_DESKTOP_H
#define Y_WIDGET_DESKTOP_H

struct Desktop;

#include <Y/y.h>
#include <Y/widget/widget.h>
#include <Y/widget/window.h>

struct Desktop * desktopCreate (void);

struct Widget *  desktopToWidget  (struct Desktop *);
struct Object *  desktopToObject  (struct Desktop *);

void desktopAddWindow    (struct Desktop *, struct Window *);
void desktopRaiseWindow  (struct Desktop *, struct Window *);
void desktopRemoveWindow (struct Desktop *, struct Window *);

void desktopCycleWindows (struct Desktop *, int direction);

#endif /* header guard */


/* arch-tag: afaeb21f-7145-4623-b10c-70d2f0795c2a
 */
