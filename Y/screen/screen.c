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

#include <Y/object/class.h>
#include <Y/screen/screen.h>
#include <Y/util/yutil.h>
#include <Y/util/index.h>

#include <stdio.h>
#include <string.h>
#include <assert.h>

static struct Index *viewports;
static struct Widget *rootWidget = NULL;
static struct Rectangle *screenRectangle = NULL;

DEFINE_CLASS(Screen);
#include "Screen.yc"

static int
viewportsKeyFunction (const void *key_v, const void *obj_v)
{
  int key = *((const int *)key_v);
  const struct Viewport *obj = obj_v;
  int objKey = viewportGetID (obj);
  if (key == objKey)
    return 0;
  if (key < objKey)
    return -1;
  else
    return 1; 
}

static int
viewportsComparisonFunction (const void *obj1_v, const void *obj2_v)
{
  const struct Viewport *obj1 = obj1_v;
  const struct Viewport *obj2 = obj2_v;
  int obj1Key = viewportGetID (obj1);
  int obj2Key = viewportGetID (obj2);
  if (obj1Key == obj2Key)
    return 0;
  if (obj1Key < obj2Key)
    return -1;
  else
    return 1; 
}

/* METHOD
 * list :: () -> (...)
 */
struct Tuple *
viewportCList (void)
{
  struct Tuple *ret = tupleCreate(indexCount(viewports));
  int i = 0;
  struct IndexIterator *iter = indexGetStartIterator (viewports);
  while (indexiteratorHasValue (iter))
    {
      struct Viewport *vp = indexiteratorGet (iter);
      const char *vpName = viewportGetName (vp);
      char buffer[strlen(vpName) + 20];
      snprintf (buffer, sizeof(buffer), "%d\t%s", viewportGetID (vp), vpName);
      ret->list[i++] = tb_string(buffer);
      indexiteratorNext (iter);
    }
  indexiteratorDestroy (iter);
  assert(i == indexCount(viewports));
  return ret;
}

/* METHOD
 * listResolutions :: (uint32) -> (...)
 */
struct Tuple *
viewportCListResolutions (const struct Tuple *args)
{
  uint32_t vpid = args->list[0].uint32;
  struct Viewport *vp = indexFind (viewports, &vpid);

  struct llist *resolutions = viewportGetResolutions (vp);
  struct Tuple *ret = tupleCreate(llist_length (resolutions));
  uint32_t i = 0;
  Y_TRACE ("Found %d resolutions", llist_length (resolutions));
  for (struct llist_node *node = llist_head (resolutions);
       node != NULL;
       node = llist_node_next (node))
    {
      struct VideoResolution *vres = llist_node_data (node);
      ret->list[i++] = tb_string(vres->name);
    }
  assert(i == llist_length (resolutions));
  free_llist (resolutions);
  return ret;
}

/* METHOD
 * setResolution :: (uint32, string) -> ()
 */
void
viewportCSetResolution (const struct Tuple *args)
{
  uint32_t vpid = args->list[0].uint32;
  struct Viewport *vp = indexFind (viewports, &vpid);

  const char *res = args->list[1].string.data;
  viewportSetResolution (vp, res);
}

/* METHOD
 * call :: (uint32, ...) -> (...)
 */
struct Tuple *
viewportCCall (const struct Tuple *args)
{
  uint32_t vpid = args->list[0].uint32;
  struct Viewport *vp = indexFind (viewports, &vpid);

  const struct Tuple vargs = {.count = args->count - 1, .list = args->list + 1};
  return viewportCall (vp, &vargs);
}

void
screenInitialise ()
{
  viewports = indexCreate (viewportsKeyFunction, viewportsComparisonFunction);
  screenRectangle = rectangleCreate (0, 0, 800, 600);
}

void
screenRegisterViewport  (struct Viewport *v)
{
  indexAdd (viewports, v);
  screenViewportsChanged ();
  screenInvalidateRectangle (viewportGetRectangle (v));
}

void
screenUnregisterViewport(struct Viewport *v)
{
  indexRemove (viewports, v);
  screenViewportsChanged ();
}

