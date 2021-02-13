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

#include <Y/c++/connection.h>
#include <Y/c++/message.h>
#include <Y/c++/timer.h>

#include <pthread.h>

#include "thread_support.h"

/* NOTE:
 *
 * "nothreads" means "run without using multiple threads".
 *
 * It does not mean "you don't have to be thread safe". People can
 * still call functions from other threads, we just won't do it
 * ourselves.
 */

void
Y::Connection::run ()
{
  int oldtype;
  lock_mutex(state_mutex, oldtype);
  if (state != none)
    abort();
  state = running;
  unlock_mutex(oldtype);

  while (!stopping)
    {
      doPoll(100);
      doDispatch();
    }

  stopping = false;

  lock_mutex(state_mutex, oldtype);
  state = none;
  pthread_cond_broadcast(&state_cond);
  unlock_mutex(oldtype);
}

void
Y::Connection::doDispatch ()
{
  unbufferMessages();

  /* Note that while dispatching an event, no mutexes may be locked */

  /* This is disgusting. I can't use a simple while loop here, because
   * I can't lock the mutex while testing the condition. pthreads
   * sucks.
   */
  for (;;)
    {
      int oldtype;
      bool empty;
      lock_mutex(messages_mutex, oldtype);
      empty = messages.empty();
      unlock_mutex(oldtype);

      /* And I have to unlock the mutex here because of the stupid
       * nesting behaviour
       */

      if (empty)
        break;

      Message *m;
      lock_mutex(messages_mutex, oldtype);
      m = messages.front();
      messages.pop_front();
      unlock_mutex(oldtype);

      processMessage (m);
      delete m;
    }

  struct timeval now;
  gettimeofday (&now, NULL);

  /* Here we go again... */
  for (;;)
    {
      int oldtype;
      bool empty;
      lock_mutex(timers_mutex, oldtype);
      empty = timers.empty();
      unlock_mutex(oldtype);

      if (empty)
        break;

      Timer *timer = NULL;
      lock_mutex(timers_mutex, oldtype);
      std::set<TimeEvent>::iterator i = timers.begin();
      if (i->ready(now))
        {
          timer = i->timer;
          timers.erase(i);
        }
      unlock_mutex(oldtype);

      if (timer)
        timer->doTick();
      else
        break;
    }
}

void
Y::Connection::unbufferMessages ()
{
  /* This function should be fast, so we'll just lock the inbound
   * buffer for its entire duration. If it ever becomes slow, then
   * this will have to change.
   */
  int oldtype;
  lock_mutex(inbound_mutex, oldtype);
  uint32_t packet_len;
  while (Message *m = Message::parseStream(inbound_buffer))
    {
      if (debug_messages)
        {
          std::cerr << "Read message " << *m << std::endl;
        }

      int oldtype2;
      lock_mutex(messages_mutex, oldtype2);
      messages.push_back (m);
      unlock_mutex(oldtype2);
    }
  unlock_mutex(oldtype);
}

/* arch-tag: 7c9f1268-cecf-44ac-bcdb-d6e293dc83c0
 */
