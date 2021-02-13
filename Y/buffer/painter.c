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

#include <Y/buffer/painter.h>
#include <Y/buffer/painterclass.h>
#include <Y/util/yutil.h>

#include <string.h>

struct Painter *
painterCreate ()
{
  struct Painter *self = ymalloc (sizeof (struct Painter));
  self -> state = ymalloc (sizeof (struct PainterState));
  self -> c = NULL;
  self -> buffer = NULL;
  self -> state -> blendMode = COLOUR_BLEND_SOURCE_OVER;
  self -> state -> penColour = 0xFF000000;
  self -> state -> fillColour = 0;
  self -> state -> scaleX = 1.0;
  self -> state -> scaleY = 1.0;
  self -> state -> translateX = 0.0;
  self -> state -> translateY = 0.0;
  self -> state -> clipping = 0;
  self -> state -> clipRectangle.x = 0xdeadbeef;
  self -> state -> clipRectangle.y = 0x0a57beef;
  self -> state -> clipRectangle.w = 0xc01dbeef;
  self -> state -> clipRectangle.h = 0xc0edbeef;
  self -> state -> previous = NULL;
  return self;
}

void
painterDestroy (struct Painter *self)
{
  struct PainterState *state = self -> state;
  while (state != NULL)
    {
      struct PainterState *prev = state -> previous;
      yfree (state);
      state = prev;
    }
  yfree (self);
}

void
painterSaveState (struct Painter *self)
{
  struct PainterState *newState = ymalloc (sizeof (struct PainterState));
  memcpy (newState, self -> state, sizeof (struct PainterState));
  newState -> previous = self -> state;
  self -> state = newState;
}

void
painterRestoreState (struct Painter *self)
{
  if (self -> state -> previous != NULL)
    {
      struct PainterState *oldState = self -> state;
      self -> state = self -> state -> previous;
      yfree (oldState);
    }
  else
    Y_TRACE ("Painter State Stack Underflow");
}

static void
painterTransformCoordinates (struct Painter *self, int *x_p, int *y_p,
                             int *w_p, int *h_p)
{
  if (w_p != NULL)
    *w_p = *w_p * self -> state -> scaleX;
  if (h_p != NULL)
    *h_p = *h_p * self -> state -> scaleY;
  if (x_p != NULL)
    *x_p = *x_p * self -> state -> scaleX
           + self -> state -> translateX;
  if (y_p != NULL)
    *y_p = *y_p * self -> state -> scaleY
           + self -> state -> translateY;
}

void
painterTranslate (struct Painter *self, double x, double y)
{
  self -> state -> translateX += x * self -> state -> scaleX;
  self -> state -> translateY += y * self -> state -> scaleY;
}

void
painterScale (struct Painter *self, double x, double y)
{
  self -> state -> scaleX *= x;
  self -> state -> scaleY *= y;
}

void
painterClipTo (struct Painter *self, struct Rectangle *rect)
{
  struct Rectangle rectT = { rect->x, rect->y, rect->w, rect->h };
  painterTransformCoordinates (self, &rectT.x, &rectT.y, &rectT.w, &rectT.h);
  if (self -> state -> clipping)
    {
      if (rectangleIntersect (&(self -> state -> clipRectangle),
                              &(self -> state -> clipRectangle), &rectT) == 0)
        {
          self -> state -> clipRectangle.w = 0;
          self -> state -> clipRectangle.h = 0;
        }
    } 
  else
    {
      self -> state -> clipping = 1;
      self -> state -> clipRectangle.x = rectT.x;
      self -> state -> clipRectangle.y = rectT.y;
      self -> state -> clipRectangle.w = rectT.w;
      self -> state -> clipRectangle.h = rectT.h;
    }
}

void
painterEnter (struct Painter *self, struct Rectangle *rect)
{
  painterClipTo (self, rect);
  painterTranslate (self, rect -> x, rect -> y);
}

