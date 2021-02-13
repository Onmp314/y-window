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

#ifndef Y_MODULES_MODULE_H
#define Y_MODULES_MODULE_H

#include <Y/y.h>
#include <Y/main/config.h>

void  moduleInitialise (struct Config *serverConfig);
void  moduleFinalise (void);

void  moduleLoad   (const char *name, const struct Tuple *args);
void  moduleUnload (const char *name);

#endif /* header guard */


/* arch-tag: 648221aa-fb95-4d45-992f-bd9addfd9c74
 */
