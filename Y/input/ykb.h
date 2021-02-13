/************************************************************************
 *   Copyright (C) Andrew Suffield <asuffield@debian.org>
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

#ifndef Y_INPUT_YKB_H
#define Y_INPUT_YKB_H

#include <Y/y.h>
#include <Y/widget/widget.h>
#include <Y/input/ykbmap.h>
#include <Y/main/config.h>

/* A string is conventional keyboard input. An event is a symbolic name */
typedef void ykbStringHandler(struct Widget *, const char *str, uint16_t modifiers);
typedef void ykbEventHandler(struct Widget *, const char *event, uint16_t modifiers);
typedef void ykbStrokeHandler(struct Widget *, bool direction, uint16_t keycode, uint16_t modifiers);

/* This function should return true if keyboard input is possible and
 * false otherwise, and should give widget-local coordinates to the
 * "cursor" location if it returns true. These coordinates will be
 * used for the top-left corner of the extended input widget, if one
 * is needed.
 */
typedef bool ykbGetCursor(struct Widget *, int32_t *x, int32_t *y);

extern void ykbSetFocus(struct Widget *);

extern void ykbKeyDown(uint16_t keycode);
extern void ykbKeyUp(uint16_t keycode);

extern char *ykbDescribe(uint16_t keycode, uint16_t modifiers);

extern void ykbUseMapSet(struct ykbMapSet *mapset);

extern void ykbInitialise(struct Config *serverConfig);
extern void ykbFinalise(void);

#endif /* header guard */

/* arch-tag: d32c4867-b9ce-4f0c-91b2-286ee8eb6aba
 */
