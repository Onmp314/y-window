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

#ifndef Y_MODULES_WINDOWMANAGER_H
#define Y_MODULES_WINDOWMANAGER_H

struct WindowManager;

#include <Y/y.h>
#include <Y/widget/widget.h>
#include <Y/widget/window.h>
#include <Y/screen/renderer.h>
#include <Y/util/rectangle.h>

void windowmanagerRegister (struct WindowManager *);

void           wmKeyboard (int, int, int);

void           wmRegisterWindow (struct Window *);
void           wmUnregisterWindow (struct Window *);
void           wmWindowPointerButton (struct Window *, int, int, int, int);
void           wmWindowPointerMotion (struct Window *, int, int, int, int);
void           wmWindowPointerEnter (struct Window *, int, int);
void           wmWindowPointerLeave (struct Window *);

struct Window *wmSelectedWindow (void);
void           wmSelectWindow (struct Window *);
void           wmMaximiseWindow (struct Window *);
void           wmRestoreWindow (struct Window *);

#endif /* macro guard */

/* arch-tag: c6156976-3148-4b1a-bc2c-3b5f54be4bec
 */
