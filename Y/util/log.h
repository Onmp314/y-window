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

#ifndef Y_UTIL_LOG_H
#define Y_UTIL_LOG_H

#include <Y/y.h>
#include <syslog.h>

void ylogInit(void);
void ylogClose(void);

void ylog(int priority, const char *str, ...) __attribute__((format(printf,2,3)));

#define YLOG_DEBUG LOG_DEBUG
#define YLOG_TRACE LOG_INFO
#define YLOG_INFO LOG_INFO
#define YLOG_WARNING LOG_NOTICE
#define YLOG_ERR LOG_WARNING
#define YLOG_FATAL LOG_ERR

#define Y_SILENT(str, ...)   
#define Y_DEBUG(str, ...)  ylog (LOG_DEBUG,   "%s:%d: " str "\n", __FUNCTION__, __LINE__ ,##__VA_ARGS__ )
#define Y_TRACE(str, ...)  ylog (LOG_INFO,    "%s:%d: " str "\n", __FUNCTION__, __LINE__ ,##__VA_ARGS__ )
#define Y_INFO(str, ...)   ylog (LOG_INFO,              str "\n"                         ,##__VA_ARGS__ )
#define Y_WARN(str, ...)   ylog (LOG_NOTICE,  "Warning: %s: "     str "\n", __FUNCTION__ ,##__VA_ARGS__ )
#define Y_ERROR(str, ...)  ylog (LOG_WARNING, "Error: %s: "       str "\n", __FUNCTION__ ,##__VA_ARGS__ )
#define Y_FATAL(str, ...)  ylog (LOG_ERR,     "Fatal Error: %s: " str "\n", __FUNCTION__ ,##__VA_ARGS__ )

#endif /* header guard */

/* arch-tag: 44a7ecc0-bdfa-434d-a47e-47fd7435fc9a
 */
