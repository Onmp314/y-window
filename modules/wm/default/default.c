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

#include <Y/modules/module_interface.h>
#include <Y/modules/windowmanager_interface.h>
#include <Y/object/object.h>
#include <Y/widget/desktop.h>
#include <Y/screen/screen.h>
#include <Y/util/zorder.h>
#include <Y/util/yutil.h>
#include <stdio.h>

static void defaultSelectWindow (struct WindowManager *wm, struct Window *w);

static struct Desktop *desktop;
static struct Window *currentWindow;

static void
defaultUnload (struct WindowManager *wm)
{
  moduleUnload ("wm/default");
}

static void
defaultRegisterWindow (struct WindowManager *wm, struct Window *w)
{
  desktopAddWindow (desktop, w);
}

static void
defaultUnregisterWindow (struct WindowManager *wm, struct Window *w)
{
  if (currentWindow == w)
    currentWindow = NULL;
  desktopRemoveWindow (desktop, w);
}

static void
defaultKeyboard (struct WindowManager *wm, int key, int pressed, int modifiers)
{
  if (pressed)
    {
      switch (key)
        {
          case YK_TAB:
             desktopCycleWindows (desktop, (modifiers & YMOD_SHIFT) ? -1 : 1);
             break;
          case YK_W:
          case YK_w:
          case YK_F4:
             if (currentWindow != NULL)
               windowRequestClose (currentWindow);
             break;
        }
    }
}

static void
defaultWindowPointerButton (struct WindowManager *wm, struct Window *w,
                            int x, int y, int button, int pressed)
{
  if (pressed == 1)
    {
      defaultSelectWindow (wm, w);
    }
}

static void
defaultWindowPointerMotion (struct WindowManager *wm, struct Window *w,
                            int x, int y, int dx, int dy)
{
}

static void
defaultWindowPointerEnter (struct WindowManager *wm, struct Window *w,
                           int x, int y)
{
}

static void
defaultWindowPointerLeave (struct WindowManager *wm, struct Window *w)
{
}

static struct Window *
defaultSelectedWindow (struct WindowManager *wm)
{
  return currentWindow;
}

static void
defaultSelectWindow (struct WindowManager *wm, struct Window *w)
{
  if (currentWindow != w)
    {
      if (currentWindow != NULL)
        {
          widgetRepaint (windowToWidget (currentWindow), NULL);
          keyboardRemoveFocus (windowGetFocussedWidget (currentWindow));
        }
      currentWindow = w;
      if (currentWindow != NULL)
        {
          keyboardSetFocus (windowGetFocussedWidget (currentWindow));
          widgetRepaint (windowToWidget (currentWindow), NULL);
          desktopRaiseWindow (desktop, currentWindow);
        }
    }
}

static void
defaultMaximiseWindow (struct WindowManager *wm, struct Window *win)
{
  int x, y, w, h;
  defaultSelectWindow (wm, win);
  widgetGetPosition (desktopToWidget (desktop), &x, &y);
  widgetGetSize (desktopToWidget (desktop), &w, &h);
  windowSaveGeometry (win);
  windowSetSizeState (win, WINDOW_SIZE_MAXIMISE);
  widgetMove (windowToWidget (win), x, y);
  widgetResize (windowToWidget (win), w, h);
}

static void
defaultRestoreWindow (struct WindowManager *wm, struct Window *win)
{
  defaultSelectWindow (wm, win);
  windowSetSizeState (win, WINDOW_SIZE_NORMAL);
  windowRestoreGeometry (win);
}

int
initialise (struct Module *m, const struct Tuple *args)
{
  struct WindowManager *self;
  self = ymalloc (sizeof (struct WindowManager));
  self -> unload              = defaultUnload;
  self -> registerWindow      = defaultRegisterWindow;
  self -> unregisterWindow    = defaultUnregisterWindow;
  self -> keyboard            = defaultKeyboard;
  self -> windowPointerButton = defaultWindowPointerButton;
  self -> windowPointerMotion = defaultWindowPointerMotion;
  self -> windowPointerEnter  = defaultWindowPointerEnter;
  self -> windowPointerLeave  = defaultWindowPointerLeave;
  self -> selectedWindow      = defaultSelectedWindow;
  self -> selectWindow        = defaultSelectWindow;
  self -> maximiseWindow      = defaultMaximiseWindow;
  self -> restoreWindow       = defaultRestoreWindow;
  self -> module = m;
  static char moduleName[] = "Default Window Manager";
  m -> name = moduleName;
  m -> data = self;

  windowmanagerRegister (self);

  desktop = desktopCreate ();
  screenSetRootWidget (desktopToWidget (desktop));
  return 0;
}

int
finalise (struct Module *m)
{
  objectDestroy (desktopToObject(desktop));
  yfree (m -> data);
  return 0;
}

/* arch-tag: bea4f81c-1f2d-41ba-be84-362a400a0cb0
 */
