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

#include <Y/c++/ticker.h>

void
Y::Ticker::start()
{
  if (running)
    return;
  running = true;
  set(interval);
}

void
Y::Ticker::stop()
{
  cancel();
  running = false;
}

void
Y::Ticker::onTick()
{
  if (running)
    set(interval);
}

/* arch-tag: dc1a1da6-c901-4f04-abd3-2624194eef84
 */
