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

#include <Y/modules/theme.h>
#include <Y/modules/theme_interface.h>

#include <string.h>

static struct Theme *topTheme = NULL;

void
themeAdd (struct Theme *theme)
{
  theme -> nextTheme = topTheme;
  topTheme = theme;
}

void
themeRemove (struct Theme *theme)
{
  struct Theme * current = topTheme;

  if (topTheme == NULL)
    return;

  if (topTheme == theme)
    {
      topTheme = topTheme -> nextTheme;
      return;
    }

  while (current -> nextTheme != NULL)
    {
      if (current -> nextTheme == theme)
        {
          current -> nextTheme = current -> nextTheme -> nextTheme;
          return;
        }
      current = current -> nextTheme;
    }
}


/**
 * draw a themed background pane
 */
void
themeDrawBackgroundPane (struct Painter *p, int32_t x, int32_t y, int32_t w, int32_t h)
{
  struct Theme *current = topTheme;

  while (current != NULL)
    {
      if (current -> drawBackgroundPane != NULL)
        return current -> drawBackgroundPane (p, x, y, w, h);
      current = current -> nextTheme;
    }
}

/**
 * draw a themed button pane
 */
void
themeDrawButtonPane (struct Painter *p, int32_t x, int32_t y, int32_t w, int32_t h,
                     enum WidgetState state)
{
  struct Theme *current = topTheme;

  while (current != NULL)
    {
      if (current -> drawButtonPane != NULL)
        return current -> drawButtonPane (p, x, y, w, h, state);
      current = current -> nextTheme;
    }
}

/**
 * draw a themed check box
 */
void
themeDrawCheckBox (struct Painter *p, struct CheckBox *w)
{
    struct Theme *current = topTheme;
    int result;

    while(current != NULL)
      {
        if(current->drawCheckBox != NULL)
          {
            current->drawCheckBox (p, w);
            return;
          }
        current = current->nextTheme;
      }
}

/**
 * draw a themed label
 */
void
themeDrawLabel (struct Painter *p, struct Label *w)
{
    struct Theme *current = topTheme;
    int result;

    while(current != NULL)
      {
        if(current->drawLabel != NULL)
          {
            current->drawLabel (p, w);
            return;
          }
        current = current->nextTheme;
      }
}

/**
 * draw a themed button
 */
void
themeDrawButton (struct Painter *p, struct Button *w)
{
    struct Theme *current = topTheme;
    int result;

    while(current != NULL)
      {
        if(current->drawButton != NULL)
          {
            current->drawButton (p, w);
            return;
          }
        current = current->nextTheme;
      }
}

struct Font *
themeGetDefaultFont (void)
{
  struct Theme *current = topTheme;

  while (current != NULL)
    {
      if (current -> getDefaultFont != NULL)
        return current -> getDefaultFont ();
      current = current -> nextTheme;
    }
  return NULL;
}


void 
themeWindowInit (struct Window *w)
{
  struct Theme *current = topTheme;

  while (current != NULL)
    {
      if (current -> windowInit != NULL)
        return current -> windowInit (w);
      current = current -> nextTheme;
    }
}

void
themeWindowPaint (struct Window *w, struct Painter *p)
{
  struct Theme *current = topTheme;

  while (current != NULL)
    {
      if (current -> windowPaint != NULL)
        return current -> windowPaint (w, p);
      current = current -> nextTheme;
    }
}

static int
themeWindowGetRegion (struct Window *w, int x, int y)
{
  struct Theme *current = topTheme;

  while (current != NULL)
    {
      if (current -> windowGetRegion != NULL)
        return current -> windowGetRegion (w, x, y);
      current = current -> nextTheme;
    }
  return 0;
}

int
themeWindowPointerMotion (struct Window *w, int32_t x, int32_t y, int32_t dx, int32_t dy)
{
  struct Theme *current = topTheme;

  while (current != NULL)
    {
      if (current -> windowPointerMotion != NULL)
        return current -> windowPointerMotion (w, x, y, dx, dy);
      current = current -> nextTheme;
    }
  return 0;
}

int
themeWindowPointerButton (struct Window *w, int32_t x, int32_t y, uint32_t b, bool s)
{
  struct Theme *current = topTheme;

  while (current != NULL)
    {
      if (current -> windowPointerButton != NULL)
        return current -> windowPointerButton (w, x, y, b, s);
      current = current -> nextTheme;
    }
  return 0;
}

void
themeWindowReconfigure (struct Window *w,
                        int32_t *minWidth_p, int32_t *minHeight_p,
                        int32_t *reqWidth_p, int32_t *reqHeight_p,
                        int32_t *maxWidth_p, int32_t *maxHeight_p)
{
  struct Theme *current = topTheme;

  while (current != NULL)
    {
      if (current -> windowReconfigure != NULL)
        return current -> windowReconfigure (w, minWidth_p, minHeight_p,
                                                reqWidth_p, reqHeight_p,
                                                maxWidth_p, maxHeight_p);
      current = current -> nextTheme;
    }
}

void themeWindowResize (struct Window *w)
{
  struct Theme *current = topTheme;

  while (current != NULL)
    {
      if (current -> windowResize != NULL)
        return current -> windowResize (w);
      current = current -> nextTheme;
    }
}


/* arch-tag: 95fe2cbe-4eb4-40f1-91ac-077f990e1e5a
 */
