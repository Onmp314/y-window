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
#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>

#include <Y/c++/connection.h>
#include <Y/c++/message.h>

#include <unistd.h>
#include <netinet/in.h>
#include <assert.h>

uint32_t
Y::Message::nextSeq()
{
  static int next = 1;
  next++;
  /* Make sure that when we wrap, we do it safely */
  if (next == 0)
    next++;
  return next;
}

Y::Message*
Y::Message::parseStream(std::string& buffer)
{
  uint32_t npacket_len;
  if (buffer.length() < sizeof(npacket_len))
    return NULL;

  buffer.copy((char *)&npacket_len, sizeof(npacket_len));
  uint32_t packet_len = ntohl(npacket_len);
  if (buffer.length() < packet_len)
    return NULL;

  std::string packet = buffer.substr(sizeof(npacket_len), packet_len);
  buffer.erase(0, sizeof(npacket_len));
  buffer.erase(0, packet_len);

  return new Y::Message::Message(packet);
}

/* Most annoyingly, std::string::copy only throws an exception if idx
 * is past the end; if I want to check len, I must check the return
 * value.
 */
static inline std::string::size_type
safe_copy(const std::string& str, char *buf, std::string::size_type len, std::string::size_type idx)
  throw(std::out_of_range)
{
  std::string::size_type copied = str.copy(buf, len, idx);
  if (copied < len)
    throw std::out_of_range("Y::Message/safe_copy(std::string&, ...)");
  return copied;
}

#define do_safe_copy(B, S, P) safe_copy((B), (char*)&(S), sizeof(S), (P))

Y::Message::Message (const std::string& buffer)
{
  uint32_t nto;
  uint32_t nfrom;
  uint32_t nid;
  uint32_t nop;
  uint32_t nmeta;
  uint32_t nseq;
  uint32_t nmembers, members;

  size_t pos = 0;

  pos += do_safe_copy(buffer, nseq, pos);
  pos += do_safe_copy(buffer, nto, pos);
  pos += do_safe_copy(buffer, nfrom, pos);
  pos += do_safe_copy(buffer, nop, pos);
  pos += do_safe_copy(buffer, nid, pos);
  pos += do_safe_copy(buffer, nmeta, pos);
  pos += do_safe_copy(buffer, nmembers, pos);

  members = ntohl(nmembers);
  v.seq = ntohl(nseq);
  v.to = ntohl(nto);
  v.from = ntohl(nfrom);
  v.id = ntohl(nid);
  v.op = ntohl(nop);
  v.meta = ntohl(nmeta);

  for (uint32_t i = 0; i < members; i++)
    {
      uint32_t nvlen;
      pos += do_safe_copy(buffer, nvlen, pos);
      uint32_t vlen = ntohl(nvlen);
      std::string vstr = buffer.substr(pos, vlen);
      pos += vlen;
      if (buffer.length() < pos)
        throw std::out_of_range("Y::Message::Message(const std::string&) (member not all present)");
      v.tuple.push_back(Message::Member::parse(vstr));
    }

  assert(pos <= buffer.length());

  if (pos != buffer.length())
    throw std::out_of_range("Y::Message::Message(const std::string&) (too much data at end of packet)");
}

void
Y::Message::serialise(std::string& buffer) const
{
  uint32_t nseq  = htonl (v.seq);
  uint32_t nto   = htonl (v.to);
  uint32_t nfrom = htonl (v.from);
  uint32_t nop   = htonl (v.op);
  uint32_t nid   = htonl (v.id);
  uint32_t nmeta = htonl (v.meta);
  uint32_t nmembers = htonl(v.tuple.size());

  std::string tmp = "";
  tmp.append ((char *)&nseq,  sizeof(nseq));
  tmp.append ((char *)&nto,   sizeof(nto));
  tmp.append ((char *)&nfrom, sizeof(nfrom));
  tmp.append ((char *)&nop,   sizeof(nop));
  tmp.append ((char *)&nid,   sizeof(nid));
  tmp.append ((char *)&nmeta, sizeof(nmeta));
  tmp.append ((char *)&nmembers, sizeof(nmembers));

  for (Members::const_iterator i = v.tuple.begin(); i < v.tuple.end(); i++)
    i->serialise(tmp);

  uint32_t npacket_len = htonl(tmp.length());
  buffer.append ((char *)&npacket_len, sizeof(npacket_len));
  buffer.append (tmp);
}

