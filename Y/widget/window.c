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

#include <Y/widget/window.h>
#include <Y/widget/widget_p.h>

#include <Y/util/yutil.h>
#include <Y/util/llist.h>
#include <Y/buffer/buffer.h>
#include <Y/buffer/rgbabuffer.h>
#include <Y/buffer/painter.h>
#include <Y/screen/screen.h>

#include <Y/object/class_p.h>
#include <Y/object/object_p.h>

#include <Y/modules/windowmanager.h>
#include <Y/modules/theme.h>

#include <Y/text/font.h>

#include <stdio.h>
#include <ctype.h>

struct Window
{
  struct Widget widget;
  struct Buffer *buffer;
  struct Widget *child;
  struct Widget *focus;
  enum WindowSizeState sizeState;
  int pointerInChild : 1;
  int storeX, storeY, storeW, storeH;
  int dragging, dragX, dragY;
  struct llist *invalidRectangles;
};

enum WindowAnchor
{
  WINDOW_ANCHOR_NONE   = 0,
  WINDOW_ANCHOR_LEFT   = 1<<0,
  WINDOW_ANCHOR_RIGHT  = 1<<1,
  WINDOW_ANCHOR_TOP    = 1<<2,
  WINDOW_ANCHOR_BOTTOM = 1<<3
};

static int windowPointerMotion (struct Widget *, int32_t, int32_t, int32_t, int32_t);
static int windowPointerButton (struct Widget *, int32_t, int32_t, uint32_t, bool);
static void windowPointerEnter (struct Widget *, int32_t x, int32_t y);
static void windowPointerLeave (struct Widget *);
static int windowKeyboardRaw   (struct Widget *, enum YKeyCode, bool, uint32_t);
static struct Window *windowGetWindow (struct Widget *);
static void windowRender (struct Widget *, struct Renderer *);
static void windowUnpack (struct Widget *, struct Widget *);
static void windowPaint (struct Widget *, struct Painter *);
static void windowRepaint (struct Widget *, struct Rectangle *rect);
static void windowResize (struct Widget *);
static void windowReconfigure (struct Widget *);

DEFINE_CLASS(Window);
#include "Window.yc"

/* SUPER
 * Widget
 */

/* PROPERTY
 * title :: string
 * background :: uint32
 */

static struct WidgetTable windowTable =
{
  getWindow:     windowGetWindow,
  unpack:        windowUnpack,
  pointerMotion: windowPointerMotion,
  pointerButton: windowPointerButton,
  pointerEnter:  windowPointerEnter,
  pointerLeave:  windowPointerLeave,
  keyboardRaw:   windowKeyboardRaw,
  render:        windowRender,
  paint:         windowPaint,
  repaint:       windowRepaint,
  reconfigure:   windowReconfigure,
  resize:        windowResize,
};

static inline struct Window *
castBack (struct Widget *widget)
{
  /* assert ( widget -> c == windowClass ); */
  return (struct Window *)widget;
}

static inline const struct Window *
castBackConst (const struct Widget *widget)
{
  /* assert ( widget -> c == windowClass ); */
  return (const struct Window *)widget;
}

static void
windowUnpack (struct Widget *self_w, struct Widget *w)
{
  struct Window *self = castBack (self_w);
  if (w == self -> child)
    {
      widgetRepaint (w, NULL);
      widgetSetContainer (w, NULL);
      self -> child = NULL;
    }
}

static struct Window *
windowGetWindow (struct Widget *self_w)
{
  struct Window *self = castBack (self_w);
  return self;
}

static void
windowPaint (struct Widget *self_w, struct Painter *painter)
{
  struct Window *self = castBack (self_w);
  themeWindowPaint (self, painter);
}

static void
windowRepaint (struct Widget *self_w, struct Rectangle *rect)
{
  struct Window *self = castBack (self_w);
  llist_add_tail (self -> invalidRectangles, rect);
  widgetRerender (windowToWidget (self), rectangleDuplicate (rect));
}

/* PROPERTY HOOK
 * title
 */
static void
windowTitleSet (struct Window *self)
{
  widgetRepaint (windowToWidget (self), NULL);
}

