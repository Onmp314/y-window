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

#ifndef Y_MODULES_WINDOWMANAGER_INTERFACE_H
#define Y_MODULES_WINDOWMANAGER_INTERFACE_H

#include <Y/y.h>
#include <Y/modules/windowmanager.h>
#include <Y/modules/module.h>
#include <Y/widget/widget.h>
#include <Y/widget/window.h>
#include <Y/screen/renderer.h>
#include <Y/util/rectangle.h>

struct WindowManager
{
  struct Module *module;
  void (*unload) (struct WindowManager *);
  void (*registerWindow) (struct WindowManager *, struct Window *);
  void (*unregisterWindow) (struct WindowManager *, struct Window *);
  void (*keyboard) (struct WindowManager *, int, int, int);
  void (*windowPointerButton) (struct WindowManager *, struct Window *,
                               int, int, int, int);
  void (*windowPointerMotion) (struct WindowManager *, struct Window *,
                               int, int, int, int);
  void (*windowPointerEnter) (struct WindowManager *, struct Window *,
                              int, int);
  void (*windowPointerLeave) (struct WindowManager *, struct Window *);
  struct Window * (*selectedWindow) (struct WindowManager *);
  void (*selectWindow) (struct WindowManager *, struct Window *);
  void (*maximiseWindow) (struct WindowManager *, struct Window *);
  void (*restoreWindow) (struct WindowManager *, struct Window *);
};

#endif

/* arch-tag: dc33795d-0686-4211-9cbc-6794789d40fc
 */
