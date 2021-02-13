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

#ifndef Y_INPUT_KEYMAP_H
#define Y_INPUT_KEYMAP_H

#include <Y/y.h>
#include <Y/const.h>
#include <Y/main/config.h>

void keymapInitialise (struct Config *serverConfig);
void keymapFinalise (void);

void keymapLoad (const char *keymap);

enum YKeyCode keymapMapKey (enum YKeyCode key, int modifierState);

#endif /* header guard */

/* arch-tag: e5b9a5c8-911a-4aa6-afc9-75c98409bd4d
 */
