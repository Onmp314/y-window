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

#ifndef Y_SCREEN_HWRENDERER_H
#define Y_SCREEN_HWRENDERER_H

struct HWRenderer;

#include <Y/y.h>
#include <Y/modules/videodriver_interface.h>
#include <Y/util/rectangle.h>

struct HWRendererAccel
{
  void (*complete)     (struct VideoDriver *);
  void (*destroy)      (struct VideoDriver *);

  bool (*renderBuffer) (struct VideoDriver *, struct Buffer *,
                        int x, int y, int xo, int yo, int rw, int rh);

  void (*blitRGBAData) (struct VideoDriver *, int, int, const uint32_t *,
                        int, int, int);
  void (*drawFilledRectangle) (struct VideoDriver *, uint32_t,
                               int, int, int, int);
};

struct HWRenderer *hwrendererCreate (struct VideoDriver *,
                                     const struct Rectangle *,
                                     const struct HWRendererAccel *);
struct Renderer   *hwrendererGetRenderer (struct HWRenderer *);

#endif

/* arch-tag: d301186f-1f20-417f-b5c4-20f1bfbf88f6
 */
