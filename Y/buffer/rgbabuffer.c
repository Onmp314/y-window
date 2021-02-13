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

#include <Y/buffer/rgbabuffer.h>
#include <Y/buffer/bufferclass.h>
#include <Y/buffer/painterclass.h>
#include <Y/util/yutil.h>
#include <Y/util/colour.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/* the minimum size of data in memory */
#define RGBABUFFER_MIN_SIZE  64

struct RGBABuffer
{
  struct Buffer buffer;
  int dataWidth;
  int dataHeight;
  uint32_t *data;
};


static void rgbabufferBDestroy    (struct Buffer *);
static void rgbabufferBSetSize    (struct Buffer *, int w, int h);
static struct Painter *
            rgbabufferBGetPainter (struct Buffer *);
static void rgbabufferBRender     (struct Buffer *, struct Renderer *,
                                   int x, int y);
static void rgbabufferBDrawOnto   (struct Buffer *, struct Painter *,
                                   int xo, int yo, int x, int y, int w, int h);

static void rgbabufferPClearRectangle (struct Painter *,
                                       int x, int y, int w, int h);
static void rgbabufferPDrawRectangle  (struct Painter *,
                                       int x, int y, int w, int h);
static void rgbabufferPDrawHLine      (struct Painter *,
                                       int x, int y, int dx);
static void rgbabufferPDrawVLine      (struct Painter *,
                                       int x, int y, int dy);
static void rgbabufferPDrawLine       (struct Painter *,
                                       int x, int y, int dx, int dy);
static void rgbabufferPDrawAlphamap   (struct Painter *painter,
                                       uint8_t *alpha,
                                       int x, int y, int w, int h, int s);
static void rgbabufferPDrawRGBAData   (struct Painter *painter,
                                       uint32_t *data,
                                       int x, int y, int w, int h, int s);

static struct PainterClass rgbabufferPainterClass =
{
  name:             "RGBABuffer/Painter",
  clearRectangle:   rgbabufferPClearRectangle,
  drawRectangle:    rgbabufferPDrawRectangle,
  drawHLine:        rgbabufferPDrawHLine,
  drawVLine:        rgbabufferPDrawVLine,
  drawLine:         rgbabufferPDrawLine,
  drawAlphamap:     rgbabufferPDrawAlphamap,
  drawRGBAData:     rgbabufferPDrawRGBAData
};

static struct BufferClass rgbabufferBufferClass =
{
  name:             "RGBABuffer",
  destroy:          rgbabufferBDestroy,
  setSize:          rgbabufferBSetSize,
  getPainter:       rgbabufferBGetPainter,
  render:           rgbabufferBRender,
  drawOnto:         rgbabufferBDrawOnto
};

struct RGBABuffer *
rgbabufferCreate ()
{
  struct RGBABuffer * self = ymalloc (sizeof (struct RGBABuffer));
  bufferInitialise (&(self->buffer), &rgbabufferBufferClass);
  self->dataWidth = RGBABUFFER_MIN_SIZE;
  self->dataHeight = RGBABUFFER_MIN_SIZE;
  self->data = ymalloc (self->dataWidth * self->dataHeight * sizeof (uint32_t));
  return self;
}

struct RGBABuffer *
rgbabufferCreateFromData (int w, int h, int dw, int dh, uint32_t *data)
{
  struct RGBABuffer * self = ymalloc (sizeof (struct RGBABuffer));
  bufferInitialise (&(self->buffer), &rgbabufferBufferClass);
  self->buffer.width = w;
  self->buffer.height = h;
  self->dataWidth = dw;
  self->dataHeight = dh;
  self->data = data;
  return self;
}

void
rgbabufferAccessInternals (struct RGBABuffer *self, int *dw_p, int *dh_p,
                           uint32_t **data_p)
{
  if (dw_p != NULL)
    *dw_p = self->dataWidth;
  if (dh_p != NULL)
    *dh_p = self->dataHeight;
  if (data_p != NULL)
    *data_p = self->data;
}

void
rgbabufferDestroy (struct RGBABuffer *self)
{
  if (self)
    {
      bufferFinalise (&(self->buffer));
      yfree (self->data);
      yfree (self);
    }
}

void
rgbabufferBDestroy (struct Buffer *self_b)
{
  rgbabufferDestroy ((struct RGBABuffer *)self_b);
}

struct Buffer *
rgbabufferToBuffer (struct RGBABuffer *self)
{
  return &(self->buffer);
}

bool
bufferIsRGBABuffer (struct Buffer *self_b)
{
  return self_b->c == &rgbabufferBufferClass;
}

