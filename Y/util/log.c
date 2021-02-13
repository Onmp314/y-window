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

#include <Y/util/log.h>
#include <syslog.h>
#include <stdarg.h>

void
ylogInit()
{
  openlog ("Y", LOG_CONS|LOG_NDELAY|LOG_PID, LOG_LOCAL7);
}

void
ylogClose()
{
  closelog();
}

void
ylog(int priority, const char *str, ...)
{
  va_list args;

  va_start(args, str);
  vsyslog(priority, str, args);
  va_end(args);
}

/* arch-tag: f25916ea-048b-4b38-8356-e246af4fb9b2
 */
