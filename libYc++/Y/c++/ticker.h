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

#ifndef Y_CPP_TICKER_H
#define Y_CPP_TICKER_H

#include <Y/c++/connection.h>
#include <Y/c++/timer.h>

namespace Y
{
  /** \brief Repeating timer
   * \ingroup local
   *
   * A timer that signals a tick regularly, at a given interval.
   */
  class Ticker : public Timer
  {
  public:
    Ticker (Y::Connection *y, long unsigned int msec, bool autostart = true) : Timer(y), interval(msec), running(false) {if (autostart) start();}

    void start();
    void stop();

    virtual void onTick ();

  private:
    long unsigned int interval;
    bool running;
  };
}

#endif

/* arch-tag: 7650e699-61e3-4944-91ea-ff933c034be4
 */
