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

#ifndef Y_BUFFERCLASS_H
#define Y_BUFFERCLASS_H

#include <Y/y.h>
#include <Y/buffer/buffer.h>
#include <Y/util/index.h>

#include <stdint.h>

struct BufferClass
{
  const char * name;

  void             (*destroy)    (struct Buffer *);
  void             (*setSize)    (struct Buffer *, int w, int h);
  struct Painter * (*getPainter) (struct Buffer *);
  void             (*render)     (struct Buffer *, struct Renderer *,
                                  int x, int y);
  void             (*drawOnto)   (struct Buffer *, struct Painter *,
                                  int xo, int yo, int x, int y, int w, int h);
};

struct Buffer
{
  struct BufferClass *c;
  uint32_t id;
  int width;
  int height;
  struct Index *contexts;
};

void bufferInitialise (struct Buffer *, struct BufferClass *);
void bufferFinalise (struct Buffer *);

/** \brief notify contexts that this buffer has been modified
 */
void bufferNotifyModified (struct Buffer *);


#endif

/* arch-tag: 5c5cf22a-fc70-4780-bea7-52f918245610
 */
