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

#include <Y/c++/class.h>
#include <Y/c++/connection.h>
#include <Y/c++/message.h>

Y::Class::Class(Y::Connection *y_, std::string name_)
  : y(y_), findReply(NULL), name_v(name_), id_v(0)
{
  Y::Message::Members v;
  v.push_back(name());
  Message req(0, 0, 0, YMO_FIND_CLASS, 0, v);
  findReply = y->sendMessage(&req);
}

Y::Class::~Class()
{
  delete findReply;
}

uint32_t
Y::Class::id() const
{
  if (findReply)
    {
      id_v = findReply->id();
      delete findReply;
      findReply = NULL;
    }
  return id_v;
}

Y::Reply*
Y::Class::instantiate (const Y::Message::Members& params)
{
  Y::Message::Members v;
  /* Name of the constructor is the name of the class */
  v.push_back(name());
  /* And then come all the parameters */
  v.insert(v.end(), params.begin(), params.end());

  return invokeMethod (v, true);
}

Y::Reply*
Y::Class::invokeMethod (const Y::Message::Members& params, bool expectReturn)
{
  Message req(0, 0, id(), YMO_INVOKE_CLASS_METHOD, expectReturn ? 0x01 : 0x00, params);

  return y->sendMessage (&req);
}

Y::Reply*
Y::Class::invokeMethod (const std::string& name, bool expectReturn)
{
  Y::Message::Members v;
  v.push_back(name);
  return invokeMethod(v, expectReturn);
}

/* arch-tag: 763ef839-5c17-4c29-a442-50f9ce79417f
 */