void
rgbabufferBSetSize (struct Buffer *self_b, int w, int h)
{
  struct RGBABuffer *self = (struct RGBABuffer *)self_b;
  int dw = RGBABUFFER_MIN_SIZE;
  int dh = RGBABUFFER_MIN_SIZE;

  if (self->buffer.width == w && self->buffer.height == h)
    return;

  bufferNotifyModified (&(self->buffer));

  while (dw < w)
    dw *= 2;
  while (dh < h)
    dh *= 2;
  if (dh != self->dataHeight || dw != self->dataWidth)
    {
      yfree (self->data);
      self->data = ymalloc (dw * dh * sizeof(uint32_t));
      memset(self->data, 0, dw * dh * sizeof(uint32_t));
      self->dataWidth = dw;
      self->dataHeight = dh;
    }
  self->buffer.width = w;
  self->buffer.height = h;
}

struct Painter *
rgbabufferBGetPainter (struct Buffer *self_b)
{
  struct RGBABuffer *self = (struct RGBABuffer *)self_b;
  struct Painter *painter = painterCreate ();
  struct Rectangle r = { 0, 0, self->buffer.width, self->buffer.height };
  painter->c = &rgbabufferPainterClass;
  painter->buffer = &(self->buffer);
  painterClipTo (painter, &r); 
  return painter;
}

void
rgbabufferBRender (struct Buffer *self_b, struct Renderer *renderer,
                   int x, int y)
{
  struct RGBABuffer *self = (struct RGBABuffer *)self_b;
  rendererBlitRGBAData (renderer, x, y, self->data,
                        self->buffer.width, self->buffer.height,
                        self->dataWidth);
}

void
rgbabufferBDrawOnto (struct Buffer *self_b, struct Painter *painter,
                     int xo, int yo, int x, int y, int w, int h)
{
  struct RGBABuffer *self = (struct RGBABuffer *)self_b;
  int destWidth = w, destHeight = h;
  uint32_t *data_start;

  if (destWidth > self->buffer.width - xo)
    destWidth = self->buffer.width - xo;
  if (destHeight > self->buffer.height - yo)
    destHeight = self->buffer.height - yo;

  data_start = self->data + yo * self->dataWidth + xo;
  painterDrawRGBAData (painter, data_start, x, y, destWidth, destHeight,
                       self->dataWidth);
}
                     

void
rgbabufferPClearRectangle (struct Painter *painter, int x, int y, int w, int h)
{
  struct RGBABuffer *self = (struct RGBABuffer *)painter->buffer;
  /* clear the rectangle to the fill colour */
  uint32_t *line;
  int i, j;
  painterClipCoordinates (painter, &x, &y, &w, &h);
  line = self->data + self->dataWidth * y + x;
  for (j = 0; j < h; ++j)
    { 
      uint32_t *data = line;
      for (i = 0; i < w; ++i, ++data)
        *data = painter->state->fillColour;
      line += self->dataWidth;
    }

  bufferNotifyModified (&(self->buffer));
}

void
rgbabufferPDrawRectangle (struct Painter *painter, int x, int y, int w, int h)
{
  struct RGBABuffer *self = (struct RGBABuffer *)painter->buffer;
  /* fill the rectangle with the fill colour, and stroke with the pen colour */
  int xc = x, yc = y, wc = w, hc = h;
  uint32_t *line;
  int i, j;
  int strokeLeft = 1, strokeRight = 1, strokeTop = 1, strokeBottom = 1;
  if (painterClipCoordinates (painter, &xc, &yc, &wc, &hc))
    {
      if (xc != x)
        strokeLeft = 0;
      if (yc != y)
        strokeTop = 0;
      if (wc != (x + w - xc))
        strokeRight = 0;
      if (hc != (y + h - yc))
        strokeBottom = 0;
    }

  line = self->data + self->dataWidth * yc + xc;
  for (j=0 ; j < hc; ++j)
    {
      uint32_t colourLeft   = strokeLeft ? painter->state->penColour
                                         : painter->state->fillColour; 
      uint32_t colourCentre = painter->state->fillColour;
      uint32_t colourRight  = strokeRight ? painter->state->penColour
                                          : painter->state->fillColour; 
      uint32_t *data = line;
      if ((j == 0 && strokeTop) || (j == hc-1 && strokeBottom))
        colourLeft = colourRight = colourCentre = painter->state->penColour;

      if (w > 0)
        {
          colourBlend (data, *data, colourLeft,
                       0xFF, painter->state->blendMode);
          ++data;
        }
      if (colourCentre & 0xFF000000)
        {
          for (i = 1; i < wc-1; ++i, ++data)
          colourBlend (data, *data, colourCentre,
                       0xFF, painter->state->blendMode);
        }
      else
        {
          data += wc-2;
        }
      if (w > 1)
        {
          colourBlend (data, *data, colourRight,
                       0xFF, painter->state->blendMode);
          ++data;
        }
      line += self->dataWidth;
    }

  bufferNotifyModified (&(self->buffer));
}

void
rgbabufferPDrawHLine (struct Painter *painter, int x, int y, int dx)
{
  struct RGBABuffer *self = (struct RGBABuffer *)painter->buffer;
  uint32_t *data;
  int i;
  int hc = 1;

  painterClipCoordinates (painter, &x, &y, &dx, &hc);

  if (hc == 0)
    return;
 
  data = self->data + self->dataWidth * y + x;

  for (i=0; i<dx; ++i)
    {
      colourBlend (data, *data, painter->state->penColour,
                   0xFF, painter->state->blendMode);
      ++data;
    }

  bufferNotifyModified (&(self->buffer));
}

