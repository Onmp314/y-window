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

#ifndef Y_MAIN_CONFIG_H
#define Y_MAIN_CONFIG_H

#include <Y/message/tuple.h>

struct Config;
struct ConfigKeyIterator;

struct Config *configRead (const char *filename);
void configDestroy (struct Config *conf);

struct Tuple *configGet(const struct Config *conf, const char *group, const char *key, const struct TupleType *type);
struct ConfigKeyIterator *configGetKeyIterator(const struct Config *conf, const char *group);
void configKeyIteratorNext(struct ConfigKeyIterator *i);
bool configKeyIteratorHasValue(struct ConfigKeyIterator *i);
const char *configKeyIteratorName(struct ConfigKeyIterator *i);
struct Tuple *configKeyIteratorValue(struct ConfigKeyIterator *i, const struct TupleType *type);
void configKeyIteratorDestroy(struct ConfigKeyIterator *i);

#endif

/* arch-tag: 65322c84-75cc-45bf-8deb-b22722c66fc4
 */
