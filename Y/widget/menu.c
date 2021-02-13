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

#include <Y/widget/menu.h>
#include <Y/widget/widget_p.h>

#include <Y/util/yutil.h>
#include <Y/util/index.h>
#include <Y/buffer/painter.h>

#include <Y/object/class_p.h>
#include <Y/object/object_p.h>
#include <Y/modules/theme.h>

#include <Y/text/font.h>

#include <stdio.h>
#include <ctype.h>
#include <string.h>

/*
 * MenuItems are always sorted in id order. There may be gaps in the id
 * sequence.
 * 
 * Text of "-" means a separator.
 * 
 * submenu of NULL means a leaf item that will emit a "clicked" signal with
 * parameter 'id'.
 */

struct MenuItem
{
  uint32_t id;
  char *text;
  int32_t yPosition;
  int32_t height;
  struct Menu *submenu;
};

struct Menu
{
  struct Widget widget;
  struct Index *items;  /* index of struct MenuItem * */
  struct MenuItem *current;
};

static int
menuitemKeyFunction (const void *key_v, const void *item_v)
{
  const uint32_t *key = key_v;
  const struct MenuItem *item = item_v;
  if (*key < item->id)
    return -1;
  else
    return (*key > item->id);
}

static int
menuitemComparisonFunction (const void *item1_v, const void *item2_v)
{
  const struct MenuItem *item1 = item1_v, *item2 = item2_v;
  if (item1->id < item2->id)
    return -1;
  else
    return (item1->id > item2->id);
}

static void
menuitemDestructorFunction (void *item_v)
{
  struct MenuItem *item = item_v;
  yfree (item -> text);
  yfree (item);
}

static void menuReconfigure (struct Widget *);
static void menuPaint (struct Widget *, struct Painter *);
static int menuPointerMotion (struct Widget *, int32_t, int32_t, int32_t, int32_t);
static int menuPointerButton (struct Widget *, int32_t, int32_t, uint32_t, bool);
static void menuPointerEnter (struct Widget *, int32_t, int32_t);
static void menuPointerLeave (struct Widget *);

DEFINE_CLASS(Menu);
#include "Menu.yc"

/* SUPER
 * Widget
 */

static struct WidgetTable menuTable =
{
  reconfigure:   menuReconfigure,
  paint:         menuPaint,
  pointerMotion: menuPointerMotion,
  pointerButton: menuPointerButton,
  pointerEnter:  menuPointerEnter,
  pointerLeave:  menuPointerLeave
};

static inline struct Menu *
castBack (struct Widget *widget)
{
  /* assert ( widget -> c == menuClass ); */
  return (struct Menu *)widget;
}

static inline const struct Menu *
castBackConst (const struct Widget *widget)
{
  /* assert ( widget -> c == menuClass ); */
  return (const struct Menu *)widget;
}

static void
menuPaint (struct Widget *self_w, struct Painter *painter)
{
  struct Menu *self = castBack (self_w);
  struct Font *font;
  int yp;
  int ascender, offset;
  struct IndexIterator *iter;

  font = themeGetDefaultFont ();
  fontGetMetrics (font, &ascender, NULL, NULL);
  iter = indexGetStartIterator (self -> items);
  while (indexiteratorHasValue (iter))
    {
      struct MenuItem *item = indexiteratorGet (iter);
      yp = item -> yPosition + 2;
      if (strcmp (item -> text, "-") == 0)
        {
          painterSetPenColour (painter, 0x80000000);
          painterDrawHLine (painter, 0, yp - 1, self -> widget.w);
          painterSetPenColour (painter, 0x80FFFFFF);
          painterDrawHLine (painter, 0, yp, self -> widget.w);
        }
      else
        {
          painterSaveState (painter);
          if (self -> current == item)
            themeDrawButtonPane (painter, 0, item -> yPosition,
                                 self -> widget.w, item -> height,
                                 self -> widget.state);
          painterSetPenColour (painter, 0xFF000000);
          fontMeasureString (font, item -> text, &offset, NULL, NULL);
          fontRenderString (font, painter, item -> text, 4 - offset, yp + ascender);
          painterRestoreState (painter);
        }
      indexiteratorNext (iter);
    }
  indexiteratorDestroy (iter);
}

