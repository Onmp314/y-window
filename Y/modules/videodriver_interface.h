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

#ifndef Y_MODULES_VIDEODRIVER_INTERFACE_H
#define Y_MODULES_VIDEODRIVER_INTERFACE_H

#include <Y/y.h>
#include <Y/modules/module.h>
#include <Y/message/message.h>
#include <Y/screen/renderer.h>
#include <Y/util/llist.h>
#include <Y/util/rectangle.h>
#include <stdint.h>

struct VideoResolution
{
  char *name;
};

struct VideoDriver
{
  struct Module *module;
  void *d;
  void (*getPixelDimensions) (struct VideoDriver *, int*, int*);
  const char *
       (*getName)            (struct VideoDriver *);
  struct llist *
       (*getResolutions)     (struct VideoDriver *);
  void (*setResolution)      (struct VideoDriver *, const char *name);
  void (*setPointer)         (struct VideoDriver *, const uint8_t *, int, int, int, int, int);
  struct Tuple *(*special)   (struct VideoDriver *, const struct Tuple *);

  void (*beginUpdates)       (struct VideoDriver *);
  void (*endUpdates)         (struct VideoDriver *);

  void (*drawPixel)          (struct VideoDriver *, uint32_t, int, int);
  void (*drawRectangle)      (struct VideoDriver *, uint32_t, int, int, int, int);
  void (*drawFilledRectangle)(struct VideoDriver *, uint32_t, int, int, int, int);

  void (*blit)               (struct VideoDriver *, uint32_t *,
                              int, int, int, int, int);

  struct Renderer * (*getRenderer) (struct VideoDriver *,
                                    const struct Rectangle *);

};

#endif

/* arch-tag: 5f5c7173-7eb3-4465-820d-1b1452eaf910
 */
