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

#include <Y/widget/canvas.h>
#include <Y/widget/widget_p.h>

#include <Y/util/yutil.h>
#include <Y/buffer/painter.h>
#include <Y/buffer/buffer.h>
#include <Y/buffer/rgbabuffer.h>

#include <Y/object/class_p.h>
#include <Y/object/object_p.h>

#include <Y/text/font.h>

#include <stdio.h>
#include <ctype.h>
#include <string.h>

struct Canvas
{
  struct Widget widget;
  struct Buffer *front, *back;
  struct Painter *painter;
  int resizing;
};

static void canvasResize (struct Widget *);
static void canvasRender (struct Widget *, struct Renderer *);

DEFINE_CLASS(Canvas);
#include "Canvas.yc"

/* SUPER
 * Widget
 */

/* PROPERTY
 * background :: uint32
 */

static struct WidgetTable canvasTable =
{
  resize:        canvasResize,
  render:        canvasRender,
};

static inline struct Canvas *
castBack (struct Widget *widget)
{
  /* assert ( widget -> c == canvasClass ); */
  return (struct Canvas *)widget;
}

static inline const struct Canvas *
castBackConst (const struct Widget *widget)
{
  /* assert ( widget -> c == canvasClass ); */
  return (const struct Canvas *)widget;
}

static void
canvasRender (struct Widget *self_w, struct Renderer *renderer)
{
  struct Canvas *self = castBack (self_w);
  int fw, fh, bw, bh;
  bufferGetSize (self -> front, &fw, &fh);
  bufferGetSize (self -> back, &bw, &bh); 
  bufferRender (self -> front, renderer, 0, 0);
}

static struct Canvas *
canvasCreate (void)
{
  struct Canvas *self = ymalloc (sizeof (struct Canvas));
  struct Rectangle rect;
  objectInitialise (&(self -> widget.o), CLASS(Canvas));
  widgetInitialise (&(self -> widget), &canvasTable);
  self -> front = rgbabufferToBuffer (rgbabufferCreate ());
  self -> back  = rgbabufferToBuffer (rgbabufferCreate ());
  self -> resizing = 0;
  bufferSetSize (self -> front, 64, 64);
  bufferSetSize (self -> back, 64, 64);
  self -> painter = bufferGetPainter (self -> back);
  rect.x = 0;
  rect.y = 0;
  rect.w = 64;
  rect.h = 64;
  painterClipTo (self -> painter, &rect);
  painterSetFillColour (self -> painter, 0);
  painterClearRectangle (self -> painter, 0, 0, 64, 64);
  return self;
}

/* METHOD
 * DESTROY :: () -> ()
 */
static void
canvasDestroy (struct Canvas *self)
{
  painterDestroy (self -> painter);
  bufferDestroy (self -> front);
  bufferDestroy (self -> back);
  widgetFinalise (canvasToWidget (self));
  objectFinalise (canvasToObject (self));
  yfree (self);
}

struct Widget *
canvasToWidget (struct Canvas *self)
{
  return &(self -> widget);
}

struct Object *
canvasToObject (struct Canvas *self)
{
  return &(self -> widget.o);
}

/* METHOD
 * Canvas :: () -> (object)
 */
static struct Object *
canvasInstantiate (void)
{
  return canvasToObject (canvasCreate ());
}

void
canvasResize (struct Widget *self_w)
{
  struct Canvas *self = castBack (self_w);

  if (self -> resizing == 0)
   {
     objectEmitSignal (canvasToObject (self), "resize");
     self -> resizing = 1;
   }
  
}

/* METHOD
 * savePainterState :: () -> ()
 */
void
canvasSavePainterState (struct Canvas *self)
{
  painterSaveState (self -> painter);
}

/* METHOD
 * restorePainterState :: () -> ()
 */
void
canvasRestorePainterState (struct Canvas *self)
{
  painterRestoreState (self -> painter);
}

/* METHOD
 * setBlendMode :: () -> ()
 */
void
canvasSetBlendMode (struct Canvas *self)
{
}

/* METHOD
 * setPenColour :: (uint32) -> ()
 */
void
canvasSetPenColour (struct Canvas *self, uint32_t colour)
{
  painterSetPenColour (self -> painter, colour);
}

/* METHOD
 * setFillColour :: (uint32) -> ()
 */
void
canvasSetFillColour (struct Canvas *self, uint32_t colour)
{
  painterSetFillColour (self -> painter, colour);
}

