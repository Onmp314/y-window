/************************************************************************
 *   Copyright (C) Mark Thomas <markbt@efaref.net>
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
#include <Y/c++/object.h>
#include <Y/c++/message.h>
#include <Y/c++/reply.h>
#include <Y/c++/timer.h>

#include <Y/const.h>

#include <iostream>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include "thread_support.h"

/** \defgroup comm Network communication
 *
 * These classes are used to communicate with the server.
 */

/** \defgroup local Local objects
 *
 * These classes do not exist on the server; they are provided to
 * simplify application programming.
 */

Y::Connection::Connection ()
{
  /* Initialise the privates */
  server_fd = -1;
  debug_io = false;
  debug_messages = false;
  pollfd_list = NULL;
  working_pollfd_list = NULL;
  stopping = false;

  /* pthreads stuff */
  pthread_mutex_init(&state_mutex, NULL);
  pthread_mutex_init(&pollfd_list_mutex, NULL);
  pthread_mutex_init(&fds_mutex, NULL);
  pthread_mutex_init(&inbound_mutex, NULL);
  pthread_mutex_init(&outbound_mutex, NULL);
  pthread_mutex_init(&timers_mutex, NULL);
  pthread_mutex_init(&replies_mutex, NULL);
  pthread_mutex_init(&objects_mutex, NULL);
  pthread_mutex_init(&classes_mutex, NULL);
  pthread_mutex_init(&messages_mutex, NULL);
  pthread_mutex_init(&dispatch_mutex, NULL);
  pthread_cond_init(&dispatch_cond, NULL);
  pthread_cond_init(&state_cond, NULL);
  state = none;

  const char *debug = getenv("YDEBUG");
  if (debug && strstr(debug, "messages"))
    {
      debug_messages = true;
    }
  if (debug && strstr(debug, "io"))
    {
      debug_io = true;
    }

  const char *display = getenv ("YDISPLAY");

  if (display == NULL)
    {
      std::cerr << "YDISPLAY not set, can't find a server" << std::endl;
      abort();
    }

  if (strncmp (display, "tcp:", 4) == 0)
    tcpInitialise(display + 4);
  else if (strncmp (display, "unix:", 5) == 0)
    unixInitialise(display + 5);
  else
    {
      /* Throw an exception here instead */
      std::cerr << "Could not identify protocol in $YDISPLAY" << std::endl;
      abort();
    }

  setNonBlocking(server_fd);

  updateFDList();
}

Y::Connection::~Connection ()
{
  stop();

  if (server_fd != -1)
    close(server_fd);
  server_fd = -1;
  outbound_buffer = "";
  updateFDList();

  for (std::map<uint32_t, Object *>::iterator i = objects.begin(); i != objects.end(); i++)
    delete i->second;

  for (std::set<TimeEvent>::iterator i = timers.begin(); i != timers.end(); i++)
    delete i->timer;

  if (pollfd_list)
    delete[] pollfd_list;
  if (working_pollfd_list)
    delete[] working_pollfd_list;
}

Y::Connection::TimeEvent*
Y::Connection::setTimer (int msec, Y::Timer* timer)
{
  struct timeval now;
  gettimeofday (&now, NULL);

  struct timeval then;
  then.tv_usec = (now.tv_usec + msec * 1000) % 1000000;
  then.tv_sec = now.tv_sec + (now.tv_usec + msec * 1000) / 1000000;

  TimeEvent *event = new TimeEvent(then, timer);

  int oldtype;
  lock_mutex(timers_mutex, oldtype);
  timers.insert(*event);
  unlock_mutex(oldtype);

  return event;
}

void
Y::Connection::unsetTimer (Y::Connection::TimeEvent* event)
{
  if (!event)
    return;

  int oldtype;
  lock_mutex(timers_mutex, oldtype);
  timers.erase(*event);
  unlock_mutex(oldtype);

  delete event;
}

void
Y::Connection::processMessage (Message *m)
{
  switch(m->op())
    {
    case YMO_EVENT:
      if (m -> id() != 0)
        {
          Object *object = findObject (m -> id());
          Y::Message::Members params = m->tuple();

          if (!params.empty () && params[0].isstring())
            {
              std::string name = params[0].string();
              params.erase (params.begin ());
              
              while (object != NULL)
                {
                  if (object -> onEvent (name, params))
                    break;
                  object = object -> parent;
                }
            }
        }
      break;
    default:
      {
        if (m->seq() == 0)
          return;

        int oldtype;
        lock_mutex(replies_mutex, oldtype);
        std::map<uint32_t, Reply *>::iterator i = replies.find(m->seq());
        if (i != replies.end())
          {
            Reply *r = i->second;
            r->dispatch(m);
            /* We leave the reply in the map for now - it'll get
             * removed when it's deleted. A map shouldn't have
             * saturation issues, and it allows us to free them all
             * automatically when the connection is destroyed
             */
          }
        unlock_mutex(oldtype);
      }
    }
}

void
Y::Connection::detachReply (uint32_t seq)
{
  int oldtype;
  lock_mutex(replies_mutex, oldtype);
  std::map<uint32_t, Reply *>::iterator i = replies.find(seq);
  if (i != replies.end())
    {
      replies.erase(i);
    }
  unlock_mutex(oldtype);
}

void
Y::Connection::stop ()
{
  int oldtype;
  lock_mutex(state_mutex, oldtype);
  switch (state)
    {
    case none:
    case polling:
      break;
    case running:
      stopping = true;
      break;
    case threaded:
      stopThreaded();
      break;
    }
  unlock_mutex(oldtype);
}

void
Y::Connection::poll (int poll_timeout)
{
  bool skip = false, was_none = false;
  int oldtype;

  lock_mutex(state_mutex, oldtype);
  if (state == none)
    {
      state = polling;
      was_none = true;
    }
  else if (state == threaded)
    skip = true;
  unlock_mutex(oldtype);

  if (skip)
    return;

  doPoll(poll_timeout);
  doDispatch();

  if (was_none)
    {
      int oldtype;
      lock_mutex(state_mutex, oldtype);
      state = none;
      pthread_cond_broadcast(&state_cond);
      unlock_mutex(oldtype);
    }
}

Y::Reply *
Y::Connection::sendMessage (const Message *m)
{
  if (server_fd == -1)
    return NULL;

  if (debug_messages)
    {
      std::cerr << "Buffering message " << *m << std::endl;
    }

  size_t old_length = 0;

  int oldtype;
  lock_mutex(outbound_mutex, oldtype);
  if (debug_io)
    old_length = outbound_buffer.length();
  m->serialise(outbound_buffer);
  unlock_mutex(oldtype);

  lock_mutex(pollfd_list_mutex, oldtype);
  pollfd_list[pollfds - 1].events |= POLLOUT;
  unlock_mutex(oldtype);

  if (debug_io)
    {
      lock_mutex(outbound_mutex, oldtype);
      std::cerr << "Outbound buffer contains " << outbound_buffer.length() << " bytes (added "
                << (outbound_buffer.length() - old_length) << ")" << std::endl;
      unlock_mutex(oldtype);
    }

  if (m->expectReply() && m->seq() > 0)
    {
      Reply *r = new Reply(this, m->seq());
      lock_mutex(replies_mutex, oldtype);
      replies[m->seq()] = r;
      unlock_mutex(oldtype);
      return r;
    }
  else
    {
      return NULL;
    }
}

/* arch-tag: 39a41201-2e34-48ef-aa33-9c6cc5b2c74b
 */