void
rgbabufferPDrawVLine (struct Painter *painter, int x, int y, int dy)
{
  struct RGBABuffer *self = (struct RGBABuffer *)painter->buffer;
  uint32_t *data;
  int i;
  int wc = 1;

  painterClipCoordinates (painter, &x, &y, &wc, &dy);

  if (wc == 0)
    return;
 
  data = self->data + self->dataWidth * y + x;

  for (i=0; i < dy; ++i)
    {
      colourBlend (data, *data, painter->state->penColour,
                   0xFF, painter->state->blendMode);
      data += self->dataWidth;
    }

  bufferNotifyModified (&(self->buffer));
}

void
rgbabufferPDrawLine (struct Painter *painter, int x, int y, int dx, int dy)
{
  struct RGBABuffer *self = (struct RGBABuffer *)painter->buffer;
  uint32_t *data = self->data + self->dataWidth * y + x;
  int dirx = dx < 0 ? -1 : 1;
  int diry = dy < 0 ? -1 : 1;
  int xc, yc, wc, hc;
  int xp = x;
  int yp = y;

  xc = dx < 0 ? x + dx : x;
  yc = dy < 0 ? y + dy : y;

  /* remove the sign from dx and dy */
  dx *= dirx; 
  dy *= diry;

  wc = dx;
  hc = dy;

  painterClipCoordinates (painter, &xc, &yc, &wc, &hc);

  /* Bresenham's integer line drawing algorithm, modified to be
   * insensitive to the sign.
   */
  if (dx >= dy)
    {
      /* nearly horizontal line */
      int er = -dx / 2;
      int i;
      for (i = 0; i < dx; ++i)
        {
	  if (xp >= xc && xp <= xc + wc &&
	      yp >= yc && yp <= yc + hc)
            colourBlend (data, *data, painter->state->penColour,
                         0xFF, painter->state->blendMode);
          er += dy;
          if (er >= 0)
            {
              data += self->dataWidth * diry;
              er -= dx;
	      yp += diry;
            }
          data += dirx;
	  xp += dirx;
        }
    }
  else
    {
      /* nearly vertical line */
      int er = -dy / 2;
      int i;
      for (i = 0; i < dy; ++i)
        {
	  if (xp >= xc && xp <= xc + wc &&
	      yp >= yc && yp <= yc + hc)
            colourBlend (data, *data, painter->state->penColour,
                         0xFF, painter->state->blendMode);
          er += dx;
          if (er >= 0)
            {
              data += dirx; 
              er -= dy;
	      xp += dirx;
            }
          data += self->dataWidth * diry;
	  yp += diry;
        }
    }

  bufferNotifyModified (&(self->buffer));
}

void
rgbabufferPDrawAlphamap (struct Painter *painter, uint8_t *alpha,
                           int x, int y, int w, int h, int s)
{
  struct RGBABuffer *self = (struct RGBABuffer *)painter->buffer;
  uint32_t *dline;
  uint8_t *sline;
  int i,j;
  int xc = x, yc = y, wc = w, hc = h;
 
  painterClipCoordinates (painter, &xc, &yc, &wc, &hc);

  dline = self->data + yc * self->dataWidth + xc;
  sline = alpha + (yc - y) * s + (xc - x);
 
  for (j=0; j<hc; ++j)
    {
      uint32_t *dest = dline;
      uint8_t *src = sline;
      for (i=0; i<wc; ++i)
        {
          colourBlend (dest, *dest, painter->state->penColour,
                       ((uint32_t)*src), painter->state->blendMode);
          ++dest;
          ++src;
        }
      dline += self->dataWidth;
      sline += s;
    }

  bufferNotifyModified (&(self->buffer));
}

void
rgbabufferPDrawRGBAData (struct Painter *painter, uint32_t *data,
                         int x, int y, int w, int h, int s)
{
  struct RGBABuffer *self = (struct RGBABuffer *)painter->buffer;
  uint32_t *dline;
  uint32_t *sline;
  int i,j;
  int xc = x, yc = y, wc = w, hc = h;
 
  painterClipCoordinates (painter, &xc, &yc, &wc, &hc);

  dline = self->data + yc * self->dataWidth + xc;
  sline = data + (yc - y) * s + (xc - x);
 
  for (j=0; j<hc; ++j)
    {
      uint32_t *dest = dline;
      uint32_t *src = sline;
      for (i=0; i<wc; ++i)
        {
          colourBlend (dest, *dest, *src, 0xFF,
                       painter->state->blendMode);
          ++dest;
          ++src;
        }
      dline += self->dataWidth;
      sline += s;
    }

  bufferNotifyModified (&(self->buffer));
}

/* arch-tag: e6e1f08b-4110-49e3-83cd-4c58597305fd
 */
