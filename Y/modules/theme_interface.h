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

#ifndef Y_MODULES_THEME_INTERFACE_H
#define Y_MODULES_THEME_INTERFACE_H

struct Theme;

#include <Y/modules/module.h>
#include <Y/buffer/painter.h>
#include <Y/widget/widget.h>
#include <Y/widget/button.h>
#include <Y/widget/checkbox.h>
#include <Y/widget/label.h>
#include <Y/widget/window.h>

void themeAdd    (struct Theme *);
void themeRemove (struct Theme *);

/* Themes are stacked, so themes that do not implement certain functions
 * will fall through to the next theme on the stack.
 *
 * The "basic" theme should always be loaded at the bottom of the stack to
 * handle the final fall through case.
 */

struct Theme
{
  const char *name;
  struct Theme *nextTheme;    /* FOR Y USE ONLY: DO NOT USE. */

  void (*drawBackgroundPane)  (struct Painter *, int32_t x, int32_t y, int32_t w, int32_t h);

  void (*drawButtonPane)      (struct Painter *, int32_t x, int32_t y, int32_t w, int32_t h,
                               enum WidgetState state);

  void (*drawCheckBox)        (struct Painter *, struct CheckBox *);
  void (*drawLabel)           (struct Painter *, struct Label *);
  void (*drawButton)          (struct Painter *, struct Button *);

  struct Font *
       (*getDefaultFont)      (void);

  /* windows are almost entirely handled by the theme, due to their
   * highly themic nature
   */
  void (*windowInit)          (struct Window *);
  void (*windowPaint)         (struct Window *, struct Painter *);
  int  (*windowGetRegion)     (struct Window *, int32_t, int32_t);
  int  (*windowPointerMotion) (struct Window *, int32_t, int32_t, int32_t, int32_t);
  int  (*windowPointerButton) (struct Window *, int32_t, int32_t, uint32_t, bool);
  void (*windowReconfigure)   (struct Window *, int32_t *, int32_t *, int32_t *, int32_t *,
                               int32_t *, int32_t *);
  void (*windowResize)        (struct Window *);

};

#endif

/* arch-tag: eedf9fca-0711-4192-a4e2-c0c4cb61234b
 */
