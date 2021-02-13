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

#include <Y/y.h>
#include <Y/modules/windowmanager_interface.h>
#include <Y/modules/windowmanager.h>
#include <Y/util/yutil.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dlfcn.h>

static struct WindowManager *currentWindowManager = NULL;

void
windowmanagerRegister (struct WindowManager *wm)
{
  if (currentWindowManager != NULL)
    currentWindowManager -> unload (currentWindowManager);
  currentWindowManager = wm; 
}

void
wmRegisterWindow (struct Window *w)
{
  if (currentWindowManager == NULL)
    return;
  currentWindowManager -> registerWindow (currentWindowManager, w);
}

void
wmUnregisterWindow (struct Window *w)
{
  if (currentWindowManager == NULL)
    return;
  currentWindowManager -> unregisterWindow (currentWindowManager, w);
}

void
wmKeyboard (int key, int pressed, int modifiers)
{
  if (currentWindowManager == NULL)
    return;
  currentWindowManager -> keyboard (currentWindowManager, key, pressed, modifiers);
}

void
wmWindowPointerButton (struct Window *w, int x, int y, int b, int pressed)
{
  if (currentWindowManager == NULL)
    return;
  currentWindowManager -> windowPointerButton (currentWindowManager, w,
                                               x, y, b, pressed);
}

void
wmWindowPointerMotion (struct Window *w, int x, int y, int dx, int dy)
{
  if (currentWindowManager == NULL)
    return;
  currentWindowManager -> windowPointerMotion (currentWindowManager, w,
                                               x, y, dx, dy);
}

void
wmWindowPointerEnter (struct Window *w, int x, int y)
{
  if (currentWindowManager == NULL)
    return;
  currentWindowManager -> windowPointerEnter (currentWindowManager, w, x, y);
}

void
wmWindowPointerLeave (struct Window *w)
{
  if (currentWindowManager == NULL)
    return;
  currentWindowManager -> windowPointerLeave (currentWindowManager, w);
}

struct Window *
wmSelectedWindow (void)
{
  if (currentWindowManager == NULL)
    return NULL;
  return currentWindowManager -> selectedWindow (currentWindowManager);
}

void
wmSelectWindow (struct Window *w)
{
  if (currentWindowManager == NULL)
    return;
  return currentWindowManager -> selectWindow (currentWindowManager, w);
}

void
wmMaximiseWindow (struct Window *w)
{
  if (currentWindowManager == NULL)
    return;
  return currentWindowManager -> maximiseWindow (currentWindowManager, w);
}

void
wmRestoreWindow (struct Window *w)
{
  if (currentWindowManager == NULL)
    return;
  return currentWindowManager -> restoreWindow (currentWindowManager, w);
}

/* arch-tag: 07b8bf39-884b-49a3-a3e6-1769c44432a2
 */
