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

#include <Y/widget/label.h>
#include <Y/widget/widget_p.h>

#include <Y/util/yutil.h>
#include <Y/buffer/painter.h>

#include <Y/object/class_p.h>
#include <Y/object/object_p.h>
#include <Y/modules/theme.h>

#include <Y/text/font.h>

#include <stdio.h>
#include <ctype.h>
#include <string.h>

struct Label
{
  struct Widget widget;
};

static void labelReconfigure (struct Widget *);
static void labelPaint (struct Widget *, struct Painter *);

DEFINE_CLASS(Label);
#include "Label.yc"

/* SUPER
 * Widget
 */

/* PROPERTY
 * text :: string
 * alignment :: string
 */

static struct WidgetTable labelTable =
{
  reconfigure:   labelReconfigure,
  paint:         labelPaint,
};

static inline struct Label *
castBack (struct Widget *widget)
{
  /* assert ( widget -> c == labelClass ); */
  return (struct Label *)widget;
}

static inline const struct Label *
castBackConst (const struct Widget *widget)
{
  /* assert ( widget -> c == labelClass ); */
  return (const struct Label *)widget;
}

static void
labelPaint (struct Widget *self_w, struct Painter *painter)
{
  painterSaveState(painter);
  themeDrawLabel (painter, castBack (self_w));
  painterRestoreState(painter);
}

static struct Label *
labelCreate (void)
{
  struct Label *self = ymalloc (sizeof (struct Label));
  objectInitialise (&(self -> widget.o), CLASS(Label));
  widgetInitialise (&(self -> widget), &labelTable);
  return self;
}

/* METHOD
 * DESTROY :: () -> ()
 */
static void
labelDestroy (struct Label *self)
{
  widgetFinalise (labelToWidget (self));
  objectFinalise (labelToObject (self));
  yfree (self);
}

struct Widget *
labelToWidget (struct Label *self)
{
  return &(self -> widget);
}

struct Object *
labelToObject (struct Label *self)
{
  return &(self -> widget.o);
}

/* METHOD
 * Label :: () -> (object)
 */
static struct Object *
labelInstantiate (void)
{
  return labelToObject (labelCreate ());
}

static void labelReconfigure (struct Widget *self_w)
{
  struct Label *self = castBack (self_w);
  struct Font *font;
  const char *text = safeGetProperty(self, text, "");
  int offset, width, ascender, descender;
  font = themeGetDefaultFont ();
  fontGetMetrics (font, &ascender, &descender, NULL);
  fontMeasureString (font, text, &offset, &width, NULL);

  self -> widget.minWidth = width - offset;
  self -> widget.minHeight = ascender + descender;
  self -> widget.reqWidth = width - offset;
  self -> widget.reqHeight = ascender + descender;

  widgetReconfigure (self -> widget.container);
}

/* PROPERTY HOOK
 * text
 */
static void
labelTextSet (struct Label *self)
{
  widgetReconfigure (labelToWidget (self));
  widgetRepaint (labelToWidget (self), NULL);
}

/* PROPERTY HOOK
 * alignment
 */
static void
labelAlignmentSet (struct Label *self)
{
  widgetReconfigure (labelToWidget (self));
  widgetRepaint (labelToWidget (self), NULL);
}

/* arch-tag: 8badd23a-de08-4a6f-91b7-a99906b1a49d
 */
