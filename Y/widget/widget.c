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

#include <Y/object/class_p.h>
#include <Y/widget/widget_p.h>
#include <Y/screen/screen.h>
#include <stdlib.h>

DEFINE_CLASS(Widget);
#include "Widget.yc"

/* SUPER
 * Object
 */

void
widgetInitialise (struct Widget *self, struct WidgetTable *t)
{
  self -> tab = t;
  self -> parent = NULL;
  self -> container = NULL;
  self -> state = WIDGET_STATE_NORMAL;
  self -> x = 0;
  self -> y = 0;
  self -> w = 0;
  self -> h = 0;
  self -> minWidth = -1;
  self -> minHeight = -1;
  self -> reqWidth = -1;
  self -> reqHeight = -1;
  self -> maxWidth = -1;
  self -> maxHeight = -1;
}

void
widgetFinalise (struct Widget *self)
{
  keyboardRemoveFocus (self);
  widgetUnpack (self -> container, self);
}

struct Object *
widgetToObject (struct Widget *self)
{
  return &(self -> o);
}

void
widgetGlobalToLocal (const struct Widget *self, int *x_p, int *y_p)
{
  if (self->container != NULL)
    widgetGlobalToLocal (self->container, x_p, y_p); 
  if (x_p != NULL)
    *x_p -= self -> x;
  if (y_p != NULL)
    *y_p -= self -> y;
}

void
widgetLocalToGlobal (const struct Widget *self, int *x_p, int *y_p)
{
  if (x_p != NULL)
    *x_p += self -> x;
  if (y_p != NULL)
    *y_p += self -> y;
  if (self->container != NULL)
    widgetLocalToGlobal (self->container, x_p, y_p);
}


struct Rectangle *
widgetGetRectangle (const struct Widget *self)
{
  return rectangleCreate (self -> x, self -> y, self -> w, self -> h);
}

void
widgetGetPosition (const struct Widget *self, int32_t *x_p, int32_t *y_p)
{
  if (self == NULL)
    return;
  if (x_p != NULL)
    *x_p = self -> x;
  if (y_p != NULL)
    *y_p = self -> y;
}

void
widgetGetSize (const struct Widget *self, int32_t *w_p, int32_t *h_p)
{
  if (self == NULL)
    return;
  if (w_p != NULL)
    *w_p = self -> w;
  if (h_p != NULL)
    *h_p = self -> h;
}

void
widgetGetConstraints (const struct Widget *self,
                      int32_t *minWidth_p, int32_t *minHeight_p,
                      int32_t *reqWidth_p, int32_t *reqHeight_p,
                      int32_t *maxWidth_p, int32_t *maxHeight_p)
{
  if (self == NULL)
    return;
  if (minWidth_p != NULL)
    *minWidth_p = self -> minWidth;
  if (minHeight_p != NULL)
    *minHeight_p = self -> minHeight;
  if (reqWidth_p != NULL)
    *reqWidth_p = self -> reqWidth;
  if (reqHeight_p != NULL)
    *reqHeight_p = self -> reqHeight;
  if (maxWidth_p != NULL)
    *maxWidth_p = self -> maxWidth;
  if (maxHeight_p != NULL)
    *maxHeight_p = self -> maxHeight;
}

enum WidgetState
widgetGetState (const struct Widget *self)
{
    return self->state;
}

bool
widgetContainsPoint (const struct Widget *self, int32_t x, int32_t y)
{
  if (self != NULL)
    {
      if (self -> tab -> containsPoint != NULL)
        return self -> tab -> containsPoint (self, x, y);
      else
        {
          if (x < 0 || y < 0)
            return false;
          return (x < self->w) && (y < self->h);
        }
    }
  else
    return false;
}

struct Window *
widgetGetWindow (struct Widget *self)
{
  if (self == NULL)
    return NULL;
  else if (self -> tab -> getWindow != NULL)
    return self -> tab -> getWindow (self);
  else if (self -> container != NULL)
    return widgetGetWindow (self -> container);
  else
    return NULL;
}

void
widgetMove (struct Widget *self, int32_t x, int32_t y)
{
  widgetRerender (self, NULL);
  self -> x = x;
  self -> y = y;
  widgetRerender (self, NULL);
}

void
widgetReconfigure (struct Widget *self)
{
  if (self == NULL)
    return;
  if (self -> tab -> reconfigure != NULL)
    self -> tab -> reconfigure (self);
  else
    widgetReconfigure (self -> container);
}

