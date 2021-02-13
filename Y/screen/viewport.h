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

#ifndef Y_SCREEN_VIEWPORT_H
#define Y_SCREEN_VIEWPORT_H

struct Viewport;

#include <Y/modules/videodriver_interface.h>
#include <Y/util/llist.h>
#include <Y/util/rectangle.h>

/*
 * A Viewport is the concept of a view onto the abstract display.
 *
 * A viewport represents a rectangle which is held by the screen as one
 * of the objects that actually display things. A viewport knows its
 * video driver, which is the object that does the actual work.
 */

/* Create a new Viewport object that is referred to by DRIVER. */
struct Viewport  *viewportCreate (struct VideoDriver *driver);

/* Destroy a Viewport. */
void              viewportDestroy (struct Viewport *);

/* Get the unique identifier for a viewport. */
int               viewportGetID (const struct Viewport *);

/* Get the name of the video driver connected to the viewport. */
const char *      viewportGetName (struct Viewport *);

/* Request the resolutions that the viewport can handle. */
struct llist *    viewportGetResolutions (struct Viewport *);
/* returns an llist of struct VideoResolution */

/* Ask the viewport to set its resolution to a resolution. */
void              viewportSetResolution (struct Viewport *, const char *name);

/* Get the device independent co-ordinates of the Viewport. */
struct Rectangle *viewportGetRectangle (struct Viewport *);

/* Invalidate a region of the viewport. */
void              viewportInvalidateRectangle (struct Viewport *,
                                               const struct Rectangle *);

void              viewportSetSize (struct Viewport *, int, int);

/* Cause the viewport to update itself. */ 
void              viewportUpdate (struct Viewport *);

struct Tuple *    viewportCall (struct Viewport *, const struct Tuple *);

#endif

/* arch-tag: d1442762-99d7-47b6-9bfd-45d5b96a72b6
 */
