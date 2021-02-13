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

#ifndef Y_CPP_OBJECT_H
#define Y_CPP_OBJECT_H

#include <Y/c++/connection.h>
#include <Y/c++/reply.h>
#include <Y/c++/message.h>
#include <Y/c++/exception.h>

#include <unistd.h>
#include <inttypes.h>

#include <sigc++/sigc++.h>

#include <string>
#include <vector>
#include <map>

namespace Y
{
  class Class;

  /** \brief Remote object superclass
   * \ingroup remote
   *
   * Every remote object should inherit from this class, and construct
   * it with the object's class name. It will then construct the
   * object on the server, asynchronously.
   */
  class Object : public SigC::Object
  {
    friend class Connection;

  protected:
    /** \brief Remote property
     * \ingroup remote
     *
     * This class provides easy access to remote object properties. It
     * cannot be instantiated directly; it should be constructed as a
     * member of the object it is associated with.
     */
    template <class T> class Property
    {
    public:
      Property(Object* o_, std::string n_) : o(o_), n(n_) {}

      /** \brief Remote property value
       * \ingroup remote
       *
       * A template thunk around a Y::Reply object; it behaves like
       * Y::Reply, but converts the reply tuple into the relevant type
       * for the property.
       */
      class Value
      {
        friend class Property;
      public:
        virtual ~Value ();

        bool hasValue() const {return rep->hasTuple();}
        T value()
        {
          if (rep)
            {
              v = getValue(rep->tuple());
              delete rep;
              rep = NULL;
            }
          return v;
        }
      private:
        Value(Reply *rep_) : rep(rep_) {}

        static T getValue(const Message::Members& m);

        Reply* rep;
        T v;
      };

      Object* object() const {return o;}
      std::string name() const {return n;}
      void set(const T value);
      Value* get();

    private:
      Object* o;
      std::string n;
    };

  public:
    virtual ~Object ();

    Class *c () const {return c_v;}
    uint32_t id () throw(Y::error);

    void subscribeSignal (const std::string &name);

    /* \todo Y::Object::parent shouldn't be public */
    Object *parent;

  protected:
    Object (Connection *y_, std::string className);

    Connection *y;

    virtual bool onEvent (const std::string &, const Y::Message::Members&) = 0;

    Reply* invokeMethod (const Y::Message::Members& params, bool expectReturn);
    Reply* invokeMethod (const std::string& name,
                         bool expectReturn);
    Reply* invokeMethod (const std::string& name,
                         const Y::Message::Member& a,
                         bool expectReturn);
    Reply* invokeMethod (const std::string& name,
                         const Y::Message::Member& a,
                         const Y::Message::Member& b,
                         bool expectReturn);
    Reply* invokeMethod (const std::string& name,
                         const Y::Message::Member& a,
                         const Y::Message::Member& b,
                         const Y::Message::Member& c,
                         bool expectReturn);
    Reply* invokeMethod (const std::string& name,
                         const Y::Message::Member& a,
                         const Y::Message::Member& b,
                         const Y::Message::Member& c,
                         const Y::Message::Member& d,
                         bool expectReturn);
    Reply* invokeMethod (const std::string& name,
                         const Y::Message::Member& a,
                         const Y::Message::Member& b,
                         const Y::Message::Member& c,
                         const Y::Message::Member& d,
                         const Y::Message::Member& e,
                         bool expectReturn);

  private:
    Reply *createReply;

    Class *c_v;
    uint32_t id_v;
  };
}

template <class T>
void
Y::Object::Property<T>::set(const T value)
{
  Y::Message::Members v;
  v.push_back("setProperty");
  v.push_back(name());
  v.push_back(value);
  object()->invokeMethod(v, false);
}

template <class T>
typename Y::Object::Property<T>::Value*
Y::Object::Property<T>::get()
{
  Y::Message::Members v;
  v.push_back("getProperty");
  v.push_back(name());
  return new Value(object()->invokeMethod(v, true));
}

template <class T>
Y::Object::Property<T>::Value::~Value()
{
  delete rep;
}

#endif

/* arch-tag: 5aeb402d-cbd4-4a3d-a5b5-6aa8cd7785b2
 */
