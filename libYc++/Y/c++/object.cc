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

#include <Y/c++/class.h>
#include <Y/c++/object.h>
#include <Y/c++/connection.h>
#include <Y/c++/reply.h>

#include <string>

/** \defgroup remote Remote objects
 *
 * Each local object corresponds to one on the server, and provides an
 * interface for manipulating the remote object.
 *
 * Constructing an object locally will automatically construct a
 * corresponding object on the server.
 *
 * \note This is not a conventional RPC system. Most methods are
 * dispatched asynchronously, and results are accessed via a Y::Reply
 * object. All object constructors return asynchronously, but most
 * further operations on the object will block until it has been
 * constructed on the server.
 */

Y::Object::Object (Y::Connection *y_, std::string className)
  : y(y_), createReply(NULL), c_v(y->findClass(className)), id_v(0)
{
  createReply = c()->instantiate();
}

Y::Object::~Object ()
{
  y->destroyObject(id());
  delete createReply;
}

uint32_t
Y::Object::id () throw(Y::error)
{
  if (createReply)
    {
      if (createReply->op() == YMO_ERROR)
        throw error(createReply->tuple());
      Y::Message::Members res = createReply->tuple();
      id_v = res[0].uint32();
      delete createReply;
      createReply = NULL;
      y->createdObject(this);
    }
  return id_v;
}

void
Y::Object::subscribeSignal (const std::string &name)
{
  invokeMethod("subscribeSignal", name, false);
}

Y::Reply*
Y::Object::invokeMethod (const Y::Message::Members& params, bool expectReturn)
{
  Message req(0, 0, id(), YMO_INVOKE_INSTANCE_METHOD, expectReturn ? 0x01 : 0x00, params);

  return y->sendMessage (&req);
}

Y::Reply*
Y::Object::invokeMethod (const std::string& name, bool expectReturn)
{
  Y::Message::Members v;
  v.push_back(name);
  return invokeMethod(v, expectReturn);
}

Y::Reply*
Y::Object::invokeMethod (const std::string& name,
                         const Y::Message::Member& a,
                         bool expectReturn)
{
  Y::Message::Members v;
  v.push_back(name);
  v.push_back(a);
  return invokeMethod(v, expectReturn);
}

Y::Reply*
Y::Object::invokeMethod (const std::string& name,
                         const Y::Message::Member& a,
                         const Y::Message::Member& b,
                         bool expectReturn)
{
  Y::Message::Members v;
  v.push_back(name);
  v.push_back(a);
  v.push_back(b);
  return invokeMethod(v, expectReturn);
}

Y::Reply*
Y::Object::invokeMethod (const std::string& name,
                         const Y::Message::Member& a,
                         const Y::Message::Member& b,
                         const Y::Message::Member& c,
                         bool expectReturn)
{
  Y::Message::Members v;
  v.push_back(name);
  v.push_back(a);
  v.push_back(b);
  v.push_back(c);
  return invokeMethod(v, expectReturn);
}

Y::Reply*
Y::Object::invokeMethod (const std::string& name,
                         const Y::Message::Member& a,
                         const Y::Message::Member& b,
                         const Y::Message::Member& c,
                         const Y::Message::Member& d,
                         bool expectReturn)
{
  Y::Message::Members v;
  v.push_back(name);
  v.push_back(a);
  v.push_back(b);
  v.push_back(c);
  v.push_back(d);
  return invokeMethod(v, expectReturn);
}

Y::Reply*
Y::Object::invokeMethod (const std::string& name,
                         const Y::Message::Member& a,
                         const Y::Message::Member& b,
                         const Y::Message::Member& c,
                         const Y::Message::Member& d,
                         const Y::Message::Member& e,
                         bool expectReturn)
{
  Y::Message::Members v;
  v.push_back(name);
  v.push_back(a);
  v.push_back(b);
  v.push_back(c);
  v.push_back(d);
  v.push_back(e);
  return invokeMethod(v, expectReturn);
}

template <>
std::string
Y::Object::Property<std::string>::Value::getValue(const Y::Message::Members& m)
{
  return m.at(0).string();
}

/* arch-tag: fc2ef8c6-795a-484e-8b3a-73d63112d415
 */
