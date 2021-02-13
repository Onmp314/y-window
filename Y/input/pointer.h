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

#ifndef Y_CONTROL_POINTER_H
#define Y_CONTROL_POINTER_H

#define POINTER_NUM_BUTTONS 12

#include <Y/y.h>
#include <Y/widget/widget.h>
#include <Y/screen/renderer.h>

void pointerGetPosition (int *xp, int *yp);

void pointerSetPosition (int x, int y);
void pointerMovePosition (int dx, int dy);

void pointerButtonChange (int button, int pressed);

void pointerGrab (struct Widget *);
void pointerRelease (void);

struct Buffer *pointerGetCurrentImage (void);

void pointerRender (struct Renderer *);


#endif /* Y_CONTROL_POINTER_H */

/* arch-tag: 13970e73-14d6-4836-af68-2c7105fd1b94
 */
