/************************************************************************
 *   Copyright (C) Mark Thomas <markbt@efaref.net>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <Y/message/message.h>
#include <Y/message/tuple.h>
#include <Y/message/client.h>
#include <Y/const.h>
#include <Y/object/class.h>
#include <Y/object/object.h>
#include <Y/util/yutil.h>
#include <Y/util/log.h>
#include <string.h>
#include <sys/types.h>
#include <assert.h>
#include <stdbool.h>
#include <netinet/in.h>

#include "parse_support.h"

struct Message *
messageCreate (enum YMessageOperation op)
{
  struct Message *m = ymalloc (sizeof (*m));
  m->op = op;
  m->to = m->from = 0;
  m->id = 0;
  m->seq = 0;
  m->meta = 0;
  m->tuple = NULL;
  return m;
}

void
messageDestroy (struct Message *m)
{
  if (m == NULL)
    return;

  tupleDestroy (m->tuple);
  yfree (m);
}

struct Message *
messageBuildReply(const struct Client *from, const struct Message *m)
{
  struct Message *rm = messageCreate(m->op);
  rm->to = clientGetID (from);
  rm->id = m->id;
  rm->seq = m->seq;
  return rm;
}

void
messageToString (const struct Message *m, char **str, size_t *slen)
{
  if (!m)
    {
      if (str)
        *str = NULL;
      *slen = 0;
      return;
    }

  uint32_t packet_len;

  *slen = 0;
  *slen += sizeof(m->seq);
  *slen += sizeof(m->to);
  *slen += sizeof(m->from);
  *slen += sizeof(m->op);
  *slen += sizeof(m->id);
  *slen += sizeof(m->meta);
  *slen += sizeof(m->tuple->count);
  if (m->tuple)
    for (uint32_t i = 0; i < m->tuple->count; i++)
      {
        size_t value_len;
        valueToString(&m->tuple->list[i], NULL, &value_len);
        *slen += sizeof(uint32_t);
        *slen += value_len;
      }

  if (!str)
    return;

  *str = ymalloc(*slen);

  char *p = *str;

  uint32_t seq = htonl(m->seq);
  ADD_SCALAR(p, seq);
  uint32_t to = htonl(m->to);
  ADD_SCALAR(p, to);
  uint32_t from = htonl(m->from);
  ADD_SCALAR(p, from);
  uint32_t op = htonl(m->op);
  ADD_SCALAR(p, op);
  uint32_t id = htonl(m->id);
  ADD_SCALAR(p, id);
  uint32_t meta = htonl(m->meta);
  ADD_SCALAR(p, meta);
  uint32_t value_count = htonl(m->tuple ? m->tuple->count : 0);
  ADD_SCALAR(p, value_count);
  if (m->tuple)
    for (uint32_t i = 0; i < m->tuple->count; i++)
      {
        char *value_str;
        size_t value_len;
        valueToString(&m->tuple->list[i], &value_str, &value_len);
        uint32_t len = htonl(value_len);
        ADD_SCALAR(p, len);
        ADD_DATA(p, value_str, value_len);
        yfree(value_str);
      }

  assert(p == (*str + *slen));
}

bool
messageFromString (const char *str, size_t slen, struct Message **m)
{
  if (!str || slen == 0)
    {
      if (m)
        *m = NULL;
      return 0;
    }

  struct Message tmp;

  const char *p = str;
  size_t l = slen;

  if (!GET_SCALAR(p, l, tmp.seq))
    {
      Y_TRACE ("Failed to parse message (too short for seq)");
      return false;
    }
  if (!GET_SCALAR(p, l, tmp.to))
    {
      Y_TRACE ("Failed to parse message (too short for to)");
      return false;
    }
  if (!GET_SCALAR(p, l, tmp.from))
    {
      Y_TRACE ("Failed to parse message (too short for from)");
      return false;
    }
  if (!GET_SCALAR(p, l, tmp.op))
    {
      Y_TRACE ("Failed to parse message (too short for op)");
      return false;
    }
  if (!GET_SCALAR(p, l, tmp.id))
    {
      Y_TRACE ("Failed to parse message (too short for id)");
      return false;
    }
  if (!GET_SCALAR(p, l, tmp.meta))
    {
      Y_TRACE ("Failed to parse message (too short for meta)");
      return false;
    }
  uint32_t nvalue_count;
  if (!GET_SCALAR(p, l, nvalue_count))
    {
      Y_TRACE ("Failed to parse message (too short for nvalue_count)");
      return false;
    }

  /* This part is a little less obvious */
  uint32_t value_count = ntohl(nvalue_count);
  size_t value_len[value_count];
  const char *value_data[value_count];
  for (uint32_t i = 0; i < value_count; i++)
    {
      /* Get the length of the value, and record it for when we parse
       * properly later
       */
      uint32_t nmlen;
      if (!GET_SCALAR(p, l, nmlen))
        {
          Y_TRACE ("Failed to parse message (too short for nmlen, i == %lu)", (long unsigned int)i);
          return false;
        }
      value_len[i] = ntohl(nmlen);
      /* Record the pointer */
      value_data[i] = p;
      /* Check that we have that much data available */
      if (!SKIP_DATA(p, l, value_len[i]))
        {
          Y_TRACE ("Failed to parse message (too short for member data, i == %lu)", (long unsigned int)i);
          return false;
        }
      /* And check that the value is parseable */
      if (!valueFromString(value_data[i], value_len[i], NULL))
        {
          Y_TRACE ("Failed to parse message (failed to parse value, i == %lu)", (long unsigned int)i);
          return false;
        }
    }

  if (l != 0)
    {
      Y_TRACE ("Failed to parse message (%lu bytes left over at end)", (long unsigned int)l);
      return false;
    }

  if (!m)
    return true;

  *m = messageCreate(ntohl(tmp.op));
  (*m)->seq = ntohl(tmp.seq);
  (*m)->to = ntohl(tmp.to);
  (*m)->from = ntohl(tmp.from);
  (*m)->id = ntohl(tmp.id);
  (*m)->meta = ntohl(tmp.meta);
  (*m)->tuple = tupleCreate(value_count);

  /* Now, we use those two arrays we built earlier (of pointer/length
   * pairs) to actually parse the tuple
   */
  for (uint32_t i = 0; i < value_count; i++)
    {
      struct Value *value;
      if (!valueFromString(value_data[i], value_len[i], &value))
        /* This is impossible (because it worked earlier), and we've
         * already committed to allocating, so abort
         */
        abort();
      /* And copy it into the message */
      memcpy(&((*m)->tuple->list[i]), value, sizeof(*value));
      yfree(value);
    }

  return true;
}

