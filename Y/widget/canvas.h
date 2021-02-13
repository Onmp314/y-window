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

#ifndef Y_WIDGET_CANVAS_H
#define Y_WIDGET_CANVAS_H

struct Canvas;

#include <Y/y.h>
#include <Y/widget/widget.h>
#include <Y/buffer/painter.h>

struct Widget * canvasToWidget  (struct Canvas *);
struct Object * canvasToObject  (struct Canvas *);

void canvasSavePainterState (struct Canvas *);
void canvasRestorePainterState (struct Canvas *);
void canvasSetBlendMode (struct Canvas *);
void canvasSetPenColour (struct Canvas *, uint32_t);
void canvasSetFillColour (struct Canvas *, uint32_t);
void canvasReset (struct Canvas *, int32_t *, int32_t *);
void canvasClearRectangles (struct Canvas *);
void canvasDrawRectangles (struct Canvas *);
void canvasDrawHLine (struct Canvas *, int32_t, int32_t, int32_t);
void canvasDrawVLine (struct Canvas *, int32_t, int32_t, int32_t);
void canvasDrawLine (struct Canvas *, int32_t, int32_t, int32_t, int32_t);
void canvasSwapBuffers (struct Canvas *self);
void canvasRequestSize (struct Canvas *, int32_t, int32_t);

#endif /* Y_WIDGET_CANVAS_H */

/* arch-tag: f14e20e8-35a1-47a7-a1a6-ecf002497f31
 */
