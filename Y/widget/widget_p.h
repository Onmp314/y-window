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

#ifndef Y_WIDGET_WIDGET_P_H
#define Y_WIDGET_WIDGET_P_H

#include <Y/widget/widget.h>
#include <Y/object/class.h>
#include <Y/object/object_p.h>
#include <Y/input/ykb.h>

struct WidgetTable
{
  bool            (*containsPoint)(const struct Widget *, int32_t, int32_t);

  struct Window * (*getWindow)    (struct Widget *);

  void            (*unpack)       (struct Widget *, struct Widget *);

  void            (*render)       (struct Widget *, struct Renderer *);
  void            (*paint)        (struct Widget *, struct Painter *);
  void            (*repaint)      (struct Widget *, struct Rectangle *);

  void            (*reconfigure)  (struct Widget *);
  void            (*resize)       (struct Widget *);

  int             (*pointerMotion)(struct Widget *, int32_t, int32_t, int32_t, int32_t);
  int             (*pointerButton)(struct Widget *, int32_t, int32_t, uint32_t, bool);
  void            (*pointerEnter) (struct Widget *, int32_t, int32_t);
  void            (*pointerLeave) (struct Widget *);
  int             (*keyboardRaw)  (struct Widget *, enum YKeyCode, bool, uint32_t);
  ykbStringHandler *ykbString;
  ykbEventHandler  *ykbEvent;
  ykbStrokeHandler *ykbStroke;
  ykbGetCursor     *ykbGetCursor;
};

struct Widget
{
  struct Object o;
  struct Object *parent;
  struct Widget *container;
  struct WidgetTable *tab;
  enum WidgetState state;
  int32_t x, y;
  int32_t w, h;
  int32_t minWidth, minHeight;
  int32_t reqWidth, reqHeight;
  int32_t maxWidth, maxHeight;
};

void widgetInitialise (struct Widget *, struct WidgetTable *);
void widgetFinalise (struct Widget *);

#endif /* header guard */

/* arch-tag: 2b199ef4-f9f1-4bbb-9323-d2b014b96f27
 */
