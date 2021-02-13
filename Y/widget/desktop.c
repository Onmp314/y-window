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

#include <Y/widget/desktop.h>
#include <Y/widget/widget_p.h>

#include <Y/util/yutil.h>
#include <Y/buffer/rgbabuffer.h>
#include <Y/buffer/painter.h>
#include <Y/screen/screen.h>

#include <Y/main/treeinfo.h>

#include <Y/modules/theme.h>

#include <Y/object/class_p.h>
#include <Y/object/object_p.h>

#include <Y/modules/windowmanager.h>

#include <Y/text/font.h>

#include <Y/util/zorder.h>

#include <stdio.h>

struct Desktop
{
  struct Widget widget;
  struct ZOrder *windows;
  struct Widget *pointerWidget;
#if ARCH_TREE_DEVELOPMENT
  struct Buffer *versionText;
#endif
};

static int desktopPointerMotion (struct Widget *, int32_t, int32_t, int32_t, int32_t);
static int desktopPointerButton (struct Widget *, int32_t, int32_t, uint32_t, bool);
                                
static int desktopKeyboardRaw   (struct Widget *, enum YKeyCode, bool, uint32_t);
static void desktopRender (struct Widget *, struct Renderer *);
static void desktopResize (struct Widget *);

DEFINE_CLASS(Desktop);
#include "Desktop.yc"

static struct WidgetTable desktopTable =
{
  pointerMotion: desktopPointerMotion,
  pointerButton: desktopPointerButton,
  keyboardRaw:   desktopKeyboardRaw,
  render:        desktopRender,
  resize:        desktopResize
};

static inline struct Desktop *
castBack (struct Widget *widget)
{
  /* assert ( widget -> c == desktopClass ); */
  return (struct Desktop *)widget;
}

static inline const struct Desktop *
castBackConst (const struct Widget *widget)
{
  /* assert ( widget -> c == desktopClass ); */
  return (const struct Desktop *)widget;
}

static int
windowsKeyFunction (const void *key_v, const void *obj_v)
{
  const int key = *((const int *)key_v);
  const struct Object *obj = obj_v;
  int objKey = objectGetID (obj);
  if (key == objKey)
    return 0;
  if (key < objKey)
    return -1;
  else
    return 1;
}

static int
windowsComparisonFunction (const void *obj1_v, const void *obj2_v)
{
  const struct Object *obj1 = obj1_v;
  const struct Object *obj2 = obj2_v;
  int obj1Key = objectGetID (obj1);
  int obj2Key = objectGetID (obj2);
  if (obj1Key == obj2Key)
    return 0;
  if (obj1Key < obj2Key)
    return -1;
  else
    return 1;
}

struct Desktop *
desktopCreate (void)
{
  struct Desktop *self = ymalloc (sizeof (struct Desktop));
  objectInitialise (&(self -> widget.o), CLASS(Desktop));
  widgetInitialise (&(self -> widget), &desktopTable);
  
  self -> widget.x = 0;
  self -> widget.y = 0;
  self -> widget.w = 1;
  self -> widget.h = 1;

  self -> windows = zorderCreate (windowsKeyFunction, windowsComparisonFunction);

  self -> pointerWidget = NULL;

#if ARCH_TREE_DEVELOPMENT
  int32_t versionWidth, versionAscent, versionDescent;
  struct Font *versionFont = themeGetDefaultFont ();
  fontGetMetrics (versionFont, &versionAscent, &versionDescent, NULL);
  fontMeasureString (versionFont, ARCH_TREE_FULLNAME, NULL, &versionWidth, NULL);
  self->versionText = rgbabufferToBuffer (rgbabufferCreate ());
  bufferSetSize (self->versionText, versionWidth + 8,
    versionAscent + versionDescent + 8);
  struct Painter *versionPainter = bufferGetPainter (self->versionText);
  painterSetPenColour (versionPainter, 0x60000000);
  fontRenderString (versionFont, versionPainter, ARCH_TREE_FULLNAME,
                      5, versionAscent + 5);
  painterSetPenColour (versionPainter, 0x30000000);
  fontRenderString (versionFont, versionPainter, ARCH_TREE_FULLNAME,
                      6, versionAscent + 5);
  fontRenderString (versionFont, versionPainter, ARCH_TREE_FULLNAME,
                      4, versionAscent + 5);
  fontRenderString (versionFont, versionPainter, ARCH_TREE_FULLNAME,
                      5, versionAscent + 6);
  fontRenderString (versionFont, versionPainter, ARCH_TREE_FULLNAME,
                      5, versionAscent + 4); 
  painterSetPenColour (versionPainter, 0xFFFFFFFF);
  fontRenderString (versionFont, versionPainter, ARCH_TREE_FULLNAME,
                      4, versionAscent + 4);
  painterDestroy (versionPainter);

#endif

  return self;
}