void
screenConstrainPoint (int32_t *x_p, int32_t *y_p)
{
  /* constrians the point (x, y) to fall within the viewports */
  /* the point is moved the minimum distance possible */

  int32_t candidateX = *x_p;
  int32_t candidateY = *y_p;
  int64_t distance = -1;
  struct IndexIterator *iter;

  iter = indexGetStartIterator (viewports);
  while (indexiteratorHasValue (iter) && distance != 0)
    {
      struct Rectangle *vRect;
      int nx, ny, nd;
      vRect = viewportGetRectangle (indexiteratorGet (iter));
      if (*x_p < vRect -> x)
        nx = vRect -> x;
      else if (*x_p > vRect -> x + vRect -> w - 1)
        nx = vRect -> x + vRect -> w - 1;
      else
        nx = *x_p;
      if (*y_p < vRect -> y)
        ny = vRect -> y;
      else if (*y_p > vRect -> y + vRect -> h - 1)
        ny = vRect -> y + vRect -> h - 1;
      else
        ny = *y_p;
      nd = (nx - *x_p) * (nx - *x_p)  +  (ny - *y_p) * (ny - *y_p);
      if (distance < 0 || nd < distance)
        {
          candidateX = nx;
          candidateY = ny;
          distance = nd;
        }

      rectangleDestroy (vRect);
      indexiteratorNext (iter);      
    }
  indexiteratorDestroy (iter);

  *x_p = candidateX;
  *y_p = candidateY;
}

void
screenSetRootWidget (struct Widget *w)
{
  rootWidget = w;
  widgetMove (rootWidget, screenRectangle -> x, screenRectangle -> y);
  widgetResize (rootWidget, screenRectangle -> w, screenRectangle -> h);
}

struct Widget *
screenGetRootWidget ()
{
  return rootWidget;
}

void 
screenInvalidateRectangle (struct Rectangle *r)
{
  struct IndexIterator *iterator;

  /* for each viewport, call viewportInvalidateRectangle */
  iterator = indexGetStartIterator (viewports);
  while (indexiteratorHasValue (iterator))
    {
      viewportInvalidateRectangle (indexiteratorGet (iterator), r);
      indexiteratorNext (iterator);
    }
  indexiteratorDestroy (iterator);

  rectangleDestroy (r);
}

void
screenViewportsChanged ()
{
  /* determine the bounding box of all viewports */
  struct IndexIterator *iterator;
  struct Rectangle *newRectangle;

  if (indexCount (viewports) == 0)
    return;

  iterator = indexGetStartIterator (viewports);

  if (!indexiteratorHasValue (iterator))
    return;

  newRectangle = viewportGetRectangle (indexiteratorGet (iterator));

  indexiteratorNext (iterator);
  while (indexiteratorHasValue (iterator))
    {
      struct Rectangle *viewportRectangle;
      viewportRectangle = viewportGetRectangle (indexiteratorGet (iterator));
      rectangleUnion (newRectangle, newRectangle, viewportRectangle);
      rectangleDestroy (viewportRectangle);
      indexiteratorNext (iterator);
    }
  indexiteratorDestroy (iterator);

  rectangleDestroy (screenRectangle);
  screenRectangle = newRectangle;

  if (rootWidget)
    {
      widgetMove (rootWidget, screenRectangle -> x, screenRectangle -> y);
      widgetResize (rootWidget, screenRectangle -> w, screenRectangle -> h);
    }
}

void
screenUpdate ()
{
  struct IndexIterator *iterator;

  /* for each viewport, call viewportUpdate */
  iterator = indexGetStartIterator (viewports);
  while (indexiteratorHasValue (iterator))
    {
      viewportUpdate (indexiteratorGet (iterator));
      indexiteratorNext (iterator);
    }
  indexiteratorDestroy (iterator);

}

void
screenRender (struct Renderer *renderer)
{
  /* How to Render:
   * 1. Paint the rootWidget (and it's non-buffered children)
   *    directly onto the renderer.
   * 2. Render the widget tree (i.e. all the buffers) onto the
   *    renderer in ZOrder.
   * 3. Paint/Render the Overlay Widgets onto the Renderer
   *    in ZOrder.
   * 3. Paint the mouse pointer onto the renderer.
   */
#if 0
  struct Painter *painter= rendererGetPainter (renderer, rect);
#endif
  
  if (rootWidget != NULL)
    {
      struct Rectangle *rect = widgetGetRectangle (rootWidget);
      rendererEnter (renderer, rect, rect->x, rect->y);
#if 0
      widgetPaint (rootWidget, painter);
#endif
      widgetRender (rootWidget, renderer);
      rendererLeave (renderer);
      rectangleDestroy (rect);
    }
  else
    {
      rendererDrawFilledRectangle (renderer, 0xFF808080,
                                   screenRectangle->x, screenRectangle->y,
                                   screenRectangle->w, screenRectangle->h);
    }

  if (rendererGetOption(renderer, "hardware pointer") == NULL)
    {
      pointerRender (renderer);
    }
}

void
screenFinalise ()
{
  rectangleDestroy (screenRectangle);
  indexDestroy (viewports, NULL);
}

/* arch-tag: 7a770377-38cb-4326-8708-a4c8f5033528
 */
