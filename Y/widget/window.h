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

#ifndef Y_WIDGET_WINDOW_H
#define Y_WIDGET_WINDOW_H

struct Window;

enum WindowReshapeMode
{
  WINDOW_RESHAPE_NONE,
  WINDOW_RESHAPE_MOVE,
  WINDOW_RESHAPE_RESIZE_NW, WINDOW_RESHAPE_RESIZE_N, WINDOW_RESHAPE_RESIZE_NE,
  WINDOW_RESHAPE_RESIZE_W,                           WINDOW_RESHAPE_RESIZE_E,
  WINDOW_RESHAPE_RESIZE_SW, WINDOW_RESHAPE_RESIZE_S, WINDOW_RESHAPE_RESIZE_SE
};

enum WindowSizeState
{
  WINDOW_SIZE_NORMAL, WINDOW_SIZE_MAXIMISE
};

#include <Y/y.h>
#include <Y/widget/widget.h>
#include <Y/buffer/painter.h>

struct Widget * windowToWidget  (struct Window *);
struct Object * windowToObject  (struct Window *);

struct Widget * windowGetChild  (struct Window *);

struct Widget * windowGetFocussedWidget  (struct Window *);
void            windowSetFocussedWidget  (struct Window *, struct Widget *);

void            windowStartReshape (struct Window *, int xHandle, int yHandle,
                                    enum WindowReshapeMode mode);
void            windowStopReshape (struct Window *);

void            windowSetSizeState (struct Window *, enum WindowSizeState);
enum WindowSizeState
                windowGetSizeState (struct Window *);

void            windowRequestClose (struct Window *);

void            windowSaveGeometry (struct Window *);
void            windowRestoreGeometry (struct Window *);

void windowSetChild    (struct Window *, struct Object *);
void windowSetFocussed (struct Window *, struct Object *);
void windowShow        (struct Window *);

#endif /* Y_WIDGET_WINDOW_H */

/* arch-tag: f4f25bca-aa44-4f8e-a586-2c0f0372f91d
 */
