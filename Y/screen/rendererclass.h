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

#ifndef Y_SCREEN_RENDERERCLASS_H
#define Y_SCREEN_RENDERERCLASS_H

#include <Y/screen/renderer.h>
#include <Y/util/llist.h>
#include <Y/util/index.h>

/* This defines the internal characteristics of an ABSTRACT Renderer */

struct Renderer
{
  struct RendererClass *c;
  struct llist *regions;
  struct Index *options;
};

struct RendererClass
{
  const char *name; /* may be used for rtti */
  void (*complete)     (struct Renderer *);
  void (*destroy)      (struct Renderer *);
  bool (*renderBuffer) (struct Renderer *, struct Buffer *, int, int,
                        int, int, int, int);
  void (*blitRGBAData) (struct Renderer *, int, int, const uint32_t *,
                        int, int, int);
  void (*drawFilledRectangle) (struct Renderer *, uint32_t,
                               int, int, int, int);
};

void rendererInitialise (struct Renderer *);

#endif /* Y_BUFFER_RENDERERCLASS_H */

/* arch-tag: 8db4f7cf-02c0-4a5a-b228-a2b16361b54f
 */
