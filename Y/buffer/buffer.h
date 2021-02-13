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

#ifndef Y_BUFFER_H
#define Y_BUFFER_H

struct Buffer;

#include <Y/y.h>
#include <Y/util/rectangle.h>
#include <Y/screen/renderer.h>

#include <stdint.h>

/** \brief Create a new, empty,  buffer.
 */
void bufferDestroy (struct Buffer *);

/** \brief Get the buffer's unique ID
 */
uint32_t bufferGetID (const struct Buffer *);

void bufferGetSize (struct Buffer *, int *w_p, int *h_p);
void bufferSetSize (struct Buffer *, int w, int h);

/** \brief Obtain a Painter for this buffer.
 */
struct Painter * bufferGetPainter  (struct Buffer *);

/** \brief Render this buffer to the given renderer.
 */
void bufferRender (struct Buffer *, struct Renderer *,
                   int x, int y);

/** \brief Draw this buffer onto the destination Painter
 */
void bufferDrawOnto (struct Buffer *, struct Painter *,
                     int xo, int yo, int x, int y, int w, int h);

/**
 * BufferContexts are a way for video drivers to store contextual
 * information about buffers.  Each video driver should obtain
 * one context ID from the bufferNewContextID function, and use
 * that ID to store its information in each buffer as and when it
 * encounters them.
 */
struct BufferContext
{
  uint32_t id;
  struct Buffer *buffer;
  void (*modified) (struct BufferContext *);
  void (*destroy) (struct BufferContext *);
};

uint32_t bufferNewContextID (void);

void bufferAddContext (struct Buffer *, struct BufferContext *);
struct BufferContext *bufferGetContext (struct Buffer *, uint32_t);
void bufferRemoveContext (struct Buffer *, uint32_t);

#endif

/* arch-tag: f8234149-a49d-49eb-a609-088a060981f2
 */