static struct Window *
windowCreate (void)
{
  struct Window *self = ymalloc (sizeof (struct Window));
  objectInitialise (&(self -> widget.o), CLASS(Window));
  widgetInitialise (&(self -> widget), &windowTable);

  self -> child = NULL; 
  self -> focus = NULL;
  self -> widget.x = (self -> widget.o.oid % 15) * 24;
  self -> widget.y = (self -> widget.o.oid % 10) * 24;
  self -> widget.w = 64;
  self -> widget.h = 48;
  self -> sizeState = WINDOW_SIZE_NORMAL;
  self -> pointerInChild = 0;
  self -> dragging = 0;
  self -> invalidRectangles = new_llist ();
  self -> buffer = rgbabufferToBuffer (rgbabufferCreate ());
  bufferSetSize (self->buffer, self->widget.w, self->widget.h);

  windowSaveGeometry (self);

  themeWindowInit (self);

  widgetReconfigure (&(self -> widget));

  return self;
}

/* METHOD
 * DESTROY :: () -> ()
 */
static void
windowDestroy (struct Window *self)
{
  wmUnregisterWindow (self);
  if (self -> child != NULL)
    widgetSetContainer (self -> child, NULL);
  llist_destroy (self -> invalidRectangles, rectangleDestroy);
  bufferDestroy (self->buffer);
  widgetFinalise (windowToWidget (self));
  objectFinalise (windowToObject (self));
  yfree (self);
}

struct Widget *
windowToWidget (struct Window *self)
{
  return &(self -> widget);
}

struct Object *
windowToObject (struct Window *self)
{
  return &(self -> widget.o);
}

struct Widget *
windowGetChild (struct Window *self)
{
  if (self == NULL)
    return NULL;
  return self -> child;
}

struct Widget *
windowGetFocussedWidget (struct Window *self)
{
  if (self == NULL)
    return NULL;
  return self -> focus;
}

void
windowSetFocussedWidget (struct Window *self, struct Widget *f)
{
  if (self == NULL)
    return;
  if (f == self -> focus)
    return;
  if (self -> focus != NULL)
    {
      widgetRepaint (self -> focus, NULL);
      if (wmSelectedWindow () == self)
        keyboardRemoveFocus (self -> focus);
    }
  self -> focus = f;
  if (self -> focus != NULL)
    {
      widgetRepaint (self -> focus, NULL);
      if (wmSelectedWindow () == self)
        keyboardSetFocus (self -> focus);
    }
}

void
windowStartReshape (struct Window *self, int xHandle, int yHandle,
                    enum WindowReshapeMode mode)
{
  if (self -> sizeState != WINDOW_SIZE_NORMAL)
    return;
  self -> dragging = mode;
  if (mode != WINDOW_RESHAPE_NONE)
    {
      self -> dragX = xHandle;
      self -> dragY = yHandle;
      pointerGrab (windowToWidget (self));
    }
}

void
windowStopReshape (struct Window *self)
{
  self -> dragging = WINDOW_RESHAPE_NONE;
  pointerRelease ();
}

void
windowSetSizeState (struct Window *self, enum WindowSizeState state)
{
  if (self -> sizeState != state)
    {
      self -> sizeState = state;
      widgetRepaint (windowToWidget (self), NULL);
    }
}

enum WindowSizeState
windowGetSizeState (struct Window *self)
{
  return self -> sizeState;
}

