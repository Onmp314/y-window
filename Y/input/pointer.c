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

#include <Y/object/object.h>
#include <Y/widget/widget.h>
#include <Y/screen/screen.h>
#include <Y/input/pointer.h>
#include <Y/buffer/buffer.h>
#include <Y/buffer/bufferio.h>
#include <stdlib.h>
#include <stdio.h>

static int pointerX = 0, pointerY = 0;
static struct Widget *pointerWidget = NULL;

void
pointerGrab (struct Widget *w)
{
  pointerWidget = w;
}

void
pointerRelease ()
{
  pointerWidget = NULL;
}

void
pointerGetPosition (int *xp, int *yp)
{
  if (xp != NULL)
    *xp = pointerX;
  if (yp != NULL)
    *yp = pointerY;
}

void
pointerMovePosition (int dx, int dy)
{
  pointerSetPosition (pointerX + dx, pointerY + dy);
}

void
pointerSetPosition (int x, int y)
{
  struct Widget *w;
  int px = x;
  int py = y;
  int dx;
  int dy;
  screenConstrainPoint (&px, &py);
  dx = px - pointerX;
  dy = py - pointerY;
  if (px == pointerX && py == pointerY)
    return;
  screenInvalidateRectangle (rectangleCreate (pointerX, pointerY, 32, 32));
  pointerX = px;
  pointerY = py;
  if (pointerWidget != NULL)
    {
      w = pointerWidget;
      widgetGlobalToLocal (w, &px, &py);
    }
  else
    w = screenGetRootWidget ();

  screenInvalidateRectangle (rectangleCreate (pointerX, pointerY, 32, 32));

  widgetPointerMotion (w, px, py, dx, dy);
}

void
pointerButtonChange (int button, int pressed)
{
  struct Widget *w;
  int x = pointerX;
  int y = pointerY;

  if (pointerWidget != NULL)
    {
      w = pointerWidget;
      widgetGlobalToLocal (w, &x, &y);
    }
  else
    w = screenGetRootWidget ();

  widgetPointerButton (w, x, y, button, pressed);

}

static struct Buffer *pointerImageDefault = NULL;

static void
pointerLoadImages (void)
{
  char path[1025];
  char *filename;
  int l = strlen (yPointerImageDir);
  path[1025] = '\0';
  strncpy (path, yPointerImageDir, 1024);
  filename = path + l;
  *filename++ = '/';
  ++l;
  
  strncpy (filename, "default.png", 1024 - l);
  pointerImageDefault = bufferLoadFromFile (path, -1, -1);
}

struct Buffer *
pointerGetCurrentImage (void)
{
  if (pointerImageDefault == NULL)
    pointerLoadImages ();
  return pointerImageDefault;
}

void
pointerRender (struct Renderer *renderer)
{
  if (pointerImageDefault == NULL)
    pointerLoadImages ();

  struct Rectangle pointer;
  pointerGetPosition (&pointer.x, &pointer.y);
  bufferGetSize (pointerImageDefault, &pointer.w, &pointer.h);
  if (rendererEnter (renderer, &pointer, 0, 0))
    {
      bufferRender (pointerImageDefault, renderer, pointerX, pointerY);
    }
}

/* arch-tag: 64b8b5ba-85b7-47cc-a69d-9bc90707c923
 */
