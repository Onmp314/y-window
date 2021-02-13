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

#include <Y/screen/viewport.h>
#include <Y/screen/screen.h>
#include <Y/screen/swrenderer.h>
#include <Y/main/control.h>
#include <Y/util/llist.h>
#include <Y/util/yutil.h>

#include <stdio.h>
#include <string.h>
#include <assert.h>

struct Viewport
{
  int id;
  struct VideoDriver *video;
  struct llist *invalidRectangles;
  int updateEventID;
  int x, y, w, h;
};

static int nextViewportID = 0;

struct Viewport *
viewportCreate (struct VideoDriver *video)
{
  struct Viewport *self = ymalloc (sizeof (struct Viewport));
  self -> id = nextViewportID++;
  self -> video = video;
  self -> invalidRectangles = new_llist ();
  self -> x = 0;
  self -> y = 0;
  video -> getPixelDimensions (video, &(self -> w), &(self -> h)); 
  if (self -> w <= 0)
    self -> w = 800;
  if (self -> h <= 0)
    self -> h = 600;
  self -> updateEventID = 0;

#if 0
  if (video -> setPointer)
    video -> setPointer (video, pointerArrow.pixel_data,
                         pointerArrow.width, pointerArrow.height,
                         pointerArrow.bytes_per_pixel,
                         0, 0);
#endif

  return self;
}

void
viewportDestroy (struct Viewport *self)
{
  llist_destroy (self->invalidRectangles, rectangleDestroy);
  yfree (self);
}

int
viewportGetID (const struct Viewport *self)
{
  return self -> id;
}

struct Rectangle *
viewportGetRectangle (struct Viewport *self)
{
  struct Rectangle *rect = ymalloc (sizeof (struct Rectangle));
  rect -> x = self -> x;
  rect -> y = self -> y;
  rect -> w = self -> w;
  rect -> h = self -> h;
  return rect;
}

void
viewportInvalidateRectangle (struct Viewport *self, const struct Rectangle *r)
{
  llist_add_tail (self->invalidRectangles, rectangleDuplicate (r));
  if (self -> updateEventID == 0)
    {
      self -> updateEventID =
        controlTimerDelay (0, 10, self, (void (*)(void *))viewportUpdate);
    }
}

void
viewportSetSize (struct Viewport *self, int w, int h)
{
  struct Rectangle r;
  r.x = self -> x;
  r.y = self -> y;
  r.w = self -> w = w;
  r.h = self -> h = h;
  screenViewportsChanged ();
  viewportInvalidateRectangle (self, &r);
}

struct Tuple *
viewportCall (struct Viewport *self, const struct Tuple *args)
{
  if (self != NULL && self -> video -> special != NULL)
    return self -> video -> special (self -> video, args);
  return NULL;
}

const char *
viewportGetName (struct Viewport *self)
{
  if (self -> video -> getName != NULL)
    return self -> video -> getName (self -> video);
  else
    return NULL;
}

struct llist *
viewportGetResolutions (struct Viewport *self)
{
  if (self -> video -> getResolutions != NULL)
    return self -> video -> getResolutions (self -> video);
  else
    return NULL;
}

void
viewportSetResolution (struct Viewport *self, const char *name)
{
  if (self -> video -> setResolution != NULL)
    self -> video -> setResolution (self -> video, name);
}

void
viewportUpdate (struct Viewport *self)
{
  struct Rectangle *viewportRectangle = viewportGetRectangle (self);

  if (llist_length (self -> invalidRectangles) == 0)
    return;

  rectanglelistUnionOverlaps (self -> invalidRectangles);

  self -> video -> beginUpdates (self -> video);

  /* for each rectangle in the collection */
  for (struct llist_node *node = llist_head (self->invalidRectangles);
       node != NULL;
       node = llist_node_next (node))
    {
      /* compute the intersected rectangle */
      struct Rectangle *invalid = llist_node_data (node);
      struct Rectangle *visible = ymalloc (sizeof (struct Rectangle));
      if (rectangleIntersect (visible, invalid, viewportRectangle))
        {
          /* create a renderer (visitor) and pass it over the widget structure
           */
          struct Renderer *renderer =
                  self -> video -> getRenderer (self -> video, visible);
          screenRender (renderer);
          rendererComplete (renderer);
          rendererDestroy (renderer);
        }
      rectangleDestroy (visible);
    }

  rectangleDestroy (viewportRectangle);

  self -> video -> endUpdates (self -> video);

  llist_destroy (self->invalidRectangles, rectangleDestroy);
  self -> invalidRectangles = new_llist ();

  self -> updateEventID = 0;

}

/* arch-tag: 24ab628b-2e9c-4bd2-9e84-f5428e71b764
 */
