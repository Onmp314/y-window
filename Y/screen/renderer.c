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

#include <Y/screen/renderer.h>
#include <Y/screen/rendererclass.h>

#include <Y/util/yutil.h>
#include <Y/util/index.h>

#include <stdlib.h>
#include <string.h>

struct RenderRegion
{
  struct Rectangle clip;
  int translateX, translateY;
};

struct RendererOption
{
  char *key;
  char *value;
};

static int
rendererOptionKeyFunction (const void *key_v, const void *obj_v)
{
  const char *key = key_v;
  const struct RendererOption *obj = obj_v;
  return strcmp(key, obj->key);
}

static int
rendererOptionComparisonFunction (const void *obj1_v, const void *obj2_v)
{
  const struct RendererOption *obj1 = obj1_v;
  const struct RendererOption *obj2 = obj2_v;
  return strcmp(obj1->key, obj2->key);
}

static struct RendererOption *
rendererOptionCreate (const char *key, const char *value)
{
  struct RendererOption *obj = ymalloc(sizeof(*obj));
  obj->key = ystrdup(key);
  obj->value = ystrdup(value);
  return obj;
}

static void
rendererOptionDestroy (void *obj_v)
{
  struct RendererOption *obj = obj_v;
  yfree(obj->key);
  yfree(obj->value);
  yfree(obj);
}

void
rendererInitialise (struct Renderer *self)
{
  self -> regions = new_llist ();
  self -> options = indexCreate (rendererOptionKeyFunction, rendererOptionComparisonFunction);
}

void
rendererComplete (struct Renderer *self)
{
  if (self != NULL)
    self -> c -> complete (self);
}

void
rendererDestroy (struct Renderer *self)
{
  if (self != NULL)
    {
      llist_destroy (self -> regions, yfree);
      indexDestroy (self -> options, rendererOptionDestroy);
      self -> c -> destroy (self);
    }
}

int
rendererEnter (struct Renderer *self, const struct Rectangle *rect,
               int dx, int dy)
{
  struct RenderRegion *reg = ymalloc (sizeof (struct RenderRegion));
  struct RenderRegion *prev = llist_node_data (llist_head (self -> regions));
  reg -> clip.x = rect -> x;
  reg -> clip.y = rect -> y;
  reg -> clip.w = rect -> w;
  reg -> clip.h = rect -> h;
  reg -> translateX = dx;
  reg -> translateY = dy;
  if (prev != NULL)
    {
      reg -> clip.x += prev -> translateX;
      reg -> clip.y += prev -> translateY;
      reg -> translateX += prev -> translateX;
      reg -> translateY += prev -> translateY;
      if (!rectangleIntersect (&(reg -> clip), &(reg -> clip),
                               &(prev -> clip)))
         {
           yfree (reg);
           return 0;
         }
    }
  llist_add_head (self -> regions, reg);
  return 1;
}

void
rendererLeave (struct Renderer *self)
{
  if (self != NULL)
    {
      if (llist_length (self -> regions) != 0)
        {
          struct llist_node *head = llist_head (self->regions);
          struct RenderRegion *r = llist_node_data (head);
          llist_delete_node (head);
          yfree (r);
        }
    }
}

bool
rendererRenderBuffer (struct Renderer *self, struct Buffer *buffer,
                      int x, int y)
{
  struct Rectangle r = { x, y, 0, 0 };
  struct Rectangle r2;
  bufferGetSize (buffer, &r.w, &r.h);
  if (self != NULL)
    {
      struct RenderRegion *reg = llist_node_data (llist_head (self->regions));
      if (reg != NULL)
        {
          r.x += reg->translateX;
          r.y += reg->translateY;
          if (!rectangleIntersect (&r2, &r, &(reg -> clip)))
            return true;
        }
      if (self->c->renderBuffer != NULL)
        return self->c->renderBuffer (self, buffer, r.x, r.y,
               r2.x-r.x, r2.y-r.y, r2.w, r2.h);
      else
        return false;
    }
  return false;
}

void
rendererBlitRGBAData (struct Renderer *self, int x, int y, const uint32_t *data,
                      int w, int h, int s)
{
  struct Rectangle r = { x, y, w, h };
  struct Rectangle r2;
  if (self != NULL)
    {
      struct RenderRegion *reg = llist_node_data (llist_head (self->regions));
      if (reg != NULL)
        {
          r.x += reg -> translateX;
          r.y += reg -> translateY;
          if (!rectangleIntersect (&r2, &r, &(reg -> clip)))
            return;
        }
      self -> c -> blitRGBAData (self, r2.x, r2.y, 
                                 data + (r2.x - r.x) + s * (r2.y - r.y),
                                 r2.w, r2.h, s);
    }
}

void
rendererDrawFilledRectangle (struct Renderer *self, uint32_t colour,
                             int x, int y, int w, int h)
{
  struct Rectangle r = { x, y, w, h };
  if (self != NULL)
    {
      struct RenderRegion *reg = llist_node_data (llist_head (self->regions));
      if (reg != NULL)
        {
          r.x += reg -> translateX;
          r.y += reg -> translateY;
          rectangleIntersect (&r, &r, &(reg -> clip));
        }
      self -> c -> drawFilledRectangle (self, colour, r.x, r.y, r.w, r.h);
    }
}

void
rendererSetOption (struct Renderer *self, const char *key, const char *value)
{
  if (!key)
    return;

  void *obj = indexFind(self -> options, key);

  if (obj)
    rendererOptionDestroy(obj);

  if (!value)
    return;

  obj = rendererOptionCreate(key, value);

  indexAdd(self -> options, obj);
}

const char *
rendererGetOption (struct Renderer *self, const char *key)
{
  struct RendererOption *obj = indexFind(self -> options, key);

  if (!obj)
    return NULL;

  return obj->value;
}

/* arch-tag: 08567273-861d-4d25-b7b2-97ce1cd993dc
 */
