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

#ifndef Y_MESSAGE_CLIENT_P_H
#define Y_MESSAGE_CLIENT_P_H

#include <Y/y.h>
#include <Y/message/client.h>
#include <Y/util/index.h>
#include <Y/util/dbuffer.h>

#include <sys/types.h>

struct Client
{
  const struct ClientClass *c;
  int id;
  struct Index *objects;
  struct Index *signals;
  struct dbuffer *recvq;
  struct dbuffer *sendq;
};

struct ClientClass
{
  const char *name;
  void (*writeData)   (struct Client *, size_t);
  void (*close)       (struct Client *);
};

/* for putting clients in Indices */
int clientKeyFunction (const void *, const void *);
int clientComparisonFunction (const void *, const void *);

#endif /* header guard */

/* arch-tag: e1e72e42-ceb0-4268-89e1-8552d39a3839
 */