int
windowPointerMotion (struct Widget *self_w, int32_t x, int32_t y, int32_t dx, int32_t dy)
{
  struct Window *self = castBack (self_w);
 
  if (self -> dragging == WINDOW_RESHAPE_NONE
      && !widgetContainsPoint (self_w, x, y))
    return 0;
  if (self -> dragging != WINDOW_RESHAPE_NONE)
    {
      int32_t mdx, mdy;

      mdx = x - self -> dragX;
      mdy = y - self -> dragY;

      switch (self -> dragging)
        {
        case WINDOW_RESHAPE_MOVE:
          widgetMove (self_w, self_w -> x + mdx, self_w -> y + mdy);
          break;
        case WINDOW_RESHAPE_RESIZE_NW:
          if (self_w -> w - mdx < self_w -> minWidth)
            mdx = self_w -> w - self_w -> minWidth;
          if (self_w -> h - mdy < self_w -> minHeight)
            mdy = self_w -> h - self_w -> minHeight;
          widgetMove (self_w, self_w -> x + mdx,  self_w -> y + mdy);
          widgetResize (self_w, self_w -> w - mdx,  self_w -> h - mdy);
          break;
        case WINDOW_RESHAPE_RESIZE_N:
          if (self_w -> h - mdy < self_w -> minHeight)
            mdy = self_w -> h - self_w -> minHeight;
          widgetMove (self_w, self_w -> x,  self_w -> y + mdy);
          widgetResize (self_w, self_w -> w,  self_w -> h - mdy);
          break;
        case WINDOW_RESHAPE_RESIZE_NE:
          if (self_w -> w + mdx < self_w -> minWidth)
            mdx = self_w -> minWidth - self_w -> w;
          if (self_w -> h - mdy < self_w -> minHeight)
            mdy = self_w -> h - self_w -> minHeight;
          widgetMove (self_w, self_w -> x,  self_w -> y + mdy);
          widgetResize (self_w, self_w -> w + mdx,  self_w -> h - mdy);
          self -> dragX += mdx;
          break;
        case WINDOW_RESHAPE_RESIZE_W:
          if (self_w -> w - mdx < self_w -> minWidth)
            mdx = self_w -> w - self_w -> minWidth;
          widgetMove (self_w, self_w -> x + mdx,  self_w -> y);
          widgetResize (self_w, self_w -> w - mdx,  self_w -> h);
          break;
        case WINDOW_RESHAPE_RESIZE_E:
          if (self_w -> w + mdx < self_w -> minWidth)
            mdx = self_w -> minWidth - self_w -> w;
          widgetResize (self_w, self_w -> w + mdx,  self_w -> h);
          self -> dragX += mdx;
          break;
        case WINDOW_RESHAPE_RESIZE_SW:
          if (self_w -> w - mdx < self_w -> minWidth)
            mdx = self_w -> w - self_w -> minWidth;
          if (self_w -> h + mdy < self_w -> minHeight)
            mdy = self_w -> minHeight - self_w -> h;
          widgetMove (self_w, self_w -> x + mdx,  self_w -> y);
          widgetResize (self_w, self_w -> w - mdx,  self_w -> h + mdy);
          self -> dragY += mdy;
          break;
        case WINDOW_RESHAPE_RESIZE_S:
          if (self_w -> h + mdy < self_w -> minHeight)
            mdy = self_w -> minHeight - self_w -> h;
          widgetResize (self_w, self_w -> w,  self_w -> h + mdy);
          self -> dragY += mdy;
          break;
        case WINDOW_RESHAPE_RESIZE_SE:
          if (self_w -> w + mdx < self_w -> minWidth)
            mdx = self_w -> minWidth - self_w -> w;
          if (self_w -> h + mdy < self_w -> minHeight)
            mdy = self_w -> minHeight - self_w -> h;
          widgetResize (self_w, self_w -> w + mdx,  self_w -> h + mdy);
          self -> dragX += mdx;
          self -> dragY += mdy;
          break;
        }
      return 1;
    }
  if (self -> child != NULL && self -> dragging == WINDOW_RESHAPE_NONE)
    {
      int32_t childX, childY, childW, childH;
      widgetGetPosition (self -> child, &childX, &childY);
      widgetGetSize (self -> child, &childW, &childH);
      if (self -> pointerInChild)
        {
          if (x < childX || x >= childX + childW
              || y < childY || y >= childY + childH)
            {
              self -> pointerInChild = 0;
              widgetPointerLeave (self -> child);
            }
        } 
      else
        {
          if (x >= childX && x < childX + childW
              && y >= childY && y < childY + childH)
            {
              self -> pointerInChild = 1;
              widgetPointerEnter (self -> child, x - childX, y - childY);
            }
        }
    }
  themeWindowPointerMotion (self, x, y, dx, dy);
  return 1;
}

int
windowPointerButton (struct Widget *self_w, int32_t x, int32_t y, uint32_t b, bool pressed)
{
  struct Window *self = castBack (self_w);
  wmWindowPointerButton (self, x, y, b, pressed);
  themeWindowPointerButton (self, x, y, b, pressed);
  return 1;
}

void
windowPointerEnter (struct Widget *self_w, int32_t x, int32_t y)
{
  struct Window *self = castBack (self_w);
  wmWindowPointerEnter (self, x, y);
  if (self -> child != NULL)
    {
      int32_t childX, childY, childW, childH;
      widgetGetPosition (self -> child, &childX, &childY);
      widgetGetSize (self -> child, &childW, &childH);
      if (x >= childX && x < childX + childW
          && y >= childY && y < childY + childH)
        {
          self -> pointerInChild = 1;
          widgetPointerEnter (self -> child, x - childX, y - childY);
        }
    }
}

