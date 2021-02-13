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

#ifndef Y_TEXT_FONT_H
#define Y_TEXT_FONT_H

struct Font;

#include <Y/y.h>
#include <Y/main/config.h>
#include <Y/buffer/painter.h>

#include <stdlib.h>

void         fontInitialise (struct Config *serverConfig);
void         fontFinalise (void);

void         fontScanDirectory (const char *path, int recursiveLevels);

struct Font *fontCreate (const char *family, const char *style, int ptSize);
void         fontDestroy (struct Font *);


/*
 * General Font Properties:
 *
 *        ^
 *        |_____________________
 *        |     |\  /|  _   _        <---- ascent
 *        | \ | | \/ | / \ / \
 *        |  \| |    | \_/ \_/
 * origin +---|----------------->    <---- base line
 *        |_--'_________________     <---- descent
 *        |
 *          distance to next line's origin is the line gap.
 */

void         fontGetMetrics      (struct Font *, int *ascent_p, int *descent_p, int *linegap_p);


/* 
 * String-specific Properties
 *
 *        ^ offset
 *    --->|  |<---
 *        |  |    |\  /|  _   _ |  
 *        |  |\ | | \/ | / \ / \|
 *        |  | \| |    | \_/ \_/| (next origin)
 * origin +--|--|---------------|-+-->
 *        |  |--'               | | 
 *        |<------ width ------>| |
 *        |<------ advance ------>|
 *        
 */


void         fontMeasureWCString (struct Font *, const wchar_t *,
                                  int *offset_p, int *width_p, int *advance_p);

void         fontMeasureString (struct Font *, const char *,
                                int *offset_p, int *width_p, int *advance_p);

void         fontRenderWCString (struct Font *, struct Painter *,
                                 const wchar_t *, int x, int y); 

void         fontRenderString (struct Font *, struct Painter *,
                               const char *, int x, int y); 



#endif

/* arch-tag: 70521fef-5b13-43a6-9adb-1e688eb6abe2
 */
