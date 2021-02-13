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

#ifndef Y_MODULES_MODULE_INTERFACE_H
#define Y_MODULES_MODULE_INTERFACE_H

#include <Y/y.h>
#include <Y/message/tuple.h>
#include <Y/modules/module.h>

/* This is the interface that all modules should implement */

struct Module
{
  char *moduleName;
  char *name;
  void *handle;
  void *data;
};

int initialise (struct Module *, const struct Tuple *);
int finalise   (struct Module *);

#endif

/* arch-tag: 4a52d978-8c2b-4664-979a-967527284ec7
 */
