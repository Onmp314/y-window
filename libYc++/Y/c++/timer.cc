/************************************************************************
 *   Copyright (C) Andrew Suffield <asuffield@debian.org>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <Y/c++/timer.h>
#include <Y/c++/connection.h>

void
Y::Timer::onTick ()
{
}

void
Y::Timer::doTick ()
{
  delete e;
  e = NULL;
  onTick();
  tick();
}

void
Y::Timer::set (long unsigned int msec)
{
  if (e)
    cancel();
  e = y->setTimer(msec, this);
}

void
Y::Timer::cancel ()
{
  if (e)
    y->unsetTimer(e);
}

Y::Timer::~Timer ()
{
  cancel();
}

/* arch-tag: 901e08f5-fdc7-49bd-b2b7-2f641b8d62eb
 */
