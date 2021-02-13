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

#include <Y/screen/simplerenderer.h>
#include <Y/screen/rendererclass.h>
#include <Y/modules/videodriver_interface.h>
#include <Y/util/yutil.h>
#include <Y/util/colour.h>

static void simplerendererComplete (struct Renderer *);
static void simplerendererDestroy (struct Renderer *);
static void simplerendererBlitRGBAData (struct Renderer *, int, int,
                                        const uint32_t *, int, int, int);
static void simplerendererDrawFilledRectangle (struct Renderer *, uint32_t,
                                               int, int, int, int);

struct SimpleRenderer
{
  struct Renderer renderer;
  struct VideoDriver *video;
  int x, y, w, h;
  uint32_t *data;
};

struct RendererClass simplerendererClass =
{
  name:                "SimpleRenderer",
  complete:            simplerendererComplete,
  destroy:             simplerendererDestroy,
  renderBuffer:        NULL,
  blitRGBAData:        simplerendererBlitRGBAData,
  drawFilledRectangle: simplerendererDrawFilledRectangle
};

static inline struct SimpleRenderer *
castBack (struct Renderer *self_r)
{
  /* assert ( self_r -> c == simplerendererClass ); */
  return (struct SimpleRenderer *)self_r;
}

struct SimpleRenderer *
simplerendererCreate (struct VideoDriver *video, const struct Rectangle *rect)
{
  struct SimpleRenderer *self = ymalloc (sizeof (struct SimpleRenderer));
  self -> renderer.c = &simplerendererClass;
  rendererInitialise (&(self -> renderer));
  self -> video = video;
  self -> x = rect -> x;
  self -> y = rect -> y;
  self -> w = rect -> w;
  self -> h = rect -> h;
  self -> data = ymalloc (self -> w * self -> h * sizeof (uint32_t));
  rendererEnter (&(self -> renderer), rect, 0, 0);
  return self;
}

static void
simplerendererDestroy (struct Renderer *self_r)
{
  struct SimpleRenderer *self = castBack (self_r);
  yfree (self -> data);
  yfree (self);
}

struct Renderer *
simplerendererGetRenderer (struct SimpleRenderer *self)
{
  return &(self -> renderer);
}

static inline void
simpleColourBlendSourceOver (uint32_t *to, const uint32_t *from)
{
  if (*from >= 0xA0000000)
    *to = *from;
  else if (*from >= 0x40000000)
    *to = ((*to   & 0xFEFEFEFE) >> 1) +
          ((*from & 0xFEFEFEFE) >> 1);
}

void
simplerendererBlitRGBAData (struct Renderer *self_r, int x, int y,
                            const uint32_t *data, int w, int h, int s)
{
  struct SimpleRenderer *self = castBack (self_r);
  int i, j;
  for (j=0; j < h; ++j)
    {
      uint32_t *toLine = self -> data + self -> w * (y + j - self -> y)
                                          + (x - self -> x);
      const uint32_t *frLine = data + s * j;
      for (i=0; i < w; ++i)
        {
          simpleColourBlendSourceOver (toLine, frLine);
          ++toLine; ++frLine;
        }
    }
}

void
simplerendererDrawFilledRectangle (struct Renderer *self_r, uint32_t colour,
                               int x, int y, int w, int h)
{
  struct SimpleRenderer *self = castBack (self_r);
  int i, j;
  for (j=0; j<h; ++j)
    {
      uint32_t *line = self -> data + self -> w * (y + j - self -> y)
                                        + (x - self -> x);
      for (i=0; i<w; ++i)
        {
          simpleColourBlendSourceOver (line, &colour);
          ++line;
        }
    }
}

void
simplerendererComplete (struct Renderer *self_r)
{
  struct SimpleRenderer *self = castBack (self_r);
  self -> video -> blit (self -> video, self -> data,
                         self -> x, self -> y, self -> w, self -> h, self -> w);
}

/* arch-tag: 868e0f94-82a7-4601-8c31-d5f27bb79a49
 */
