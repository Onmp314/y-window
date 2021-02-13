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

#include <iostream>

#include <Y/c++/connection.h>
#include <Y/c++/reply.h>
#include <Y/c++/message.h>

#include <assert.h>

#include "thread_support.h"

Y::Reply::~Reply()
{
  y->detachReply(seq);
}

void
Y::Reply::dispatch(Message* reply)
{
  assert(reply->seq() == seq);

  int oldtype;
  lock_mutex(got_tuple_mutex, oldtype);
  v.id = reply->id();
  v.op = reply->op();
  v.meta = reply->meta();
  v.tuple = reply->tuple();
  got_tuple = true;
  pthread_cond_broadcast(&got_tuple_cond);
  unlock_mutex(oldtype);
}

/* There is a partial failure case here when somebody calls y->stop()
 * on a threaded connection. In that case, this function will stall
 * until something else calls run()/runThreaded()/poll() and its reply
 * is delivered - it won't fall back to polling automatically. I judge
 * this case to be too obscure to be worth supporting directly.
 */
void
Y::Reply::wait()
{
  if (got_tuple)
    return;

  bool use_threads;
  int oldtype;
  lock_mutex(y->state_mutex, oldtype);
  use_threads = y->state == Y::Connection::threaded;
  unlock_mutex(oldtype);

  if (use_threads)
    {
      int oldtype;
      lock_mutex(got_tuple_mutex, oldtype);
      while (!got_tuple)
        pthread_cond_wait(&got_tuple_cond, &got_tuple_mutex);
      unlock_mutex(oldtype);
    }
  else
    {
      while (!got_tuple)
        y->poll(10);
    }
}

std::ostream&
operator<< (std::ostream& strm, struct Y::Reply r)
{
  strm << "<";
  for (Y::Message::Members::const_iterator i = r.tuple().begin(); i != r.tuple().end(); i++)
    {
      if (i != r.tuple().begin())
        strm << ", ";
          
      strm << '"' << *i << '"';
    }
  strm << ">";

  return strm;
}

/* arch-tag: bc65d7d3-0427-4bd0-abef-706803e1a3a9
 */
