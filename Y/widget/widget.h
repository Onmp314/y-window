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

#ifndef Y_WIDGET_WIDGET_H
#define Y_WIDGET_WIDGET_H

#include <stdbool.h>

struct Widget;
struct WidgetTable;

enum WidgetState
{
  WIDGET_STATE_NORMAL,
  WIDGET_STATE_HOVER,
  WIDGET_STATE_FOCUS,
  WIDGET_STATE_SELECTED,
  WIDGET_STATE_PRESSED,
  WIDGET_STATE_CANCELLING,
  WIDGET_STATE_DISABLED
};

#include <Y/y.h>
#include <Y/const.h>
#include <Y/util/rectangle.h>
#include <Y/screen/renderer.h>
#include <Y/buffer/painter.h>
#include <Y/input/pointer.h>
#include <Y/input/keyboard.h>
#include <Y/input/ykb.h>

struct Object *
       widgetToObject      (struct Widget *);

void   widgetGlobalToLocal (const struct Widget *, int *, int *);
void   widgetLocalToGlobal (const struct Widget *, int *, int *);

struct Rectangle *
       widgetGetRectangle  (const struct Widget *);

void   widgetGetPosition   (const struct Widget *, int32_t *, int32_t *);
void   widgetGetSize       (const struct Widget *, int32_t *, int32_t *);
void   widgetGetConstraints(const struct Widget *, int32_t *, int32_t *,
                            int32_t *, int32_t *, int32_t *, int32_t *);
enum WidgetState
       widgetGetState      (const struct Widget *);
bool   widgetContainsPoint (const struct Widget *, int32_t x, int32_t y); 

struct Window *
       widgetGetWindow     (struct Widget *);

void   widgetMove          (struct Widget *, int32_t, int32_t);
void   widgetResize        (struct Widget *, int32_t, int32_t);
void   widgetReconfigure   (struct Widget *);

void   widgetSetContainer  (struct Widget *, struct Widget *);
void   widgetUnpack        (struct Widget *, struct Widget *);

void   widgetRender        (struct Widget *, struct Renderer *);
void   widgetPaint         (struct Widget *, struct Painter *);
void   widgetRepaint       (struct Widget *, struct Rectangle *);
void   widgetRerender      (struct Widget *, struct Rectangle *);

int    widgetPointerMotion (struct Widget *, int32_t x, int32_t y, int32_t dx, int32_t dy);
int    widgetPointerButton (struct Widget *, int32_t x, int32_t y, uint32_t b, bool pressed);
void   widgetPointerEnter  (struct Widget *, int32_t x, int32_t y);
void   widgetPointerLeave  (struct Widget *);
int    widgetKeyboardRaw   (struct Widget *, enum YKeyCode, bool pressed,
                                             uint32_t modifierState);

ykbStringHandler widgetykbString;
ykbEventHandler widgetykbEvent;
ykbStrokeHandler widgetykbStroke;
ykbGetCursor widgetykbGetCursor;

#endif /* header guard */

/* arch-tag: 1b0c27f4-f65e-48fa-a762-c2bac42cffc9
 */
