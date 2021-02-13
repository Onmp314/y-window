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

#ifndef Y_CPP_TIMER_H
#define Y_CPP_TIMER_H

#include <Y/c++/connection.h>
#include <sigc++/sigc++.h>

namespace Y
{
  /** \brief Egg timer
   * \ingroup local
   *
   * A timer will signal a tick once, when a given duration has
   * elapsed. Timers are handled by the Y::Connection object they are
   * associated with, so the connection must be running for them to
   * tick.
   *
   * \note This is not a high-resolution timer; it is accurate to
   * within no more than a few hundred milliseconds. It is intended to
   * provide a simple interface for applications that need
   * low-resolution timers, similar to sleep().
   */
  class Timer
  {
    friend class Connection;
  public:
    Timer (Y::Connection *y_) : y(y_), e(NULL) {}
    Timer (Y::Connection *y_, long unsigned int msec) : y(y_), e(NULL) {set(msec);}
    virtual ~Timer ();

    void set (long unsigned int msec);
    void cancel ();

    /** Signalled when the timer has elapsed */
    SigC::Signal0<void> tick;
    virtual void onTick ();

  private:
    Y::Connection *y;
    Y::Connection::TimeEvent *e;

    void doTick ();
  };
}

#endif

/* arch-tag: 1a808160-c07c-4466-a0bf-8d6ac92309e4
 */
