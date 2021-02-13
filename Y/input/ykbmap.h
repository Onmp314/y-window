/************************************************************************
 *   Copyright (C) Andrew Suffield <asuffield@debian.org>
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

#ifndef Y_INPUT_YKBMAP_H
#define Y_INPUT_YKBMAP_H

struct ykbMapSet;

extern struct ykbMapSet *ykbLoadMapSet(const char *path);
extern void ykbMapSetDestroy(struct ykbMapSet *mapset);

#endif /* header guard */

/* arch-tag: 3f4e290b-8f1c-4fcf-8b83-2a0d94d99a2e
 */
