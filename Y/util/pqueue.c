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

#include <Y/util/pqueue.h>
#include <Y/util/yutil.h>

struct PQueueNode
{
  struct PQueueNode *next;
  void *obj;
};

struct PQueue
{
  struct PQueueNode *first, *last;
  int (*comparisonFunction)(const void *obj1, const void *obj2);
  int length;
};

struct PQueue *
pqueueCreate (int (*comparisonFunction)(const void *obj1, const void *obj2))
{
  struct PQueue *self = ymalloc (sizeof (struct PQueue));
  self -> first = NULL;
  self -> last = NULL;
  self -> length = 0;
  self -> comparisonFunction = comparisonFunction;
  return self;
}

void
pqueueDestroy (struct PQueue *self, void (*destructorFunction)(void *obj))
{
  struct PQueueNode *current = self -> first;
  while (current != NULL)
    {
      struct PQueueNode *next = current -> next;
      if (destructorFunction)
        destructorFunction (current -> obj);
      yfree (current);
      current = next;
    }
  yfree (self);
}

void
pqueueInsert (struct PQueue *self, void *obj)
{
  int cmp;
  struct PQueueNode *node, *current;

  node = ymalloc (sizeof (struct PQueueNode));
  node -> obj = obj;

  self -> length++;

  /* is it the only node */
  if (self -> first == NULL)
    {
      self -> first = node;
      self -> last = node;
      node -> next = NULL;
      return;
    }

  /* does it belong at the end? */
  cmp = self -> comparisonFunction (obj, self -> last -> obj);
  if (cmp >= 0)
    {
      self -> last -> next = node;
      self -> last = node;
      node -> next = NULL;
      return;
    }
 
  /* does it belong at the beginning? */ 
  cmp = self -> comparisonFunction (obj, self -> first -> obj);
  if (cmp < 0)
    {
      node -> next = self -> first;
      self -> first = node;
      return;
    }

  /* find which node it belongs after */
  current = self -> first;
  while (current -> next != NULL)
    {
      cmp = self -> comparisonFunction (obj, current -> next -> obj);
      if (cmp < 0)
        break;
      current = current -> next;
    }

  node -> next = current -> next;
  current -> next = node;
  return;

}

void *
pqueueGetNext (struct PQueue *self)
{
  struct PQueueNode *first;
  void *obj;

  first = self -> first;
  if (first == NULL)
    return NULL;

  self -> length --;
  self -> first = first -> next;
  if (self -> first == NULL)
    self -> last = NULL;

  obj = first -> obj;
  yfree (first);
  return obj; 
}

void *
pqueuePeekNext (const struct PQueue *self)
{
  if (self -> first == NULL)
    return NULL;
  return self -> first -> obj;
}

void
pqueueRemove (struct PQueue *self, void *userData, int (*testFunction)(void *obj, void *data))
{
  struct PQueueNode *previous = NULL;
  struct PQueueNode *current = self -> first;
  while (current != NULL)
    {
      struct PQueueNode *next = current -> next;
      if (testFunction (current->obj, userData) != 0)
        {
          if (previous == NULL)
              self -> first = next;
          else
              previous -> next = next;
          yfree (current);
          current = next;
          if (current == NULL)
            self -> last = previous;
        }
      else
        {
          previous = current;
          current = next;
        }
    }
}

int
pqueueLength (const struct PQueue *self)
{
  return self -> length;
}

/* arch-tag: 104663f2-1a13-45f3-b69c-f8024f146af8
 */