static void
messageDespatchFindClass (const struct Client *clientFrom, const struct Message *m)
{
  if (m->tuple->count != 1)
    {
      messageDespatch(NULL, messageBuildReplyError(clientFrom, m, "Type mismatch"));
      return;
    }
  else if (m->tuple->list[0].type != t_string)
    {
      messageDespatch(NULL, messageBuildReplyError(clientFrom, m, "Type mismatch"));
      return;
    }

  struct Class *class = classFindByName (m->tuple->list[0].string.data);
  if (class != NULL)
    {
      struct Message *rm = messageBuildReply(clientFrom, m);
      rm->id = classGetID(class);
      messageDespatch(NULL, rm);
    }
  else
    messageDespatch(NULL, messageBuildReplyError(clientFrom, m, "Class not found"));
}

static void
messageDespatchInvokeClassMethod (struct Client *clientFrom, const struct Message *m)
{
  if (m->tuple->count < 1)
    {
      messageDespatch(NULL, messageBuildReplyError(clientFrom, m, "Type mismatch"));
      return;
    }
  else if (m->tuple->list[0].type != t_string)
    {
      messageDespatch(NULL, messageBuildReplyError(clientFrom, m, "Type mismatch"));
      return;
    }

  struct Class *class = classFindByID(m->id);
  if (!class)
    {
      messageDespatch(NULL, messageBuildReplyError(clientFrom, m, "Class not found"));
      return;
    }

  const struct Tuple args =
    {
      .count = m->tuple->count - 1,
      .list = m->tuple->list + 1
    };
  struct Tuple *t = classInvokeClassMethod (class, clientFrom, m->tuple->list[0].string.data, &args);
  /* Only send a response if one was requested (but always send errors) */
  if (m->meta && (!t || !t->error))
    {
      struct Message *rm = messageBuildReply(clientFrom, m);
      rm->tuple = t;
      if (t && t->error)
        rm->op = YMO_ERROR;
      messageDespatch(NULL, rm);
    }
  else
    {
      tupleDestroy(t);
    }
}

