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

#include <Y/screen/hwrenderer.h>
#include <Y/screen/rendererclass.h>
#include <Y/modules/videodriver_interface.h>
#include <Y/util/yutil.h>
#include <Y/util/colour.h>

static void hwrendererComplete (struct Renderer *);
static void hwrendererDestroy (struct Renderer *);
static bool hwrendererRenderBuffer (struct Renderer *, struct Buffer *,
                                    int, int, int, int, int, int);
static void hwrendererBlitRGBAData (struct Renderer *, int, int,
                                    const uint32_t *, int, int, int);
static void hwrendererDrawFilledRectangle (struct Renderer *, uint32_t,
                                           int, int, int, int);

struct HWRenderer
{
  struct Renderer renderer;
  const struct HWRendererAccel *accel;
  struct VideoDriver *video;
  int x, y, w, h;
};

struct RendererClass hwrendererClass =
{
  name:                "HWRenderer",
  complete:            hwrendererComplete,
  destroy:             hwrendererDestroy,
  renderBuffer:        hwrendererRenderBuffer,
  blitRGBAData:        hwrendererBlitRGBAData,
  drawFilledRectangle: hwrendererDrawFilledRectangle
};

static inline struct HWRenderer *
castBack (struct Renderer *self_r)
{
  /* assert ( self_r instanceof hwrendererClass ); */
  return (struct HWRenderer *)self_r;
}

struct HWRenderer *
hwrendererCreate (struct VideoDriver *video, const struct Rectangle *rect,
                  const struct HWRendererAccel *accel)
{
  struct HWRenderer *self = ymalloc (sizeof (struct HWRenderer));
  self -> renderer.c = &hwrendererClass;
  rendererInitialise (&(self -> renderer));
  self -> accel = accel;
  self -> video = video;
  self -> x = rect -> x;
  self -> y = rect -> y;
  self -> w = rect -> w;
  self -> h = rect -> h;
  rendererEnter (&(self -> renderer), rect, 0, 0);
  return self;
}

static void
hwrendererDestroy (struct Renderer *self_r)
{
  struct HWRenderer *self = castBack (self_r);
  yfree (self);
}

struct Renderer *
hwrendererGetRenderer (struct HWRenderer *self)
{
  return &(self -> renderer);
}

bool
hwrendererRenderBuffer (struct Renderer *self_r, struct Buffer *buffer,
                        int x, int y, int xo, int yo, int rw, int rh)
{
  struct HWRenderer *self = castBack (self_r);
  return self->accel->renderBuffer (self->video, buffer, x, y, xo, yo, rw, rh);
}

void
hwrendererBlitRGBAData (struct Renderer *self_r, int x, int y,
                        const uint32_t *data, int w, int h, int s)
{
  struct HWRenderer *self = castBack (self_r);
  self->accel->blitRGBAData (self->video, x, y, data, w, h, s);
}

void
hwrendererDrawFilledRectangle (struct Renderer *self_r, uint32_t colour,
                               int x, int y, int w, int h)
{
  struct HWRenderer *self = castBack (self_r);
  self->accel->drawFilledRectangle (self->video, colour, x, y, w, h);
}

void
hwrendererComplete (struct Renderer *self_r)
{
}

/* arch-tag: 362dff10-171c-49f2-bad5-f241dd7feed9
 */
