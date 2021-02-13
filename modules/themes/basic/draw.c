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

#include <string.h>

#include <Y/modules/module_interface.h>
#include <Y/modules/theme_interface.h>

#include <Y/text/font.h>
#include <Y/util/yutil.h>
#include <Y/util/log.h>
#include <Y/util/rectangle.h>
#include <Y/buffer/painter.h>
#include <Y/object/class.h>
#include <Y/object/object.h>
#include <Y/widget/widget.h>
#include <Y/widget/checkbox.h>
#include <Y/widget/button.h>
#include <Y/widget/label.h>
#include <Y/modules/theme.h>

#include "draw.h"
#include "font.h"

void
basicDrawBackgroundPane (struct Painter *painter, int32_t x, int32_t y, int32_t w, int32_t h)
{
}

void
basicDrawButtonPane (struct Painter *painter, int32_t x, int32_t y, int32_t w, int32_t h,
                     enum WidgetState state)
{
  painterSaveState (painter);
  painterSetBlendMode (painter, COLOUR_BLEND_SOURCE_OVER);
  painterSetPenColour (painter, 0xFF000000);
  if (state == WIDGET_STATE_HOVER
      || state == WIDGET_STATE_PRESSED
      || state == WIDGET_STATE_CANCELLING)
    painterSetFillColour (painter, 0x40FFFFFF);
  else
    painterSetFillColour (painter, 0x00000000);
  painterDrawRectangle (painter, x, y, w, h);
  if (state == WIDGET_STATE_PRESSED)
    painterSetPenColour (painter, 0x80000000);
  else
    painterSetPenColour (painter, 0x80FFFFFF);
  painterDrawHLine (painter, x+1, y+1, w-2);
  painterDrawVLine (painter, x+1, y+2, h-2);
  if (state == WIDGET_STATE_PRESSED) 
    painterSetPenColour (painter, 0x80FFFFFF);
  else
    painterSetPenColour (painter, 0x80000000);
  painterDrawHLine (painter, x+2, y+h-2, w-3);
  painterDrawVLine (painter, x+w-2, y+1, h-3);
  painterRestoreState (painter);
  if (state == WIDGET_STATE_PRESSED)
    painterTranslate (painter, 1, 1);
}

void
basicDrawButton(struct Painter *painter, struct Button *button)
{
  struct Font *font;
  int offset, width, ascender, descender;
  const struct Value *textValue = objectGetProperty (buttonToObject (button), "text");
  const char *text = textValue ? textValue->string.data : "";
  struct Rectangle *rect = widgetGetRectangle (buttonToWidget (button));
  enum WidgetState state = widgetGetState (buttonToWidget (button));

  painterSaveState (painter);

  basicDrawButtonPane (painter, 0, 0, rect->w, rect->h, state);

  painterSetPenColour (painter, 0xFF000000);
  font = basicGetDefaultFont ();
  fontGetMetrics (font, &ascender, &descender, NULL);
  fontMeasureString (font, text, &offset, &width, NULL);
  fontRenderString (font, painter, text,
                    (rect -> w - width + offset) / 2 - offset,
                    (rect -> h - ascender - descender) / 2 + ascender);

  painterRestoreState (painter);

  rectangleDestroy(rect);

}

void
basicDrawCheckBox(struct Painter *painter, struct CheckBox *checkbox)
{
  struct Font *font;
  int offset, ascender, descender;
  const struct Value *textValue = objectGetProperty (checkboxToObject (checkbox), "text");
  const char *text = textValue ? textValue->string.data : "";
  struct Rectangle *rect = widgetGetRectangle (checkboxToWidget (checkbox));
  enum WidgetState state = widgetGetState (checkboxToWidget (checkbox));

  painterSaveState (painter);

  painterSetPenColour (painter, 0xFF000000);
  font = basicGetDefaultFont ();
  fontGetMetrics (font, &ascender, &descender, NULL);
  fontMeasureString (font, text, &offset, NULL, NULL);
  fontRenderString (font, painter, text, 20 - offset,
                    (rect -> h - ascender - descender) / 2 + ascender);

  themeDrawButtonPane (painter, 0, (rect->h - 16) / 2,
                                16, 16, state);
  const struct Value *checkedValue = objectGetProperty (checkboxToObject (checkbox), "checked");
  uint32_t checked = checkedValue ? checkedValue->uint32 : 0;
  if (checked)
    {
      int yp = (rect->h - 16) / 2;
      painterDrawLine (painter, 3, yp + 7, 3, 6); 
      painterDrawLine (painter, 6, yp + 13, 6, -12); 
    }

  painterRestoreState (painter);

  rectangleDestroy(rect);
}

void
basicDrawLabel(struct Painter *painter, struct Label *label)
{
  int offset, width, ascender, descender;
  struct Font *font;
  const struct Value *textValue = objectGetProperty (labelToObject (label), "text");
  const char *text = textValue ? textValue->string.data : "";
  const struct Value *alignmentValue = objectGetProperty (labelToObject (label), "alignment");
  const char *alignment = alignmentValue ? alignmentValue->string.data : NULL;
  struct Rectangle *rect = widgetGetRectangle (labelToWidget (label));
  enum WidgetState state = widgetGetState (labelToWidget (label));

  int xp, yp;

  painterSaveState (painter);

  painterSetPenColour (painter, 0xFF000000);

  font = basicGetDefaultFont ();

  fontGetMetrics (font, &ascender, &descender, NULL); 
  fontMeasureString (font, text, &offset, &width, NULL);

  yp = (rect->h - ascender - descender ) / 2 + ascender;
  if (alignment && strcasecmp (alignment, "right") == 0)
    xp = (rect->w - width);
  else if (alignment && strcasecmp (alignment, "center") == 0)
    xp = (rect->w - width + offset) / 2 - offset;
  else
    xp = - offset;

  fontRenderString (font, painter, text, xp, yp);

  painterRestoreState (painter);

  rectangleDestroy(rect);
}

/* arch-tag: 87b56315-7ef3-4569-8091-45b21653c601
 */
