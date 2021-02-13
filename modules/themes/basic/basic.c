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

#include <Y/modules/module_interface.h>
#include <Y/modules/theme_interface.h>

#include <Y/util/yutil.h>
#include <Y/text/font.h>

#include "window.h"
#include "draw.h"
#include "font.h"

static struct Theme basicTheme =
{
  name:           "Basic Theme",

  drawBackgroundPane:  basicDrawBackgroundPane,
  drawButtonPane:      basicDrawButtonPane,

  drawButton:          basicDrawButton,
  drawCheckBox:        basicDrawCheckBox,
  drawLabel:           basicDrawLabel,

  getDefaultFont:      basicGetDefaultFont,

  windowInit:          basicWindowInit,
  windowPaint:         basicWindowPaint,
  windowGetRegion:     basicWindowGetRegion,
  windowPointerMotion: basicWindowPointerMotion,
  windowPointerButton: basicWindowPointerButton,
  windowReconfigure:   basicWindowReconfigure,
  windowResize:        basicWindowResize

};

int
initialise (struct Module *module, const struct Tuple *args)
{
  static char moduleName[] = "Basic Theme";
  module -> name = moduleName;
  module -> data = &basicTheme;

  basicInitialiseFont ();

  themeAdd (&basicTheme);
  return 0;
}

int
finalise (struct Module *module)
{
  basicFinaliseFont ();
  themeRemove (&basicTheme);
  return 0;
}

/* arch-tag: 7eddb9ed-9c2f-4dbe-b45c-eb9db7cca2fd
 */
