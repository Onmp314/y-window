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

#ifndef Y_RGBABUFFER_H
#define Y_RGBABUFFER_H

struct RGBABuffer;

#include <Y/y.h>
#include <Y/util/rectangle.h>
#include <Y/screen/renderer.h>
#include <stdint.h>

/* Implementation of a buffer that stores Red-Green-Blue-Alpha data. */

/** \brief create a new, empty RGBA buffer
 */
struct RGBABuffer * rgbabufferCreate (void);

/** \brief create a new RGBA buffer from existing data
 * \param w  the width of the meaningful data in the buffer
 * \param h  the height of the meaningful data in the buffer
 * \param dw the actual width of allocated data; must be a power of 2,
 *           and not less than \p w
 * \param dh the actual height of allocated data; must be a power of 2,
 *           and not less than \p h
 */
struct RGBABuffer * rgbabufferCreateFromData (int w, int h, int dw, int dh,
                                              uint32_t *data);

void rgbabufferDestroy   (struct RGBABuffer *);

void rgbabufferAccessInternals (struct RGBABuffer *,
                                int *dw_p, int *dh_p, uint32_t **data_p);

struct Buffer *rgbabufferToBuffer (struct RGBABuffer *);

bool bufferIsRGBABuffer (struct Buffer *);

#endif

/* arch-tag: d687c025-5036-437c-915d-98bf15d84784
 */