static struct Menu *
menuCreate (void)
{
  struct Menu *self = ymalloc (sizeof (struct Menu));
  objectInitialise (&(self -> widget.o), CLASS(Menu));
  widgetInitialise (&(self -> widget), &menuTable);
  self -> items = indexCreate (menuitemKeyFunction, menuitemComparisonFunction);
  self -> current = NULL;
  return self;
}

/* METHOD
 * DESTROY :: () -> ()
 */
static void
menuDestroy (struct Menu *self)
{
  indexDestroy (self -> items, menuitemDestructorFunction);
  widgetFinalise (menuToWidget (self));
  objectFinalise (menuToObject (self));
  yfree (self);
}

struct Widget *
menuToWidget (struct Menu *self)
{
  return &(self -> widget);
}

struct Object *
menuToObject (struct Menu *self)
{
  return &(self -> widget.o);
}

/* METHOD
 * Menu :: () -> (object)
 */
static struct Object *
menuInstantiate (void)
{
  return menuToObject (menuCreate ());
}

static void menuReconfigure (struct Widget *self_w)
{
  struct Menu *self = castBack (self_w);
  struct Font *font;
  int32_t maxWidth = 0;
  int32_t height = 0;
  int ascender, descender, linegap, width, offset;
  struct IndexIterator *iter;
  font = themeGetDefaultFont ();
  fontGetMetrics (font, &ascender, &descender, &linegap);
  iter = indexGetStartIterator (self -> items);
  while (indexiteratorHasValue (iter))
    {
      struct MenuItem *item = indexiteratorGet (iter);
      if (strcmp (item -> text, "-") == 0)
        {
          item -> yPosition = height;
          item -> height = 4;
          height += 4;
        }
      else
        {
          fontMeasureString (font, item -> text, &offset, &width, NULL);
          if ((width - offset) > maxWidth)
            maxWidth = width - offset;
          item -> yPosition = height;
          item -> height = ascender + descender + 4;
          height += ascender + descender + 4;
        }
      indexiteratorNext (iter);
    }
  indexiteratorDestroy (iter);

  self -> widget.minWidth  = maxWidth + 8;
  self -> widget.minHeight = height;
  self -> widget.reqWidth  = maxWidth + 8;
  self -> widget.reqHeight = height;
  self -> widget.maxHeight = height;

  widgetReconfigure (self -> widget.container);
}

static struct MenuItem *
menuItemAt (struct Menu *self, int32_t x, int32_t y)
{
  struct IndexIterator *iter = indexGetStartIterator (self -> items);
  while (indexiteratorHasValue (iter))
    {
      struct MenuItem *item = indexiteratorGet (iter);
      if (item -> yPosition <= y && y < item -> yPosition + item -> height)
        {
          indexiteratorDestroy (iter);
          return item;
        }

      indexiteratorNext (iter);
    }
  indexiteratorDestroy (iter);
  return NULL; 
}

int
menuPointerButton (struct Widget *self_w, int32_t x, int32_t y, uint32_t b, bool pressed)
{
  struct Menu *self = castBack (self_w);
  if (b==0)
    {
      if (pressed)
        {
          pointerGrab (menuToWidget (self));
          self -> widget.state = WIDGET_STATE_PRESSED;
          self -> current = menuItemAt (self, x, y);
          if (self -> current != NULL)
            widgetRepaint (menuToWidget (self),
                rectangleCreate (0, self -> current -> yPosition,
                    self -> widget.w, self -> current -> height));
        }
      else
        {
          pointerRelease ();
          if (self -> current != NULL)
            {
              objectEmitSignal (menuToObject (self), "clicked", tb_uint32(self->current->id));
              widgetRepaint (menuToWidget (self),
                  rectangleCreate (0, self -> current -> yPosition,
                      self -> widget.w, self -> current -> height));
            }
          if (widgetContainsPoint (menuToWidget (self), x, y))
            self -> widget.state = WIDGET_STATE_HOVER;
          else
            {
              self -> widget.state = WIDGET_STATE_NORMAL;
              self -> current = NULL;
            }
        }
    }
  return 1; 
}

