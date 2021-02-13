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

#ifndef Y_CPP_MESSAGE_H
#define Y_CPP_MESSAGE_H

#include <iostream>
#include <vector>
#include <string>

#include <inttypes.h>

#include <Y/const.h>

namespace Y
{
  /** \brief Protocol message
   * \ingroup comm
   */
  class Message
  {
  public:
    /** \brief Value in a tuple
     */
    class Member
    {
      friend class Message;
    public:
      enum memberType
        {
          t_string, t_uint32, t_int32
        };

      Member(uint32_t i) : type_v(t_uint32), uint32_v(i) {}
      Member(int32_t i) : type_v(t_int32), int32_v(i) {}
      Member(std::string s) : type_v(t_string), s_v(s) {}
      Member(const char *s) : type_v(t_string), s_v(std::string(s)) {}

      enum memberType type() const {return type_v;}

      const std::string& string() const {return s_v;}
      uint32_t uint32() const {return uint32_v;}
      int32_t int32() const {return int32_v;}

      bool isstring() const {return type() == t_string;}
      bool isuint32() const {return type() == t_uint32;}

      void serialise(std::string& buffer) const;

    private:
      static Member parse(const std::string &str);

      enum memberType type_v;
      std::string s_v;
      uint32_t uint32_v;
      int32_t int32_v;
    };

    /** \brief Message tuple
     */
    class Members : public std::vector<Member>
    {
    public:
      std::string pretty() const;
    };
    
    /* This is for parsing received messages */
    static Message* parseStream(std::string &str);

    /* This is for creating messages to send */
    Message (uint32_t to_, uint32_t from_, uint32_t id_, enum YMessageOperation op_, uint32_t meta_,
             const Members& tuple_ = Members())
      {
        v.to = to_;
        v.from = from_;
        v.id = id_;
        v.op = op_;
        v.meta = meta_;
        v.seq = nextSeq();
        v.tuple = tuple_;
      }

    void serialise(std::string& buffer) const;
    bool expectReply() const;

    enum YMessageOperation op() const {return static_cast<enum YMessageOperation>(v.op);}
    uint32_t to() const {return v.to;}
    uint32_t from() const {return v.from;}
    uint32_t id() const {return v.id;}
    uint32_t meta() const {return v.meta;}
    uint32_t seq() const {return v.seq;}
    const Members& tuple () const {return v.tuple;}

  private:
    Message (const std::string& buffer);

    static uint32_t nextSeq();

    struct
    {
      uint32_t seq, to, from;
      uint32_t op;
      uint32_t id, meta;
      Members tuple;
    } v;
  };
}

extern std::ostream& operator<< (std::ostream& strm, enum YMessageOperation op);
extern std::ostream& operator<< (std::ostream& strm, const Y::Message m);

extern std::ostream& operator<< (std::ostream& strm, const Y::Message::Members m);
extern std::ostream& operator<< (std::ostream& strm, const Y::Message::Member m);

#endif

/* arch-tag: 4c8f1758-d923-4a3c-bce2-68dd7b29f2c9
 */
