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

#ifndef Y_SCREEN_SWRENDERER_H
#define Y_SCREEN_SWRENDERER_H

struct SWRenderer;

#include <Y/y.h>
#include <Y/modules/videodriver_interface.h>
#include <Y/util/rectangle.h>

struct SWRenderer *swrendererCreate (struct VideoDriver *,
                                     const struct Rectangle *);
struct Renderer   *swrendererGetRenderer (struct SWRenderer *);

#endif /* Y_BUFFER_SWRENDERER_H */

/* arch-tag: b931e7c5-d3bd-4de2-acd3-3f7c2f90be6a
 */
