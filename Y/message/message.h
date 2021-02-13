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

#ifndef Y_MESSAGE_MESSAGE_H
#define Y_MESSAGE_MESSAGE_H

#include <inttypes.h>
#include <sys/types.h>
#include <stdbool.h>

#include <Y/const.h>
#include <Y/message/tuple.h>

struct Message
{
  uint32_t seq, to, from;
  uint32_t op;
  uint32_t id, meta;
  struct Tuple *tuple;
};

#include <Y/y.h>
#include <Y/message/client.h>

struct Message *messageCreate (enum YMessageOperation op);
void            messageDestroy (struct Message *);

struct Message *messageBuildReply(const struct Client *from, const struct Message *m);

#define messageBuildReplyError(from, m, msg)                            \
  ({struct Message *rm = messageBuildReply(from, m);                    \
    rm->tuple = tupleBuild(tb_string(msg));                             \
    rm->op = YMO_ERROR;                                                 \
    rm;                                                                 \
  })

void messageToString (const struct Message *m, char **str, size_t *len);
bool messageFromString (const char *str, size_t len, struct Message **m);

void   messageDespatch (struct Client *, struct Message *);

#endif /* header guard */

/* arch-tag: c3e4d5c8-5eae-404d-8015-2687b2ee9a32
 */
