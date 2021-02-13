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

#ifndef Y_WIDGET_CONSOLE_H
#define Y_WIDGET_CONSOLE_H

struct Console;

#include <Y/y.h>
#include <Y/widget/widget.h>
#include <wchar.h>

struct Widget *   consoleToWidget  (struct Console *);
struct Object *   consoleToObject  (struct Console *);

void consoleClearRect              (struct Console *, uint32_t, uint32_t, uint32_t, uint32_t);
void consoleDrawText               (struct Console *, uint32_t, uint32_t, wchar_t *, uint32_t);
void consoleRing                   (struct Console *);
void consoleScrollView             (struct Console *, uint32_t destRow, uint32_t srcRow, uint32_t numLines);
void consoleSetRendition           (struct Console *, int, int, int, int, int, int, const char *);
void consoleSwapVideo              (struct Console *);
void consoleUpdateCursorPos        (struct Console *, uint32_t, uint32_t);


#endif /* Y_WIDGET_CONSOLE_H */

/* arch-tag: 5b0298fc-904b-4647-a238-89127956cbd0
 */
