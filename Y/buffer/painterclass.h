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

#ifndef Y_BUFFER_PAINTERCLASS_H
#define Y_BUFFER_PAINTERCLASS_H

struct PainterClass;

#include <Y/y.h>
#include <Y/buffer/painter.h>
#include <Y/util/colour.h>

struct PainterState
{
  enum ColourBlendMode blendMode;
  uint32_t penColour, fillColour;
  double scaleX, scaleY, translateX, translateY;
  int clipping;
  struct Rectangle clipRectangle;

  struct PainterState *previous;
};

struct Painter
{
  struct PainterClass *c;
  struct Buffer *buffer;
  struct PainterState *state;
};

struct PainterClass
{
  const char *name;
  void (*clearRectangle) (struct Painter *, int x, int y, int w, int h);
  void (*drawRectangle)  (struct Painter *, int x, int y, int w, int h);
  void (*drawHLine)      (struct Painter *, int x, int y, int dx);
  void (*drawVLine)      (struct Painter *, int x, int y, int dy);
  void (*drawLine)       (struct Painter *, int x, int y, int dx, int dy);
  void (*drawAlphamap)   (struct Painter *, uint8_t *alpha,
                          int x, int y, int w, int h, int s);
  void (*drawRGBAData) (struct Painter *, uint32_t *data,
                        int x, int y, int w, int h, int s);
};

/** \brief Create an empty painter for subclass population
 * \internal this is the wrong way to do it. FIXME
 */
struct Painter *painterCreate (void);

#endif /* Y_BUFFER_PAINTERCLASS_H */

/* arch-tag: a885a98d-30d8-47ae-960e-c48837859d1f
 */
