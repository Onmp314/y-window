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

#ifndef Y_SCREEN_SIMPLERENDERER_H
#define Y_SCREEN_SIMPLERENDERER_H

struct SimpleRenderer;

#include <Y/y.h>
#include <Y/modules/videodriver_interface.h>
#include <Y/util/rectangle.h>

struct SimpleRenderer *simplerendererCreate (struct VideoDriver *,
                                             const struct Rectangle *);
struct Renderer       *simplerendererGetRenderer (struct SimpleRenderer *);

#endif /* Y_BUFFER_SIMPLERENDERER_H */

/* arch-tag: ea755ef2-de3c-4261-9c11-289c7d0685da
 */
