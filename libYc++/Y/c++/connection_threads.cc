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

#include <iostream>
#include <pthread.h>

#include "thread_support.h"

void *
Y::Connection::dispatchThreadLauncher(void *arg)
{
  Y::Connection *y = reinterpret_cast<Y::Connection *>(arg);
  y->dispatchThreadMain();
  return NULL;
}

void
Y::Connection::runThreaded ()
{
  int oldtype;
  lock_mutex(state_mutex, oldtype);
  if (state != none)
    abort();
  state = threaded;
  unlock_mutex(oldtype);

  if (pthread_create(&dispatch_thread, NULL, &dispatchThreadLauncher, this) != 0)
    abort();

  pollThreadMain();

  stopping = false;

  if (pthread_join(dispatch_thread, NULL) != 0)
    abort();

  lock_mutex(state_mutex, oldtype);
  state = none;
  pthread_cond_broadcast(&state_cond);
  unlock_mutex(oldtype);
}

void
Y::Connection::stopThreaded ()
{
  stopping = true;

  /* If we are one of the threads associated with the connection,
   * terminate ourselves. We may not return.
   */
  if (pthread_equal(dispatch_thread, pthread_self()))
    pthread_exit(NULL);

  /* Wait for the other threads to shut down */
  int oldtype;
  lock_mutex(state_mutex, oldtype);
  while (state == running)
    pthread_cond_wait(&state_cond, &state_mutex);
  unlock_mutex(oldtype);
}

void
Y::Connection::pollThreadMain ()
{
  while (!stopping)
    {
      int oldtype;

      size_t old_length;

      lock_mutex(inbound_mutex, oldtype);
      old_length = inbound_buffer.length();
      unlock_mutex(oldtype);

      doPoll(100);

      bool signal_dispatch = false;

      lock_mutex(inbound_mutex, oldtype);
      if (inbound_buffer.length() > old_length)
        signal_dispatch = true;
      unlock_mutex(oldtype);
      struct timeval now;
      gettimeofday (&now, NULL);

      lock_mutex(timers_mutex, oldtype);
      if (!timers.empty() && timers.begin()->ready(now))
        signal_dispatch = true;
      unlock_mutex(oldtype);

      if (signal_dispatch)
        {
          lock_mutex(dispatch_mutex, oldtype);
          pthread_cond_signal(&dispatch_cond);
          unlock_mutex(oldtype);
        }
    }
}

void
Y::Connection::dispatchThreadMain ()
{
  while (!stopping)
    {
      for(;;)
        {
          int oldtype;
          Message *m;
          lock_mutex(inbound_mutex, oldtype);
          m = Message::parseStream(inbound_buffer);
          unlock_mutex(oldtype);

          if (m == NULL)
            break;
            
          if (debug_messages)
            {
              std::cerr << "Read message " << *m << std::endl;
            }

          lock_mutex(messages_mutex, oldtype);
          messages.push_back (m);
          unlock_mutex(oldtype);
        }

      for (;;)
        {
          int oldtype;
          bool empty;
          lock_mutex(messages_mutex, oldtype);
          empty = messages.empty();
          unlock_mutex(oldtype);

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

      /* Note that we don't have a while loop here. The actual
       * conditional is really complicated, but it doesn't really
       * matter if we go around the loop when we don't need to, so
       * long as it doesn't happen very often. So, we leave it to the
       * polling loop to figure out when to wake us up, and then we
       * just wait once.
       *
       * This is a minor abuse of conditional waits, but it's the best
       * I can come up with for pthreads.
       */
      int oldtype;
      lock_mutex(dispatch_mutex, oldtype);
      pthread_cond_wait(&dispatch_cond, &dispatch_mutex);
      unlock_mutex(oldtype);
    }
}

/* arch-tag: d8ec21f9-5023-4f91-8e6c-a4e43a982e36
 */
