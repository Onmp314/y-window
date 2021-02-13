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

#include <Y/screen/swrenderer.h>
#include <Y/screen/rendererclass.h>
#include <Y/modules/videodriver_interface.h>
#include <Y/util/yutil.h>
#include <Y/util/colour.h>

static void swrendererComplete (struct Renderer *);
static void swrendererDestroy (struct Renderer *);
static void swrendererBlitRGBAData (struct Renderer *, int, int,
                                    const uint32_t *, int, int, int);
static void swrendererDrawFilledRectangle (struct Renderer *, uint32_t,
                                           int, int, int, int);

struct SWRenderer
{
  struct Renderer renderer;
  struct VideoDriver *video;
  int x, y, w, h;
  uint32_t *data;
};

struct RendererClass swrendererClass =
{
  name:                "SWRenderer",
  complete:            swrendererComplete,
  destroy:             swrendererDestroy,
  renderBuffer:        NULL,
  blitRGBAData:        swrendererBlitRGBAData,
  drawFilledRectangle: swrendererDrawFilledRectangle
};

static inline struct SWRenderer *
castBack (struct Renderer *self_r)
{
  /* assert ( self_r -> c == swrendererClass ); */
  return (struct SWRenderer *)self_r;
}

struct SWRenderer *
swrendererCreate (struct VideoDriver *video, const struct Rectangle *rect)
{
  struct SWRenderer *self = ymalloc (sizeof (struct SWRenderer));
  self -> renderer.c = &swrendererClass;
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
swrendererDestroy (struct Renderer *self_r)
{
  struct SWRenderer *self = castBack (self_r);
  yfree (self -> data);
  yfree (self);
}

struct Renderer *
swrendererGetRenderer (struct SWRenderer *self)
{
  return &(self -> renderer);
}

void
swrendererBlitRGBAData (struct Renderer *self_r, int x, int y,
                        const uint32_t *data, int w, int h, int s)
{
  struct SWRenderer *self = castBack (self_r);
  int i, j;
  for (j=0; j < h; ++j)
    {
      uint32_t *toLine = self -> data + self -> w * (y + j - self -> y)
                                          + (x - self -> x);
      const uint32_t *frLine = data + s * j;
      for (i=0; i < w; ++i)
        {
          colourBlendSourceOver (toLine, *toLine, *frLine, 0xFF);
          ++toLine; ++frLine;
        }
    }
}

void
swrendererDrawFilledRectangle (struct Renderer *self_r, uint32_t colour,
                               int x, int y, int w, int h)
{
  struct SWRenderer *self = castBack (self_r);
  int i, j;
  if ((colour & 0xFF000000) == 0xFF000000)
    {
      for (j=0; j<h; ++j)
        {
          uint32_t *line = self -> data + self -> w * (y + j - self -> y)
                                            + (x - self -> x);
          for (i=0; i<w; ++i)
            *(line++) = colour;
        }
    }
  else
    {
      for (j=0; j<h; ++j)
        {
          uint32_t *line = self -> data + self -> w * (y + j - self -> y)
                                            + (x - self -> x);
          for (i=0; i<w; ++i)
            {
              colourBlendSourceOver (line, *line, colour, 0xFF);
              ++line;
            }
        }
    }
}

void
swrendererComplete (struct Renderer *self_r)
{
  struct SWRenderer *self = castBack (self_r);
  self -> video -> blit (self -> video, self -> data,
                         self -> x, self -> y, self -> w, self -> h, self -> w);
}

/* arch-tag: d1a749d1-8e4f-4837-8a4e-8920bb63877d
 */
