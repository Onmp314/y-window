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

#include <Y/util/zorder.h>
#include <Y/util/index.h>
#include <Y/util/yutil.h>

#include <stdlib.h>

struct ZOrderNode
{
  void *obj;
  struct ZOrderNode *above;
  struct ZOrderNode *below;
  struct ZOrder *zorder;
};

struct ZOrder
{
  struct Index *index;
  struct ZOrderNode *top;
  struct ZOrderNode *bottom;
  int (*keyFunction)(const void *, const void *);
  int (*comparisonFunction)(const void *, const void *);
};

struct ZOrderIterator
{
  struct ZOrder *zorder;
  struct ZOrderNode *currentNode;
};

static int
zorderKeyFunction (const void *key_v, const void *obj_v)
{
  const struct ZOrderNode *obj = obj_v;
  return obj -> zorder -> keyFunction (key_v, obj -> obj);
}

static int
zorderComparisonFunction (const void *obj1_v, const void *obj2_v)
{
  const struct ZOrderNode *obj1 = obj1_v;
  const struct ZOrderNode *obj2 = obj2_v;
  return obj1 -> zorder -> comparisonFunction (obj1 -> obj, obj2 -> obj);
}

static void
zorderPrint (struct ZOrder *self)
{
  struct ZOrderNode *node = self -> top;

  Y_TRACE ("ZORDER: %10p", self);
  Y_TRACE ("TOP:    %10p", self-> top);

  while (node != NULL)
    {
      Y_TRACE ("NODE:   ( %10p < %10p > %10p ) -> %10p", node -> above, node, node -> below, node -> obj);
      node = node -> below;
    }
  
  Y_TRACE ("BOTTOM: %10p", self-> bottom);
  Y_TRACE ("----------");
}

struct ZOrder *
zorderCreate (int (*keyFunction)(const void *, const void *),
              int (*comparisonFunction)(const void *, const void *))
{
  struct ZOrder *z = ymalloc (sizeof (struct ZOrder));
  z -> index = indexCreate (zorderKeyFunction, zorderComparisonFunction);
  z -> top = NULL;
  z -> bottom = NULL;
  z -> keyFunction = keyFunction;
  z -> comparisonFunction = comparisonFunction;
  return z;
}

void
zorderDestroy (struct ZOrder *self, void (*destructorFunction)(void *))
{
  struct ZOrderNode *current = self -> top;
  while (current)
    {
      struct ZOrderNode *next = current -> below;
      if (destructorFunction != NULL)
        destructorFunction (current -> obj);
      yfree (current);
      current = next;
    }
  indexDestroy (self -> index, NULL);
  yfree (self);
}

void
zorderAddAtTop (struct ZOrder *self, void *obj)
{
  struct ZOrderNode *node = ymalloc (sizeof (struct ZOrderNode));
  node -> zorder = self;
  node -> obj = obj;
  node -> above = NULL;
  node -> below = self -> top;
  if (self -> top)
    self -> top -> above = node;
  self -> top = node;
  if (self -> bottom == NULL)
    self -> bottom = node;
  indexAdd (self -> index, node);
}

static void *
zorderFind (struct ZOrder *self, void *objKey)
{
  struct ZOrderNode *node = indexFind (self -> index, objKey);
  if (node == NULL)
    return NULL;
  return node -> obj;
}

void
zorderRemove (struct ZOrder *self, void *objKey)
{
  struct ZOrderNode *node = indexRemove (self -> index, objKey);
  if (node == NULL)
    return;
  if (node -> above == NULL)
    self -> top = node -> below;
  else
    node -> above -> below = node -> below;
  if (node -> below == NULL)
    self -> bottom = node -> above;
  else
    node -> below -> above = node -> above;
  yfree (node);
}

void
zorderMoveToTop (struct ZOrder *self, void *objKey)
{
  struct ZOrderNode *node = indexFind (self -> index, objKey);
  if (node == NULL || node -> above == NULL)
    return;
  node -> above -> below = node -> below;
  if (node -> below == NULL)
    self -> bottom = node -> above;
  else
    node -> below -> above = node -> above;
  node -> below = self -> top;
  node -> above = NULL;
  if (self -> top != NULL)
    self -> top -> above = node;
  self -> top = node;
}

static void
zorderSwapAdjacentNodes (struct ZOrder *self, struct ZOrderNode *node1,
                         struct ZOrderNode *node2)
{
  if (node1 -> above == NULL)
    self -> top = node2;
  else
    node1 -> above -> below = node2;
  if (node2 -> below == NULL)
    self -> bottom = node1;
  else
   node2 -> below -> above = node1;
  node2 -> above = node1 -> above;
  node1 -> below = node2 -> below;
  node2 -> below = node1;
  node1 -> above = node2;
}

void
zorderMoveUp (struct ZOrder *self, void *objKey)
{
  struct ZOrderNode *node = indexFind (self -> index, objKey);
  if (node == NULL || node -> above == NULL)
    return;
  zorderSwapAdjacentNodes (self, node->above, node);
} 

void
zorderMoveDown (struct ZOrder *self, void *objKey)
{
  struct ZOrderNode *node = indexFind (self -> index, objKey);
  if (node == NULL || node -> below == NULL)
    return;
  zorderSwapAdjacentNodes (self, node, node->below);
}

void
zorderMoveToBottom (struct ZOrder *self, void *objKey)
{
  struct ZOrderNode *node = indexFind (self -> index, objKey);
  if (node == NULL || node -> below == NULL)
    return;
  node -> below -> above = node -> above;
  if (node -> above == NULL)
    self -> top = node -> below;
  else
    node -> above -> below = node -> below;
  node -> above = self -> bottom;
  node -> below = NULL;
  if (self -> bottom != NULL)
    self -> bottom -> below = node;
  self -> bottom = node;
}

void *
zorderGetTop (struct ZOrder *self)
{
  if (self -> top == NULL)
    return NULL;
  return self -> top -> obj;
}

void *
zorderGetBottom (struct ZOrder *self)
{
  if (self -> bottom == NULL)
    return NULL;
  return self -> bottom -> obj;
}

struct ZOrderIterator *zorderGetTopIterator (struct ZOrder *self)
{
  struct ZOrderIterator *iterator = ymalloc (sizeof (struct ZOrderIterator));
  iterator -> zorder = self;
  iterator -> currentNode = self -> top;
  return iterator;
}

struct ZOrderIterator *zorderGetBottomIterator (struct ZOrder *self)
{
  struct ZOrderIterator *iterator = ymalloc (sizeof (struct ZOrderIterator));
  iterator -> zorder = self;
  iterator -> currentNode = self -> bottom;
  return iterator;
}

void
zorderiteratorDestroy (struct ZOrderIterator *self)
{
  yfree (self);
}

void *
zorderiteratorGet (struct ZOrderIterator *self)
{
  if (self -> currentNode)
    return self -> currentNode -> obj;
  else return NULL;
}

int
zorderiteratorHasValue (struct ZOrderIterator *self)
{
  return (self -> currentNode != NULL) ? 1 : 0;
}

void
zorderiteratorMoveUp (struct ZOrderIterator *self)
{
  if (self -> currentNode)
    {
      self -> currentNode = self -> currentNode -> above;
    }
}

void
zorderiteratorMoveDown (struct ZOrderIterator *self)
{
  if (self -> currentNode)
    {
      self -> currentNode = self -> currentNode -> below;
    }
}


/* arch-tag: f705f565-42a9-48e0-99e7-69427cfc7fd9
 */