static void
messageDespatchInvokeInstanceMethod (struct Client *clientFrom, const struct Message *m)
{
  if (m->tuple->count < 1)
    {
      messageDespatch(NULL, messageBuildReplyError(clientFrom, m, "Type mismatch"));
      return;
    }
  else if (m->tuple->list[0].type != t_string)
    {
      messageDespatch(NULL, messageBuildReplyError(clientFrom, m, "Type mismatch"));
      return;
    }

  struct Object *object = objectFind(m->id);
  if (!object)
    {
      messageDespatch(NULL, messageBuildReplyError(clientFrom, m, "Object not found"));
      return;
    }

  const struct Tuple args =
    {
      .count = m->tuple->count - 1,
      .list = m->tuple->list + 1
    };
  struct Tuple *t = classInvokeInstanceMethod (object, clientFrom, m->tuple->list[0].string.data, &args);
  /* Only send a response if one was requested (but always send errors) */
  if (m->meta && (!t || !t->error))
    {
      struct Message *rm = messageBuildReply(clientFrom, m);
      rm->tuple = t;
      if (t && t->error)
        rm->op = YMO_ERROR;
      messageDespatch(NULL, rm);
    }
  else
    {
      tupleDestroy(t);
    }
}

static void
messageDoDespatch (struct Client *clientFrom, struct Message *m)
{
  /* check the message has a valid source. */
  if (clientFrom == NULL && m->from != 0)
    {
      clientFrom = clientFind (m->from);
      if (clientFrom == NULL)
        {
          Y_WARN ("Unknown message source (%d), ignoring.", m->from);
          return;
        }
    }

  /* is the message addressed to another client? */
  if (m->to != 0)
    {
      struct Client *clientTo = clientFind (m->to);
      if (clientTo == NULL)
        {
          Y_WARN ("Unknown message destination (%d), ignoring.", m->to);
          return;
        }
      clientSendMessage (clientTo, m);
      return;
    }

  /* despatch standard operations */
  switch (m->op)
    {
    case YMO_FIND_CLASS:
      messageDespatchFindClass (clientFrom, m);
      return;
    case YMO_INVOKE_CLASS_METHOD:
      messageDespatchInvokeClassMethod (clientFrom, m);
      return;
    case YMO_INVOKE_INSTANCE_METHOD:
      messageDespatchInvokeInstanceMethod (clientFrom, m);
      return;
    case YMO_QUIT:
      Y_TRACE ("YMO_QUIT from client %d", clientGetID(clientFrom));
      clientClose (clientFrom);
      return;
    case YMO_ERROR:
      Y_TRACE ("YMO_ERROR from client %d", clientGetID(clientFrom));
      clientClose (clientFrom);
      return;
    case YMO_EVENT:
      Y_WARN ("Ignoring event message from client");
      return; 
    default:
      break;
    }

  Y_WARN ("Message not handled.");
}

void
messageDespatch (struct Client *clientFrom, struct Message *m)
{
  struct Client *oldClient = getCurrentClient();
  setCurrentClient(clientFrom);
  messageDoDespatch (clientFrom, m);
  messageDestroy (m);
  setCurrentClient(oldClient);
}

/* arch-tag: 2edc8cfa-2fdd-45ea-851b-db14440d3205
 */
