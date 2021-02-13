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

#include <Y/widget/checkbox.h>
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

struct CheckBox
{
  struct Widget widget;
};

static void checkboxReconfigure (struct Widget *);
static void checkboxPaint (struct Widget *, struct Painter *);
static int checkboxPointerMotion (struct Widget *, int32_t, int32_t, int32_t, int32_t);
static int checkboxPointerButton (struct Widget *, int32_t, int32_t, uint32_t, bool);
static void checkboxPointerEnter (struct Widget *, int32_t, int32_t);
static void checkboxPointerLeave (struct Widget *);

DEFINE_CLASS(CheckBox);
#include "CheckBox.yc"

/* SUPER
 * Widget
 */

/* PROPERTY
 * text :: string
 * checked :: uint32
 */

static struct WidgetTable checkboxTable =
{
  reconfigure:   checkboxReconfigure,
  paint:         checkboxPaint,
  pointerButton: checkboxPointerButton,
  pointerMotion: checkboxPointerMotion,
  pointerEnter:  checkboxPointerEnter,
  pointerLeave:  checkboxPointerLeave
};

static inline struct CheckBox *
castBack (struct Widget *widget)
{
  /* assert ( widget -> c == checkboxClass ); */
  return (struct CheckBox *)widget;
}

static inline const struct CheckBox *
castBackConst (const struct Widget *widget)
{
  /* assert ( widget -> c == checkboxClass ); */
  return (const struct CheckBox *)widget;
}

static void
checkboxPaint (struct Widget *self_w, struct Painter *painter)
{
  painterSaveState (painter);
  themeDrawCheckBox (painter, castBack (self_w));
  painterRestoreState (painter);
}

static struct CheckBox *
checkboxCreate (void)
{
  struct CheckBox *self = ymalloc (sizeof (struct CheckBox));
  objectInitialise (&(self -> widget.o), CLASS(CheckBox));
  widgetInitialise (&(self -> widget), &checkboxTable);
  return self;
}

/* METHOD
 * DESTROY :: () -> ()
 */
static void
checkboxDestroy (struct CheckBox *self)
{
  widgetFinalise (checkboxToWidget (self));
  objectFinalise (checkboxToObject (self));
  yfree (self);
}

struct Widget *
checkboxToWidget (struct CheckBox *self)
{
  return &(self -> widget);
}

struct Object *
checkboxToObject (struct CheckBox *self)
{
  return &(self -> widget.o);
}

/* METHOD
 * CheckBox :: () -> (object)
 */
static struct Object *
checkboxInstantiate (void)
{
  return checkboxToObject (checkboxCreate ());
}

static void checkboxReconfigure (struct Widget *self_w)
{
  struct CheckBox *self = castBack (self_w);
  struct Font *font;
  int offset, width, ascender, descender;
  const char *text = safeGetProperty(self, text, "");
  font = themeGetDefaultFont ();
  fontGetMetrics (font, &ascender, &descender, NULL);
  fontMeasureString (font, text, &offset, &width, NULL);

  self -> widget.minWidth = width - offset + 22;
  self -> widget.minHeight = ascender + descender + 2;
  self -> widget.reqWidth = width - offset + 22;
  self -> widget.reqHeight = ascender + descender + 2;

  if (self -> widget.minHeight < 16)
    {
      self -> widget.minHeight = 16;
      self -> widget.reqHeight = 16;
    }

  /* we would prefer to be square rather than narrow... */
  if (self -> widget.reqWidth < self -> widget.reqHeight)
    self -> widget.reqWidth = self -> widget.reqHeight;

  widgetReconfigure (self -> widget.container);
}

/* PROPERTY HOOK
 * text
 */
static void
checkboxTextSet (struct CheckBox *self)
{
  widgetReconfigure (checkboxToWidget (self));
  widgetRepaint (checkboxToWidget (self), NULL);
}

/* PROPERTY HOOK
 * checked
 */
static void
checkboxCheckedSet (struct CheckBox *self)
{
  widgetRepaint (checkboxToWidget (self), NULL);
}

static int
checkboxPointerButton (struct Widget *self_w, int32_t x, int32_t y, uint32_t b, bool pressed)
{
  struct CheckBox *self = castBack (self_w);
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
              uint32_t value = safeGetProperty(self, checked, 0);
              value = !value;
              setProperty(self, checked, value);
              objectEmitSignal (checkboxToObject (self), "clicked", tb_uint32(value));
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
checkboxPointerMotion (struct Widget *self_w, int32_t x, int32_t y, int32_t dx, int32_t dy)
{
  struct CheckBox *self = castBack (self_w);
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
checkboxPointerEnter (struct Widget *self_w, int32_t x, int32_t y)
{
  struct CheckBox *self = castBack (self_w);
  self -> widget.state = WIDGET_STATE_HOVER;
  widgetRepaint (checkboxToWidget (self), NULL);
}

static void
checkboxPointerLeave (struct Widget *self_w)
{
  struct CheckBox *self = castBack (self_w);
  self -> widget.state = WIDGET_STATE_NORMAL;
  widgetRepaint (checkboxToWidget (self), NULL);
}


/* arch-tag: 14a3dfdd-2ed1-4460-8630-96cc4a61094d
 */