Y::Message::Member
Y::Message::Member::parse(const std::string& buffer)
{
  uint32_t ntype;

  size_t pos = 0;

  pos += do_safe_copy(buffer, ntype, pos);

  enum memberType type = static_cast<enum memberType>(ntohl(ntype));

  switch(type)
    {
    case t_string:
      return Member(buffer.substr(pos));
    case t_uint32:
      uint32_t nuint32;
      pos += do_safe_copy(buffer, nuint32, pos);
      return Member(static_cast<uint32_t>(htonl(nuint32)));
    case t_int32:
      int32_t nint32;
      pos += do_safe_copy(buffer, nint32, pos);
      return Member(static_cast<int32_t>(htonl(nint32)));
    default:
      abort();
    }
}

void
Y::Message::Member::serialise(std::string& buffer) const
{
  uint32_t ntype = htonl (type_v);
  std::string tmp = "";
  tmp.append ((char *)&ntype, sizeof(ntype));
  switch(type_v)
    {
    case t_string:
      tmp.append(s_v);
      break;
    case t_uint32:
      {
        uint32_t nuint32 = htonl(uint32_v);
        tmp.append((char *)&nuint32, sizeof(nuint32));
        break;
      }
    case t_int32:
      {
        int32_t nint32 = htonl(int32_v);
        tmp.append((char *)&nint32, sizeof(nint32));
        break;
      }
    default:
      abort();
    }  
  
  uint32_t nmember_len = htonl(tmp.length());
  buffer.append ((char *)&nmember_len, sizeof(nmember_len));
  buffer.append (tmp);
}

bool
Y::Message::expectReply() const
{
  switch (op())
    {
      /* One-way messages */
    case YMO_ERROR:
    case YMO_QUIT:
    case YMO_EVENT:
      return false;
      /* Messages that expect a reply */
    case YMO_FIND_CLASS:
      return true;
      /* Messages that might do either, so we need to peek inside them */
    case YMO_INVOKE_CLASS_METHOD:
    case YMO_INVOKE_INSTANCE_METHOD:
      return meta() & 0x01;
      /* There is no default label here, so that gcc will complain if
       * there are any messages missing from this list.
       */
    }
  abort();
}

std::ostream&
operator<< (std::ostream& strm, enum YMessageOperation op)
{
  switch (op)
    {
    case YMO_ERROR:
      strm << "YMO_ERROR";
      break;
    case YMO_QUIT:
      strm << "YMO_QUIT";
      break;
    case YMO_EVENT:
      strm << "YMO_EVENT";
      break;
    case YMO_FIND_CLASS:
      strm << "YMO_FIND_CLASS";
      break;
    case YMO_INVOKE_CLASS_METHOD:
      strm << "YMO_INVOKE_CLASS_METHOD";
      break;
    case YMO_INVOKE_INSTANCE_METHOD:
      strm << "YMO_INVOKE_INSTANCE_METHOD";
      break;
    default:
      strm << (int)op;
      break;
    }
  return strm;
}

std::ostream&
operator<< (std::ostream& strm, const Y::Message m)
{
  strm << "op == " << (enum YMessageOperation)m.op()
       << ", to == " << m.to()
       << ", from == " << m.from()
       << ", id == " << m.id()
       << ", meta == " << m.meta()
       << ", seq == " << m.seq()
       << ", tuple == " << m.tuple()
    ;
  return strm;
}

std::ostream&
operator<< (std::ostream& strm, const Y::Message::Members m)
{
  strm << "<";
  for (Y::Message::Members::const_iterator i = m.begin(); i != m.end(); i++)
    {
      if (i != m.begin())
        strm << ", ";
      strm << *i;
    }
  strm << ">";
  return strm;
}

std::ostream&
operator<< (std::ostream& strm, const Y::Message::Member m)
{
  switch (m.type())
    {
    case Y::Message::Member::t_uint32:
      strm << m.uint32();
      break;
    case Y::Message::Member::t_int32:
      {
        std::ios::fmtflags oldflags = strm.setf(std::ios::showpos);
        strm << m.int32();
        strm.flags(oldflags);
        break;
      }
    case Y::Message::Member::t_string:
      strm << "\"" << m.string() << "\"";
      break;
    }

  return strm;
}

std::string
Y::Message::Members::pretty() const
{
  std::ostringstream strm;
  strm << *this;
  return strm.str();
}

/* arch-tag: 08756a13-dbe3-4308-bcad-761ff255de2c
 */
