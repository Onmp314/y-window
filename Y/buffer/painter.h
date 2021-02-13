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

#ifndef Y_BUFFER_PAINTER_H
#define Y_BUFFER_PAINTER_H

struct Painter;

#include <Y/y.h>
#include <Y/buffer/buffer.h>
#include <Y/util/colour.h>
#include <Y/util/rectangle.h>

/* This defines an ABSTRACT Painter.
 *
 * Painters are very closely tied to buffers, i.e. a buffer might have
 * several painters associated with it, and painter implementations will
 * usually be defined within the file for a buffer implementation, since
 * it might need access to the internal structures of the buffer.
 */

void     painterDestroy (struct Painter *);

void     painterSaveState (struct Painter *);
void     painterRestoreState (struct Painter *);

void     painterTranslate (struct Painter *, double x, double y);
void     painterScale (struct Painter *, double x, double y);

void     painterClipTo (struct Painter *, struct Rectangle *);
void     painterEnter (struct Painter *, struct Rectangle *);

struct Rectangle *
         painterGetClipRectangle (struct Painter *);

int      painterFullyClipped (struct Painter *);

int      painterClipCoordinates (struct Painter *, int *x, int *y, int *w, int *h);

void     painterSetBlendMode (struct Painter *, enum ColourBlendMode);

void     painterSetPenColour (struct Painter *, uint32_t);
uint32_t painterGetPenColour (struct Painter *);
void     painterSetFillColour (struct Painter *, uint32_t);
uint32_t painterGetFillColour (struct Painter *);

/*
void  painterDrawText (struct Painter *, struct Font *, int x, int y,
                       const char *text);
*/

/* Virtual Methods *
 * =============== */

void   painterClearRectangle (struct Painter *, int x, int y, int w, int h);
void   painterDrawRectangle  (struct Painter *, int x, int y, int w, int h);
void   painterDrawHLine      (struct Painter *, int x, int y, int dx);
void   painterDrawVLine      (struct Painter *, int x, int y, int dy);
void   painterDrawLine       (struct Painter *, int x, int y, int dx, int dy);
void   painterDrawBuffer     (struct Painter *, struct Buffer *,
                              int x, int y, int w, int h, int xo, int yo);


/* renders a w by h filled rectangle at x,y in the current pen
   colour modulated by the alpha map given at alpha. */
void   painterDrawAlphamap (struct Painter *, uint8_t *alpha,
                            int x, int y, int w, int h, int s);
void   painterDrawRGBAData (struct Painter *, uint32_t *data,
                            int x, int y, int w, int h, int s);

#endif /* Y_BUFFER_PAINTER_H */

/* arch-tag: a6488fab-d0e8-4027-80f3-6539dc3cda51
 */
