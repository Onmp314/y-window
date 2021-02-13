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

#include <Y/y.h>
#include <Y/main/paths.tab>

const char * yModuleDir = PKGLIBDIR;

const char * yVideoDriverDir = PKGLIBDIR "/driver/video";
const char * yInputDriverDir = PKGLIBDIR "/driver/input";
const char * yIPCDriverDir = PKGLIBDIR "/driver/ipc";
const char * yWindowManagerDir = PKGLIBDIR "/wm";
const char * yConfigDir = SYSCONFDIR "/Y";
const char * yDataDir = DATADIR "/Y";
const char * yImageDir = DATADIR "/Y/images";
const char * yPointerImageDir = DATADIR "/Y/images/pointers";

/* arch-tag: bc6ccd7c-cdff-43e5-a8bf-9d5f839098e2
 */