void
widgetResize (struct Widget *self, int32_t w, int32_t h)
{
  widgetRerender (self, NULL);
  if (w < 0)
    w = 0;
  if (h < 0)
    h = 0;
  self -> w = w;
  self -> h = h;
  if (self -> minWidth != -1 && self -> w < self -> minWidth)
    self -> w = self -> minWidth;
  if (self -> minHeight != -1 && self -> h < self -> minHeight)
    self -> h = self -> minHeight;
  if (self -> maxWidth != -1 && self -> w > self -> maxWidth)
    self -> w = self -> maxWidth;
  if (self -> maxHeight != -1 && self -> h > self -> maxHeight)
    self -> h = self -> maxHeight;
  if (self -> tab -> resize != NULL)
    self -> tab -> resize (self);
  widgetRerender (self, NULL);
}

void
widgetSetContainer (struct Widget *self, struct Widget *container)
{
  self -> container = container;
  if (container != NULL)
    widgetReconfigure (self);
}

void
widgetUnpack (struct Widget *self, struct Widget *w)
{
  if (self != NULL && self -> tab -> unpack != NULL)
    {
      self -> tab -> unpack (self, w);
    }
}


void
widgetRender (struct Widget *self, struct Renderer *renderer)
{
  if (self != NULL && self -> tab -> render != NULL)
    self -> tab -> render (self, renderer);
}

void
widgetPaint (struct Widget *self, struct Painter *painter)
{
  if (self != NULL && self -> tab -> paint != NULL)
    self -> tab -> paint (self, painter);
}

void
widgetRerender (struct Widget *self, struct Rectangle *rect)
{
  if (self == NULL)
    return;
  if (rect == NULL)
    rect = rectangleCreate (0, 0, self -> w, self -> h);
  rect -> x += self -> x;
  rect -> y += self -> y;
  if (self -> container != NULL)
    widgetRerender (self -> container, rect);
  else if (screenGetRootWidget () == self)
    screenInvalidateRectangle (rect);
  else
    rectangleDestroy (rect);
}

void
widgetRepaint (struct Widget *self, struct Rectangle *rect)
{
  if (self == NULL)
    return;
  if (rect == NULL)
    rect = rectangleCreate (0, 0, self -> w, self -> h);
  if (self -> tab -> repaint != NULL)
    self -> tab -> repaint (self, rect);
  else if (self -> container != NULL)
    {
      rect -> x += self -> x;
      rect -> y += self -> y;
      widgetRepaint (self -> container, rect);
    }
  else
    rectangleDestroy (rect);
}

int
widgetPointerMotion (struct Widget *self, int32_t x, int32_t y, int32_t dx, int32_t dy)
{
  if (self == NULL || self -> tab -> pointerMotion == NULL)
    return 0;
  return self -> tab -> pointerMotion (self, x, y, dx, dy);
}

int
widgetPointerButton (struct Widget *self, int32_t x, int32_t y, uint32_t b, bool pressed)
{
  if (self == NULL || self -> tab -> pointerButton == NULL)
    return 0;
  return self -> tab -> pointerButton (self, x, y, b, pressed);
}

void
widgetPointerEnter (struct Widget *self, int32_t x, int32_t y)
{
  if (self != NULL && self -> tab -> pointerEnter != NULL)
    self -> tab -> pointerEnter (self, x, y);
}

void
widgetPointerLeave (struct Widget *self)
{
  if (self != NULL && self -> tab -> pointerLeave != NULL)
    self -> tab -> pointerLeave (self);
}

int
widgetKeyboardRaw (struct Widget *self, enum YKeyCode code,
                   bool pressed, uint32_t modifierState)
{
  return self -> tab -> keyboardRaw (self, code, pressed, modifierState);
}

void
widgetykbString(struct Widget *self, const char *str, uint16_t modifiers)
{
  if (self && self->tab->ykbString)
    self->tab->ykbString(self, str, modifiers);
}

void
widgetykbEvent(struct Widget *self, const char *event, uint16_t modifiers)
{
  if (self && self->tab->ykbEvent)
    self->tab->ykbEvent(self, event, modifiers);
}

void
widgetykbStroke(struct Widget *self, bool direction, uint16_t keycode, uint16_t modifiers)
{
  if (self && self->tab->ykbStroke)
    self->tab->ykbStroke(self, direction, keycode, modifiers);
}

bool
widgetykbGetCursor(struct Widget *self, int32_t *x, int32_t *y)
{
  if (self && self->tab->ykbGetCursor)
    return self->tab->ykbGetCursor(self, x, y);
  else
    return false;
}

/* arch-tag: fb0a05b0-bb68-4b03-8d25-b88a7f2da626
 */
