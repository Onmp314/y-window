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

#ifndef Y_CPP_CLASS_H
#define Y_CPP_CLASS_H

#include <Y/c++/connection.h>
#include <Y/c++/reply.h>

#include <string>

namespace Y
{
  class Object;
  /** \brief Remote class
   * \ingroup comm
   *
   * Provides an interface for manipulating a remote class.
   *
   * To instantiate this class, use Y::Connection::findClass. To
   * instantiate objects of the remote class this represents, use a
   * subclass of Y::Object.
   */
  class Class
  {
    friend class Y::Connection;
    friend class Y::Object;
  public:
    virtual ~Class();

    uint32_t id() const;
    const std::string& name() const {return name_v;}

    bool operator<(const Y::Class c) {return id() < c.id();}
    bool operator==(const Y::Class c) {return id() == c.id();}

    Reply* invokeMethod (const Y::Message::Members& params, bool expectReturn);
    Reply* invokeMethod (const std::string& name,
                         bool expectReturn);

  private:
    Class (Connection *y_, std::string name_);

    Reply* instantiate (const Y::Message::Members& params = Y::Message::Members());

    Connection *y;
    mutable Reply *findReply;
    std::string name_v;
    mutable uint32_t id_v;
  };
}

#endif

/* arch-tag: 3badbb35-bb69-493d-b4dd-ce3c70198b34
 */