struct Rectangle *
painterGetClipRectangle (struct Painter *self)
{
  struct Rectangle *r = rectangleDuplicate (&(self -> state -> clipRectangle));
  r -> x -= self -> state -> translateX;
  r -> x /= self -> state -> scaleX;
  r -> y -= self -> state -> translateY;
  r -> y /= self -> state -> scaleY;
  r -> w /= self -> state -> scaleX;
  r -> h /= self -> state -> scaleY;
  return r;
}

int
painterFullyClipped (struct Painter *self)
{
  return self -> state -> clipping &&
    (self -> state -> clipRectangle.w == 0 ||
     self -> state -> clipRectangle.h == 0);
}

int
painterClipCoordinates (struct Painter *self, int *x_p, int *y_p,
                        int *w_p, int *h_p)
{
  struct Rectangle region = { *x_p, *y_p, *w_p, *h_p };
  if (!self -> state -> clipping)
    return 0;

  if (rectangleIntersect (&region, &region, &(self -> state -> clipRectangle)))
    {
      *x_p = region.x; *y_p = region.y; *w_p = region.w; *h_p = region.h;
    }
  else
    {
      *x_p = *y_p = *w_p = *h_p = 0;
    }
  return 1;
}

void
painterSetBlendMode (struct Painter *self, enum ColourBlendMode mode)
{
  self -> state -> blendMode = mode;
}

void
painterSetPenColour (struct Painter *self, uint32_t colour)
{
  self -> state -> penColour = colour;
}

uint32_t
painterGetPenColour (struct Painter *self)
{
  return self -> state -> penColour;
}

void
painterSetFillColour (struct Painter *self, uint32_t colour)
{
  self -> state -> fillColour = colour;
}

uint32_t
painterGetFillColour (struct Painter *self)
{
  return self -> state -> fillColour;
}

void
painterClearRectangle (struct Painter *self, int x, int y, int w, int h)
{
  /* perform co-ordinate transformation... */
  painterTransformCoordinates (self, &x, &y, &w, &h);
  painterClipCoordinates (self, &x, &y, &w, &h);
  self -> c -> clearRectangle (self, x, y, w, h);
}

void
painterDrawRectangle (struct Painter *self, int x, int y, int w, int h)
{
  /* perform co-ordinate transformation... */
  painterTransformCoordinates (self, &x, &y, &w, &h);
  self -> c -> drawRectangle (self, x, y, w, h);
}

void
painterDrawHLine (struct Painter *self, int x, int y, int dx)
{
  painterTransformCoordinates (self, &x, &y, &dx, NULL);
  self -> c -> drawHLine (self, x, y, dx);
}

void
painterDrawVLine (struct Painter *self, int x, int y, int dy)
{
  painterTransformCoordinates (self, &x, &y, NULL, &dy);
  self -> c -> drawVLine (self, x, y, dy);
}

void
painterDrawLine (struct Painter *self, int x, int y, int dx, int dy)
{
  painterTransformCoordinates (self, &x, &y, &dx, &dy);
  self -> c -> drawLine (self, x, y, dx, dy);
}

void
painterDrawBuffer (struct Painter *self, struct Buffer *buffer,
                   int x, int y, int w, int h, int xo, int yo)
{
  /* Visitor pattern */
  if (buffer == NULL)
    return;
  bufferDrawOnto (buffer, self, xo, yo, x, y, w, h);
}

void
painterDrawAlphamap (struct Painter *self, uint8_t *alpha,
                       int x, int y, int w, int h, int s)
{
  /* perform co-ordinate transformation... */
  painterTransformCoordinates (self, &x, &y, &w, &h);
  self -> c -> drawAlphamap (self, alpha, x, y, w, h, s);
}

void
painterDrawRGBAData (struct Painter *self, uint32_t *data,
                     int x, int y, int w, int h, int s)
{
  /* perform co-ordinate transformation... */
  painterTransformCoordinates (self, &x, &y, &w, &h);
  self -> c -> drawRGBAData (self, data, x, y, w, h, s);
}

/* arch-tag: bf17ba2a-5aa2-494b-82db-6c4a3ba68e79
 */