/* METHOD
 * DESTROY :: () -> ()
 */
static void
desktopDestroy (struct Desktop *self)
{
  zorderDestroy (self -> windows, NULL);
#if ARCH_TREE_DEVELOPMENT
  bufferDestroy (self->versionText);
#endif
  objectFinalise (desktopToObject (self));
  yfree (self);
}

struct Widget *
desktopToWidget (struct Desktop *self)
{
  return &(self -> widget);
}

struct Object *
desktopToObject (struct Desktop *self)
{
  return &(self -> widget.o);
}

int
desktopPointerMotion (struct Widget *self_w, int32_t x, int32_t y, int32_t dx, int32_t dy)
{
  struct Desktop *self = castBack (self_w);
  struct ZOrderIterator *iterator; 
  iterator = zorderGetTopIterator (self -> windows);
  while (zorderiteratorHasValue (iterator)) 
    {
      struct Widget *widget = zorderiteratorGet (iterator);
      int wx, wy;
      widgetGetPosition (widget, &wx, &wy);
      if (widgetContainsPoint (widget, x - wx, y - wy))
        {
          if (widget != self -> pointerWidget)
            {
              if (self -> pointerWidget != NULL)
                widgetPointerLeave (self -> pointerWidget);
              widgetPointerEnter (widget, x - wx, y - wy);
              self -> pointerWidget =  widget;
            }
          if (widgetPointerMotion (widget, x - wx, y - wy, dx, dy))
            {
              zorderiteratorDestroy (iterator);
              return 1;
            }
        }
      zorderiteratorMoveDown (iterator);
    }
  zorderiteratorDestroy (iterator);
 
  if (self -> pointerWidget != NULL)
    {
      widgetPointerLeave (self -> pointerWidget);
      self -> pointerWidget = NULL;
    } 

  return 1;
}

int
desktopPointerButton (struct Widget *self_w, int32_t x, int32_t y, uint32_t b, bool pressed)
{
  struct Desktop *self = castBack (self_w);
  struct ZOrderIterator *iterator; 
  iterator = zorderGetTopIterator (self -> windows);
  while (zorderiteratorHasValue (iterator)) 
    {
      struct Widget *widget = zorderiteratorGet (iterator);
      int wx, wy;
      widgetGetPosition (widget, &wx, &wy);
      if (widgetContainsPoint (widget, x - wx, y - wy))
        {
          if (widgetPointerButton (widget, x - wx, y - wy, b, pressed))
            {
              zorderiteratorDestroy (iterator);
              return 1;
            }
        }
      zorderiteratorMoveDown (iterator);
    }
  zorderiteratorDestroy (iterator);
  

  return 1;
}

int
desktopKeyboardRaw (struct Widget *self_w, enum YKeyCode code,
                    bool pressed, uint32_t modifierState)
{
  /* struct Desktop *self = castBack (self_w); */
  return 1;
}

void
desktopRender (struct Widget *self_w, struct Renderer *renderer)
{
  struct Desktop *self = castBack (self_w);
  struct ZOrderIterator *iter;

  /* This should be in paint?
   */
  rendererDrawFilledRectangle (renderer, 0xFF404080,
                               self -> widget.x, self -> widget.y,
                               self -> widget.w, self -> widget.h);

#if ARCH_TREE_DEVELOPMENT
  int32_t versionWidth, versionHeight;
  bufferGetSize (self->versionText, &versionWidth, &versionHeight);
  bufferRender (self->versionText, renderer, self->widget.w - versionWidth,
                self->widget.h - versionHeight);
#endif

  /* render the windows */
  iter = zorderGetBottomIterator (self -> windows);

  while (zorderiteratorHasValue (iter))
    {
      struct Widget *widget = zorderiteratorGet (iter);
      struct Rectangle *widgetRectangle = widgetGetRectangle (widget);
      if (rendererEnter (renderer, widgetRectangle,
                         widgetRectangle->x, widgetRectangle->y))
        {
          widgetRender (widget, renderer);
          rendererLeave (renderer);
        }
      rectangleDestroy (widgetRectangle);
      zorderiteratorMoveUp (iter);
    }
  zorderiteratorDestroy (iter);

}