void
windowPointerLeave (struct Widget *self_w)
{
  struct Window *self = castBack (self_w);
  wmWindowPointerLeave (self);
  if (self -> child != NULL && self -> pointerInChild)
    {
      self -> pointerInChild = 0;
      widgetPointerLeave (self -> child);
    }
}

int
windowKeyboardRaw (struct Widget *self_w, enum YKeyCode code,
                   bool pressed, uint32_t modifierState)
{
  Y_TRACE ("window %3d: keyboard event: %d, modifiers: %6x, key: %6d '%c'",
           self_w -> o.oid,  pressed, modifierState, code,
           isprint (code) ? code : ' ');
  return 1;
}

/* METHOD
 * Window :: () -> (object)
 */
static struct Object *
windowInstantiate (void)
{
  return windowToObject (windowCreate ());
}

/* METHOD
 * setChild :: (object) -> ()
 */
void
windowSetChild (struct Window *self, struct Object *obj)
{
  struct Widget *child = (struct Widget *)obj;
  
  if (child == NULL)
    return;
  self -> child = child;
  self -> widget.reqWidth = -1;
  self -> widget.reqHeight = -1;
  widgetSetContainer (child, windowToWidget (self));
  widgetRepaint (child, NULL);
}

/* METHOD
 * setFocussed :: (object) -> ()
 */
void
windowSetFocussed (struct Window *self, struct Object *obj)
{
  struct Widget *focus = (struct Widget *)obj;
  
  if (focus == NULL)
    return;
  windowSetFocussedWidget (self, focus);
}

/* METHOD
 * show :: () -> ()
 */
void
windowShow (struct Window *self)
{
  if (self != NULL)
    {
      self -> widget.reqHeight = -1;
      self -> widget.reqWidth = -1;
      wmRegisterWindow (self);
      widgetReconfigure (windowToWidget (self));
      widgetRepaint (windowToWidget (self), NULL);
    }
}

void
windowRender (struct Widget *self_w, struct Renderer *renderer)
{
  struct Window *self = castBack (self_w);
  struct llist_node *node;
  rectanglelistUnionOverlaps (self -> invalidRectangles);
  node = llist_head (self->invalidRectangles);
  while (node != NULL)
    {
      struct llist_node *next_node = llist_node_next (node);
      struct Rectangle *rect = llist_node_data (node);
      struct Painter *painter = bufferGetPainter (self->buffer);
      painterClipTo (painter, rect);
      windowPaint (self_w, painter);
      painterDestroy (painter);
      llist_delete_node (node);
      rectangleDestroy (rect);
      node = next_node;
    }

  bufferRender (self->buffer, renderer, 0, 0);

  if (self -> child != NULL)
    {
      struct Rectangle *childRect = widgetGetRectangle (self -> child);
      if (rendererEnter (renderer, childRect, childRect->x, childRect->y))
        {
          widgetRender (self -> child, renderer);
          rendererLeave (renderer);
        }
      rectangleDestroy (childRect);
    }
}

void
windowReconfigure (struct Widget *self_w)
{
  struct Window *self = castBack (self_w);
  themeWindowReconfigure (self,
                          &(self->widget.minWidth), &(self->widget.minHeight),
                          &(self->widget.reqWidth), &(self->widget.reqHeight),
                          &(self->widget.maxWidth), &(self->widget.maxHeight));
  widgetResize (self_w, self->widget.reqWidth, self->widget.reqHeight);
}

void
windowResize (struct Widget *self_w)
{
  struct Window *self = castBack (self_w);

  bufferSetSize (self->buffer, self_w->w, self_w->h);
  themeWindowResize (self);

  widgetRepaint (windowToWidget (self), NULL);

  /* windows like to stay the same size, if they can */
  self -> widget.reqWidth = self -> widget.w;
  self -> widget.reqHeight = self -> widget.h;
}

void
windowRequestClose (struct Window *self)
{
  objectEmitSignal (windowToObject (self), "requestClose");
}

void
windowSaveGeometry (struct Window *self)
{
  self -> storeX = self -> widget.x;
  self -> storeY = self -> widget.y;
  self -> storeW = self -> widget.w;
  self -> storeH = self -> widget.h;
}

void
windowRestoreGeometry (struct Window *self)
{
  widgetMove (windowToWidget (self), self -> storeX, self -> storeY);
  widgetResize (windowToWidget (self), self -> storeW, self -> storeH);
}

/* arch-tag: 979f2694-e407-4555-9518-3d8ba5fc512d
 */
