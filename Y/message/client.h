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

#ifndef Y_MESSAGE_CLIENT_H
#define Y_MESSAGE_CLIENT_H

struct Client;

#include <Y/y.h>
#include <Y/message/message.h>
#include <Y/object/object.h>

#include <sys/types.h>

void           clientInitialise (void);
void           clientFinalise (void);

void           clientRegister (struct Client *);
void           clientReadData (struct Client *);
void           clientClose (struct Client *);

int            clientGetID (const struct Client *);
struct Client *clientFind (int id);

void           clientAddObject (struct Client *, struct Object *);
void           clientRemoveObject (struct Client *, struct Object *);

struct Client *getCurrentClient(void);
void           setCurrentClient (struct Client *);

void           clientSubscribedSignal (struct Client *, struct Object *, const char *);
void           clientUnsubscribedSignal (struct Client *, struct Object *, const char *);

void           clientSendMessage (struct Client *, struct Message *);

#endif /* header guard */

/* arch-tag: 7a621d95-8378-4d45-8caf-4fd02a1188d7
 */
