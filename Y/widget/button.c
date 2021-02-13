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

#include <Y/widget/button.h>
#include <Y/widget/widget_p.h>

#include <Y/util/yutil.h>
#include <Y/buffer/painter.h>

#include <Y/modules/theme.h>

#include <Y/object/class_p.h>
#include <Y/object/object_p.h>

#include <Y/text/font.h>

#include <stdio.h>
#include <ctype.h>
#include <string.h>

struct Button
{
  struct Widget widget;
};

static void buttonReconfigure (struct Widget *);
static void buttonPaint (struct Widget *, struct Painter *);
static int buttonPointerMotion (struct Widget *, int32_t, int32_t, int32_t, int32_t);
static int buttonPointerButton (struct Widget *, int32_t, int32_t, uint32_t, bool);
static void buttonPointerEnter (struct Widget *, int32_t, int32_t);
static void buttonPointerLeave (struct Widget *);

DEFINE_CLASS(Button);
#include "Button.yc"

/* SUPER
 * Widget
 */

/* PROPERTY
 * text :: string
 */

static struct WidgetTable buttonTable =
{
  reconfigure:   buttonReconfigure,
  paint:         buttonPaint,
  pointerButton: buttonPointerButton,
  pointerMotion: buttonPointerMotion,
  pointerEnter:  buttonPointerEnter,
  pointerLeave:  buttonPointerLeave
};

static inline struct Button *
castBack (struct Widget *widget)
{
  /* assert ( widget -> c == buttonClass ); */
  return (struct Button *)widget;
}

static inline const struct Button *
castBackConst (const struct Widget *widget)
{
  /* assert ( widget -> c == buttonClass ); */
  return (const struct Button *)widget;
}

static void
buttonPaint (struct Widget *self_w, struct Painter *painter)
{
  painterSaveState (painter);
  themeDrawButton (painter, castBack (self_w));
  painterRestoreState (painter);
}

static struct Button *
buttonCreate (void)
{
  struct Button *self = ymalloc (sizeof (struct Button));
  objectInitialise (&(self -> widget.o), CLASS(Button));
  widgetInitialise (&(self -> widget), &buttonTable);
  return self;
}

/* METHOD
 * DESTROY :: () -> ()
 */
static void
buttonDestroy (struct Button *self)
{
  widgetFinalise (buttonToWidget (self));
  objectFinalise (buttonToObject (self));
  yfree (self);
}

struct Widget *
buttonToWidget (struct Button *self)
{
  return &(self -> widget);
}

struct Object *
buttonToObject (struct Button *self)
{
  return &(self -> widget.o);
}

/* METHOD
 * Button :: () -> (object)
 */
static struct Object *
buttonInstantiate (void)
{
  return buttonToObject (buttonCreate ());
}

static void buttonReconfigure (struct Widget *self_w)
{
  struct Button *self = castBack (self_w);
  struct Font *font;
  int offset, width, ascender, descender;
  const char *text = safeGetProperty(self, text, "");
  font = themeGetDefaultFont ();
  fontGetMetrics (font, &ascender, &descender, NULL);
  fontMeasureString (font, text, &offset, &width, NULL);

  self -> widget.minWidth = width - offset + 10;
  self -> widget.minHeight = ascender + descender + 12;
  self -> widget.reqWidth = width - offset + 10;
  self -> widget.reqHeight =  ascender + descender + 12;

  /* we would prefer to be square rather than narrow... */
  if (self -> widget.reqWidth < self -> widget.reqHeight)
    self -> widget.reqWidth = self -> widget.reqHeight;

  widgetReconfigure (self -> widget.container);
}

/* PROPERTY HOOK
 * text
 */
static void
buttonTextSet (struct Button *self)
{
  widgetReconfigure (buttonToWidget (self));
  widgetRepaint (buttonToWidget (self), NULL);
}

static int
buttonPointerButton (struct Widget *self_w, int32_t x, int32_t y, uint32_t b, bool pressed)
{
  struct Button *self = castBack (self_w);
  if (b==0)
    {
      if (pressed)
        {
          pointerGrab (self_w);
          self -> widget.state = WIDGET_STATE_PRESSED;
          widgetRepaint (self_w, NULL); 
        }
      else
        {
          pointerRelease ();
          if (self -> widget.state == WIDGET_STATE_PRESSED)
            {
              objectEmitSignal (buttonToObject (self), "clicked");
            }
          if (widgetContainsPoint (self_w, x, y))
            self -> widget.state = WIDGET_STATE_HOVER;
          else
            self -> widget.state = WIDGET_STATE_NORMAL;
          widgetRepaint (self_w, NULL);
        }
    }
  return 1;
}

static int
buttonPointerMotion (struct Widget *self_w, int32_t x, int32_t y, int32_t dx, int32_t dy)
{
  struct Button *self = castBack (self_w);
  if (self -> widget.state == WIDGET_STATE_PRESSED
      && !widgetContainsPoint (self_w, x, y))
    {
      self -> widget.state = WIDGET_STATE_CANCELLING;
      widgetRepaint (self_w, NULL);
    }
  else if (self -> widget.state == WIDGET_STATE_CANCELLING
      && widgetContainsPoint (self_w, x, y))
    {
      self -> widget.state = WIDGET_STATE_PRESSED;
      widgetRepaint (self_w, NULL);
    }
  return 1;
}

static void
buttonPointerEnter (struct Widget *self_w, int32_t x, int32_t y)
{
  struct Button *self = castBack (self_w);
  self -> widget.state = WIDGET_STATE_HOVER;
  widgetRepaint (buttonToWidget (self), NULL);
}

static void
buttonPointerLeave (struct Widget *self_w)
{
  struct Button *self = castBack (self_w);
  self -> widget.state = WIDGET_STATE_NORMAL;
  widgetRepaint (buttonToWidget (self), NULL);
}

/* arch-tag: 791d4e69-40ee-48d3-885f-fcb076464847
 */
