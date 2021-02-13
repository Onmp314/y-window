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

#include <Y/buffer/buffer.h>
#include <Y/buffer/bufferclass.h>

#include <stdlib.h>

static uint32_t bufferNextID = 1;

static int
buffercontextKeyFunction (const void *key_v, const void *obj_v)
{
  const uint32_t *key = key_v;
  const struct BufferContext *obj = obj_v;
  return *key - obj->id;
}

static int
buffercontextComparisonFunction (const void *obj1_v, const void *obj2_v)
{
  const struct BufferContext *obj1 = obj1_v;
  const struct BufferContext *obj2 = obj2_v;
  return obj1->id - obj2->id;
}

static void
buffercontextDestructorFunction (void *obj_v)
{
  struct BufferContext *obj = obj_v;
  obj->destroy (obj);
}

void
bufferInitialise (struct Buffer *self, struct BufferClass *c)
{
  self->id = bufferNextID++;
  self->c = c;
  self->width = 0;
  self->height = 0;
  self->contexts = indexCreate (buffercontextKeyFunction,
                                buffercontextComparisonFunction);
}

void
bufferFinalise (struct Buffer *self)
{
  indexDestroy (self->contexts, buffercontextDestructorFunction);
}

void
bufferDestroy (struct Buffer *self)
{
  self->c->destroy (self);
}

uint32_t
bufferGetID (const struct Buffer *self)
{
  if (self == NULL)
    return 0;
  return self->id;
}

void
bufferGetSize (struct Buffer *self, int *w_p, int *h_p)
{
  if (w_p != NULL)
    *w_p = self->width;
  if (h_p != NULL)
    *h_p = self->height;
}

void
bufferSetSize (struct Buffer *self, int w, int h)
{
  self->c->setSize (self, w, h);
}

struct Painter *
bufferGetPainter (struct Buffer *self)
{
  return self->c->getPainter (self);
}

void
bufferRender (struct Buffer *self, struct Renderer *renderer,
              int x, int y)
{
  if (rendererRenderBuffer (renderer, self, x, y) == false)
    self->c->render (self, renderer, x, y);
}

static uint32_t bufferNextContextID = 1;

uint32_t
bufferNewContextID (void)
{
  return bufferNextContextID++;
}

void
bufferAddContext (struct Buffer *self, struct BufferContext *context)
{
  struct BufferContext *oldContext;

  oldContext = indexRemove (self->contexts, &(context->id));
  if (oldContext != NULL && oldContext != context)
    oldContext->destroy (oldContext);
  indexAdd (self->contexts, context);
}

struct BufferContext *
bufferGetContext (struct Buffer *self, uint32_t id)
{
  return indexFind (self->contexts, &id);
}

void
bufferRemoveContext (struct Buffer *self, uint32_t id)
{
  struct BufferContext *context;

  context = indexRemove (self->contexts, &id);
  if (context != NULL)
    context->destroy (context);
}

void
bufferNotifyModified (struct Buffer *self)
{
  struct IndexIterator *iter = indexGetStartIterator (self->contexts);
  while (indexiteratorHasValue (iter))
    {
      struct BufferContext *context = indexiteratorGet (iter);
      context->modified (context);
      indexiteratorNext (iter);
    }
  indexiteratorDestroy (iter);
}

void
bufferDrawOnto (struct Buffer *self, struct Painter *dest, int xo, int yo,
                int x, int y, int w, int h)
{
  self->c->drawOnto (self, dest, xo, yo, x, y, w, h); 
}

/* arch-tag: 37250df8-9065-43f2-899b-c69e72bb3d11
 */