int
menuPointerMotion (struct Widget *self_w, int32_t x, int32_t y, int32_t dx, int32_t dy)
{
  struct Menu *self = castBack (self_w);
  if (self -> widget.state == WIDGET_STATE_PRESSED &&
      !widgetContainsPoint (menuToWidget (self), x, y))
    {
      self -> widget.state = WIDGET_STATE_CANCELLING;
      if (self -> current != NULL)
        widgetRepaint (menuToWidget (self),
            rectangleCreate (0, self -> current -> yPosition,
                self -> widget.w, self -> current -> height));
      self -> current = NULL;
    }
  else if (widgetContainsPoint (menuToWidget (self), x, y))
    {
      struct MenuItem *item = menuItemAt (self, x, y);
      if (item != NULL && strcmp (item -> text, "-") == 0)
        item = NULL;

      if (item != NULL && self -> widget.state == WIDGET_STATE_CANCELLING)
        self -> widget.state = WIDGET_STATE_PRESSED;

      if (item != self -> current)
        {
          if (self -> current != NULL)
            {
              widgetRepaint (menuToWidget (self),
                  rectangleCreate (0, self -> current -> yPosition,
                      self -> widget.w, self -> current -> height));
            }
          if (item != NULL)
            {
              widgetRepaint (menuToWidget (self),
                  rectangleCreate (0, item -> yPosition,
                      self -> widget.w, item -> height));
            }
          self -> current = item;
        }
      
    }
  return 1;
}

void
menuPointerEnter (struct Widget *self_w, int32_t x, int32_t y)
{
  struct Menu *self = castBack (self_w);
  self -> widget.state = WIDGET_STATE_HOVER;
  self -> current = menuItemAt (self, x, y);
  if (self -> current != NULL)
    widgetRepaint (menuToWidget (self),
        rectangleCreate (0, self -> current -> yPosition,
            self -> widget.w, self -> current -> height));
}

void
menuPointerLeave (struct Widget *self_w)
{
  struct Menu *self = castBack (self_w);
  self -> widget.state = WIDGET_STATE_NORMAL;
  if (self -> current != NULL)
    widgetRepaint (menuToWidget (self),
        rectangleCreate (0, self -> current -> yPosition,
            self -> widget.w, self -> current -> height));
  self -> current = NULL; 
}  

void
menuAddItem (struct Menu *self, uint32_t id, const char *text, uint32_t submenuId)
{
  struct Object *submenu;
  struct MenuItem *item;
  
  if (indexFind (self -> items, &id) != NULL)
    return;

  submenu = objectFind (submenuId);
  if (submenu != NULL && !classInherits (objectClass (submenu), CLASS(Menu)))
    return; 

  item = ymalloc (sizeof (struct MenuItem));
  item -> id = id;
  item -> text = ystrdup (text);
  item -> height = 0;
  item -> submenu = (struct Menu *) submenu;  /* ok, because we checked */ 

  indexAdd (self -> items, item);
  widgetReconfigure (menuToWidget (self));
}

/* METHOD
 * addItem :: (uint32, string, uint32) -> ()
 */
static void
menuCAddItem (struct Menu *self, uint32_t id, uint32_t text_len, const char *text, uint32_t submenu)
{
  menuAddItem (self, id, text, submenu);
}

/* arch-tag: 550b1df2-ba00-4c70-942e-2591256f5eff
 */