void
desktopResize (struct Widget *self_w)
{
  struct Desktop *self = castBack (self_w);
  struct ZOrderIterator *iter;

  iter = zorderGetBottomIterator (self -> windows);
  while (zorderiteratorHasValue (iter))
    {
      struct Window *win = zorderiteratorGet (iter);
      enum WindowSizeState winState = windowGetSizeState (win);
      struct Rectangle *winRect = widgetGetRectangle (windowToWidget (win));
      struct Rectangle *newRect = rectangleDuplicate (winRect);
      switch (winState)
        {
          case WINDOW_SIZE_NORMAL:
            /* push any off bottom/right windows back on-to the screen,
             * if possible */
            if (newRect -> w > self -> widget.w)
              newRect -> w = self -> widget.w;
            if (newRect -> x + newRect -> w > self -> widget.w)
              newRect -> x = self -> widget.w - newRect -> w;
            if (newRect -> h > self -> widget.h)
              newRect -> h = self -> widget.h;
            if (newRect -> y + newRect -> h > self -> widget.h)
              newRect -> y = self -> widget.h - newRect -> h;
            if (winRect -> x != newRect -> x || winRect -> y != newRect -> y)
              widgetMove (windowToWidget (win), newRect -> x, newRect -> y);
            if (winRect -> w != newRect -> w || winRect -> h != newRect -> h)
              widgetResize (windowToWidget (win), newRect -> w, newRect -> h);
            break;
          case WINDOW_SIZE_MAXIMISE:
            /* just resize it */
            widgetResize (windowToWidget (win), self -> widget.w, self -> widget.h);
            break;
        }
      rectangleDestroy (winRect);
      rectangleDestroy (newRect);
      zorderiteratorMoveUp (iter);
    }
  zorderiteratorDestroy (iter);
}

void
desktopAddWindow (struct Desktop *self, struct Window *win)
{
  zorderAddAtTop (self -> windows, win);
  widgetSetContainer (windowToWidget (win), desktopToWidget (self));
  widgetRepaint (desktopToWidget (self),
                    widgetGetRectangle (windowToWidget (win)));
}

void
desktopRaiseWindow (struct Desktop *self, struct Window *win)
{
  uint32_t id = objectGetID (windowToObject (win));
  struct ZOrderIterator *iter = zorderGetTopIterator (self -> windows);
  struct Object *top = zorderiteratorGet (iter);
  if (top != NULL && objectGetID (top) != id)
    {
      zorderMoveToTop (self -> windows, &id);
      widgetRepaint (desktopToWidget (self),
                        widgetGetRectangle (windowToWidget (win)));
    }
  zorderiteratorDestroy (iter);
}

void
desktopRemoveWindow (struct Desktop *self, struct Window *win)
{
  int id = objectGetID (windowToObject (win));
  zorderRemove (self -> windows, &id);
  if (self -> pointerWidget == windowToWidget (win))
    self -> pointerWidget = NULL;
  widgetRerender (desktopToWidget (self),
                  widgetGetRectangle (windowToWidget (win)));
}

void
desktopCycleWindows (struct Desktop *self, int direction)
{
  struct Window *win;
  if (direction == 1)
    {
      win = zorderGetTop (self -> windows);
      if (win == NULL)
        return;
      zorderMoveToBottom (self -> windows, win);
      widgetRerender (windowToWidget (win), NULL);
    }
  else
    {
      win = zorderGetBottom (self -> windows);
      if (win == NULL)
        return;
      zorderMoveToTop (self -> windows, win);
      widgetRerender (windowToWidget (win), NULL);
    }
  win = zorderGetTop (self -> windows);
  if (win != NULL)
    wmSelectWindow (win);
  widgetRerender (windowToWidget (win), NULL);
}

/* arch-tag: c944bbcb-243c-491e-af68-1953384e05ac
 */
