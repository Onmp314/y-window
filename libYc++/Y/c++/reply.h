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

#ifndef Y_CPP_REPLY_H
#define Y_CPP_REPLY_H

#include <iostream>
#include <vector>
#include <string>

#include <Y/const.h>
#include <Y/c++/connection.h>
#include <Y/c++/message.h>

#include <pthread.h>

namespace Y
{
  class Connection;

  /** \brief %Message reply thunk
   * \ingroup comm
   */
  class Reply
  {
    friend class Connection;

  public:
    virtual ~Reply();

    const Y::Message::Members& tuple() {wait(); return v.tuple;}
    uint32_t id() {wait(); return v.id;}
    uint32_t meta() {wait(); return v.meta;}
    enum YMessageOperation op() {wait(); return v.op;}
    bool hasTuple() const {return got_tuple;}

  private:
    Reply(Connection *y_, uint32_t seq_) : y(y_), seq(seq_), got_tuple(false)
      {
        pthread_mutex_init(&got_tuple_mutex, NULL);
        pthread_cond_init(&got_tuple_cond, NULL);
      }

    void wait();
    void dispatch(Message* reply);

    Connection *y;
    uint32_t seq;

    bool got_tuple;
    pthread_mutex_t got_tuple_mutex;
    pthread_cond_t got_tuple_cond;

    struct
    {
      enum YMessageOperation op;
      uint32_t id, meta;
      Y::Message::Members tuple;
    } v;
  };
}

extern std::ostream& operator<< (std::ostream& strm, struct Y::Reply r);

#endif

/* arch-tag: f1bafccd-c755-4c65-a041-fdf20195cb25
 */