void
canvasReset (struct Canvas *self, int32_t *w, int32_t *h)
{
  int32_t fc = painterGetFillColour (self -> painter);
  self -> resizing = 0;
  painterSetFillColour (self -> painter, safeGetProperty(self, background, 0));
  bufferSetSize (self -> back, self -> widget.w, self -> widget.h);
  painterDestroy (self -> painter);
  self -> painter = bufferGetPainter (self -> back);
  painterClearRectangle (self -> painter, 0, 0,
                         self -> widget.w, self -> widget.h);
  painterSetFillColour (self -> painter, fc);
  *w = self -> widget.w;
  *h = self -> widget.h;
}

/* METHOD
 * reset :: () -> (uint32, uint32)
 */
static struct Tuple *
canvasCReset (struct Canvas *self)
{
  int32_t w, h;
  
  canvasReset (self, &w, &h);

  return tupleBuild(tb_uint32(w), tb_uint32(h));  
}

/* METHOD
 * clearRectangles :: () -> ()
 */
void
canvasClearRectangles (struct Canvas *self)
{
}

/* METHOD
 * drawRectangles :: () -> ()
 */
void
canvasDrawRectangles (struct Canvas *self)
{
}

void
canvasDrawHLine (struct Canvas *self, int32_t x, int32_t y, int32_t dx)
{
  painterDrawHLine (self -> painter, x, y, dx);
}

/* METHOD
 * drawHLines :: (...) -> ()
 */
static void
canvasCDrawHLines (struct Canvas *self, const struct Tuple *args)
{
  for (uint32_t i = 0; i < args->count - 2; i += 3)
    {
      if (args->list[i + 0].type != t_uint32)
        continue;
      if (args->list[i + 1].type != t_uint32)
        continue;
      if (args->list[i + 2].type != t_uint32)
        continue;
      canvasDrawHLine (self,
                       args->list[i + 0].uint32, args->list[i + 1].uint32,
                       args->list[i + 2].uint32
                       );
    }
}

void
canvasDrawVLine (struct Canvas *self, int32_t x, int32_t y, int32_t dy)
{
  painterDrawVLine (self -> painter, x, y, dy);
}

/* METHOD
 * drawVLines :: (...) -> ()
 */
static void
canvasCDrawVLines (struct Canvas *self, const struct Tuple *args)
{
  for (uint32_t i = 0; i < args->count - 2; i += 3)
    {
      if (args->list[i + 0].type != t_uint32)
        continue;
      if (args->list[i + 1].type != t_uint32)
        continue;
      if (args->list[i + 2].type != t_uint32)
        continue;
      canvasDrawVLine (self,
                       args->list[i + 0].uint32, args->list[i + 1].uint32,
                       args->list[i + 2].uint32
                       );
    }
}

void
canvasDrawLine (struct Canvas *self, int32_t x, int32_t y, int32_t dx, int32_t dy)
{
  painterDrawLine (self -> painter, x, y, dx, dy);
}

/* METHOD
 * drawLines :: (...) -> ()
 */
static void
canvasCDrawLines (struct Canvas *self, const struct Tuple *args)
{
  for (uint32_t i = 0; i < args->count - 3; i += 4)
    {
      if (args->list[i + 0].type != t_uint32)
        continue;
      if (args->list[i + 1].type != t_uint32)
        continue;
      if (args->list[i + 2].type != t_int32)
        continue;
      if (args->list[i + 3].type != t_int32)
        continue;
      canvasDrawLine (self,
                      args->list[i + 0].uint32, args->list[i + 1].uint32,
                      args->list[i + 2].int32,  args->list[i + 3].int32
                      );
    }
}

/* METHOD
 * swapBuffers :: () -> ()
 */
void
canvasSwapBuffers (struct Canvas *self)
{
  register struct Buffer *t = self -> front;
  painterDestroy (self -> painter);
  self -> front = self -> back;
  self -> back = t;
  self -> painter = bufferGetPainter (self -> back);
  widgetRerender (canvasToWidget (self), NULL);
}

/* METHOD
 * requestSize :: (int32, int32) -> ()
 */
void
canvasRequestSize (struct Canvas *self, int32_t reqWidth, int32_t reqHeight)
{
  self -> widget.reqWidth = reqWidth;
  self -> widget.reqHeight = reqHeight;
  widgetReconfigure (canvasToWidget (self));
}

/* arch-tag: 47bb0c50-39ce-4ff0-aca8-138b0ce5922f
 */
