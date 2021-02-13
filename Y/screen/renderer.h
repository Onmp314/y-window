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

#ifndef Y_SCREEN_RENDERER_H
#define Y_SCREEN_RENDERER_H

struct Renderer;

#include <Y/y.h>
#include <Y/util/rectangle.h>
#include <Y/buffer/buffer.h>
#include <stdint.h>

/* This represents an ABSTRACT renderer.
 *
 * A Renderer is a "visitor" in the Visitor pattern.  A specific
 * implementation if visitor is selected by the video driver, and
 * is then passed over the Widget hierarchy.  The renderer provides
 * the interface by which Buffer-to-Video transfers may occur.
 */

/* Signal that all processing for this renderer is complete. Renderers
 * that buffer information should flush the buffer here.
 */
void rendererComplete (struct Renderer *);

/* Destroy the renderer.  If the renderer has not been "Complete"d, then
 * the state of the display for the renderer's rectangle is undefined.
 */
void rendererDestroy (struct Renderer *);

/* Attempt to enter the region (X, Y, W, H).
 * Translate all further coordinates by DX and DY (until the corresponding
 * Leave).
 * Entering succeeds if there is a non-zero resulting region.
 *
 * Returns 1 if entering succeeds.
 */
int rendererEnter (struct Renderer *, const struct Rectangle *rect,
                   int dx, int dy);

/* Leave the last region enterered.
 */
void rendererLeave (struct Renderer *);

/* \brief attempt to render the buffer to the given co-ordinates
 *
 * \returns true if the buffer successfully rendered, or false if the
 *          renderer doesn't know how to deal with that kind of buffer
 */
bool rendererRenderBuffer (struct Renderer *, struct Buffer *, int x, int y);

/* Blit (blending) a rectangle of RGBA data.
 * 
 * The data should be blitted to the region-based co-ordinates (X,Y).
 * The data starts at the position given by DATA. The data consists of rows
 * of W pixels worth of RGBA colour data, with each row S pixels from the
 * previous. H rows should be blitted.
 */
void rendererBlitRGBAData (struct Renderer *, int x, int y, const uint32_t *data,
                           int w, int h, int s);

/* Draw a rectangle filled with a solid colour, COLOUR. The rectangle should
 * placed with the upper-left corner at the device independent co-ordinates
 * (X,Y), and be W by H in size.
 */
void rendererDrawFilledRectangle (struct Renderer *, uint32_t colour,
                                  int x, int y, int w, int h);

/* Stash string key/value pairs in the renderer
 */
void rendererSetOption (struct Renderer *self, const char *key, const char *value);
const char *rendererGetOption (struct Renderer *self, const char *key);

#endif /* Y_BUFFER_RENDERER_H */

/* arch-tag: 0888bf20-161a-421c-9257-1ee7c46c64cf
 */
