/************************************************************************
 *   Copyright (C) Mark Thomas <markbt@efaref.net>
 *
 *   Based heavily on GNU libavl's rbtree implementation, available at
 *   http://www.msu.edu/user/pfaffben/avl/.
 *   Copyright (C) 2002 Ben Pfaff <blp@gnu.org>
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

#include <Y/util/index.h>
#include <Y/util/yutil.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <Y/y.h>

enum IndexColour
{
  INDEX_BLACK,
  INDEX_RED
};

#ifndef INDEX_MAX_HEIGHT
#define INDEX_MAX_HEIGHT 48
#endif

struct IndexNode
{
  struct IndexNode *link[2]; /* this MUST be first, because of ugliness below */
  void *obj;
  unsigned char colour;
};

struct Index
{
  int (*keyFunction)(const void *key, const void *obj);
  int (*comparisonFunction)(const void *obj1, const void *obj2);
  int count, generation;
  struct IndexNode *root;
};

struct IndexIterator
{
  struct Index *index;
  struct IndexNode *pa[INDEX_MAX_HEIGHT];
  unsigned char da[INDEX_MAX_HEIGHT];
  int k;
};

struct Index *
indexCreate (int (*keyFunction)(const void *, const void *),
             int (*comparisonFunction)(const void *, const void *))
{
  struct Index *self = ymalloc (sizeof (struct Index));
  self -> keyFunction = keyFunction;
  self -> comparisonFunction = comparisonFunction;
  self -> root = NULL;
  self -> count = 0;
  self -> generation = 0;
  return self;
}

static void
indexDestroyRecursive (struct Index *self, struct IndexNode *node,
                       void (*destructorFunction)(void *obj))
{
  if (node -> link[0])
    indexDestroyRecursive(self, node->link[0], destructorFunction);
  if (destructorFunction)
    destructorFunction (node -> obj);
  if (node -> link[1])
    indexDestroyRecursive(self, node->link[1], destructorFunction);
  yfree (node);  
}

void
indexDestroy (struct Index *self,
              void (*destructorFunction)(void *obj))
{
  struct IndexNode *root = self->root;
  if (root)
    {
      /* we set the root to NULL here so that destructor callbacks
       * can safely call index functions.
       */
      self->root = NULL;
      indexDestroyRecursive (self, root, destructorFunction);
    }
  yfree (self);
}

void
indexAdd (struct Index *self, void *obj)
{
  struct IndexNode *pa[INDEX_MAX_HEIGHT];
  unsigned char     da[INDEX_MAX_HEIGHT];
  int k;

  struct IndexNode *p;
  struct IndexNode *newNode;

  /* BEGIN UGLINESS
   * here, the first item on the node stack is a pointer
   * to the root pointer of the tree, and, by virtue of sheer luck
   * when cast to a (struct IndexNode *), ->link[0] is also the
   * pointer.  Yuck.  Find a better RB tree implementation to kaiugh
   */ 
  pa[0] = (struct IndexNode *) &(self -> root);
  da[0] = 0;
  k  = 1;

  /* Find the location to insert the object */

  for (p = self -> root; p != NULL; 
       p = p -> link[da[k-1]])
    {
      int compare = self -> comparisonFunction (obj, p->obj);
      if (compare == 0)
        return;
      pa[k] = p;
      da[k] = (compare > 0) ? 1 : 0;
      k++;
      if (k >= INDEX_MAX_HEIGHT)
        abort ();
    }

  newNode = ymalloc (sizeof (struct IndexNode));
  newNode -> obj = obj;
  newNode -> link[0] = NULL;
  newNode -> link[1] = NULL;
  newNode -> colour = INDEX_RED;
  self -> count++;
  self -> generation++;

  pa[k-1] -> link[da[k-1]] = newNode;
  while (k >= 3 && pa[k-1] -> colour == INDEX_RED)
    {
      int left, right;
      struct IndexNode *y;
      if (da[k-2]==0)
        {
          /* left-side rebalance */
          left = 0;
          right = 1;
        }
      else
        {
          /* right-side rebalance; opposite of left-side, so we swap
           * left and right */
          left = 1;
          right = 0;
        }
      /* rebalancing after insertion */
      y = pa[k-2]->link[right];
      if (y != NULL && y->colour == INDEX_RED)
        {
           /* case 1 */
            y -> colour = INDEX_BLACK;
            pa[k-1] -> colour = INDEX_BLACK;
            pa[k-2] -> colour = INDEX_RED;
            k -= 2;
        }
      else
        {
          struct IndexNode *x;
          if (da[k-1]==left)
            y = pa[k-1];
          else
            { 
              /* case 3 */
              x = pa[k-1];
              y = x -> link[right];
              x -> link[right] = y->link[left];
              y -> link[left] = x;
              pa[k-2]->link[left] = y;
            }
          /* case 2 */ 
          x = pa[k-2];
          x -> colour = INDEX_RED;
          y -> colour = INDEX_BLACK;
          x -> link[left] = y -> link[right];
          y -> link[right] = x;
          pa[k-3] -> link[da[k-3]] = y;
          break;
        }
    }
  self -> root -> colour = INDEX_BLACK;
  indexVerifyConstraints (self);
}

void *
indexFind (const struct Index *self, const void *key)
{
  struct IndexNode *p = self -> root;
  while (p != NULL)
    {
      int compare = self -> keyFunction (key, p->obj);
      if (compare == 0)
        return p -> obj;
      p = p -> link[(compare > 0) ? 1 : 0];
    }
  return NULL;
}

void *
indexRemove (struct Index *self, const void *key)
{
  struct IndexNode *pa[INDEX_MAX_HEIGHT];
  unsigned char     da[INDEX_MAX_HEIGHT];
  void *data;
  int compare;

  struct IndexNode *p = (struct IndexNode *) &(self -> root);
  int k  = 0;

  /* Find the location of the object to delete */

  compare = -1;
  while (compare != 0)
    {
      int dir = compare > 0 ? 1 : 0;
      pa[k] = p;
      da[k] = dir;
      p = p -> link[dir];
      k++;
      if (p == NULL)
        return NULL;
      compare = self -> keyFunction (key, p->obj);
    }

  /* delete item from the RB tree */

  if (p -> link[1] == NULL)
    {
      /* case 1 */
      pa[k-1] -> link[da[k-1]] = p -> link [0];
    }
  else
    {
      enum IndexColour temp;
      struct IndexNode *r = p -> link[1];
      if (r -> link[0] == NULL)
        {
          /* case 2 */
          r -> link[0] = p -> link[0];
          temp = r -> colour;
          r -> colour = p -> colour;
          p -> colour = temp;
          pa[k-1] -> link[da[k-1]] = r;
          pa[k] = r;
          da[k] = 1;
          k++;
        }
      else
        {
          /* case 3 */
          struct IndexNode *s;
          int j = k;
          k++;
          for (;;)
            {
              pa[k] = r;
              da[k] = 0;
              k++;
              s = r -> link[0];
              if (s -> link[0] == NULL)
                break;
              r = s;
            }
          da[j] = 1;
          pa[j] = s;
          pa[j-1] -> link[da[j-1]] = s;
          s -> link[0] = p -> link[0];
          r -> link[0] = s -> link[1];
          s -> link[1] = p -> link[1];
          temp = s -> colour;
          s -> colour = p -> colour;
          p -> colour = temp;
        } 
    }

  if (p -> colour == INDEX_BLACK)
    {
      /* rebalance after RB deletion */
      for (;;)
        {
          struct IndexNode *x = pa[k-1] -> link[da[k-1]];
          struct IndexNode *w;
          int left, right;
          if (x != NULL && x -> colour == INDEX_RED)
            {
              x -> colour = INDEX_BLACK;
              break;
            }
          if (k < 2)
            break;
          if (da[k-1] == 0)
            {
              left = 0;
              right = 1;
            }
          else
            {
              left = 1;
              right = 0;
            }
          w = pa[k-1]->link[right];
          if (w -> colour == INDEX_RED)
            {
              /* ensure w is black */
              w -> colour = INDEX_BLACK;
              pa[k-1] -> colour = INDEX_RED;
              pa[k-1] -> link[right] = w->link[left];
              w->link[left] = pa[k-1];
              pa[k-2] -> link[da[k-2]] = w;
              pa[k] = pa[k-1];
              da[k] = left;
              pa[k-1] = w;
              k++;
              w = pa[k-1]->link[right];
            }
          if ((w -> link[left] == NULL ||
                w -> link[left] -> colour == INDEX_BLACK)
              && (w -> link[right] == NULL ||
                w -> link[right] -> colour == INDEX_BLACK))
            {
              /* case 1 (w has no red children) */
              w -> colour = INDEX_RED;
            }
          else
            {
              if (w -> link[right] == NULL ||
                    w -> link[right] -> colour == INDEX_BLACK)
                {
                  /* case 3 (w's left child is red)
                   * (transform into case 2) */
                  struct IndexNode *y = w->link[left];
                  y -> colour = INDEX_BLACK;
                  w -> colour = INDEX_RED;
                  w -> link[left] = y->link[right];
                  y -> link[right] = w;
                  pa[k-1]->link[right] = y;
                  w = y;
                }
              /* case 2 (w's right child is red) */
              w -> colour = pa[k-1]->colour;
              pa[k-1]->colour = INDEX_BLACK;
              w -> link[right]->colour = INDEX_BLACK;
              pa[k-1]->link[right] = w->link[left];
              w -> link[left] = pa[k-1];
              pa[k-2]->link[da[k-2]] = w;
              break;
            }
          k --;
        }
    }
  data = p -> obj;
  yfree (p);
  self -> count--;
  self -> generation++;
  indexVerifyConstraints (self);
  return data;
}

int
indexCount (const struct Index *self)
{
  return self->count;
}

static void
indexIteration (struct Index *self, struct IndexNode *node,
                  void *userData, void (*iterationFunction)(const void *, void *))
{
  if (node -> link[0] != NULL)
    indexIteration (self, node -> link[0], userData, iterationFunction);

  iterationFunction (node->obj, userData);

  if (node -> link[1] != NULL)
    indexIteration (self, node -> link[1], userData, iterationFunction);
}

void
indexIterate (struct Index *self, void *userData, void (*iterationFunction)(const void *, void *))
{
  if (self -> root)
    indexIteration (self, self -> root, userData, iterationFunction);
}

struct IndexIterator *
indexGetStartIterator (struct Index *self)
{
  struct IndexIterator *iter = ymalloc (sizeof (struct IndexIterator));
  struct IndexNode *p;
  iter -> index = self;
  iter -> k = 0;
  p = self -> root;
  while (p != NULL)
    {
      iter -> pa[iter -> k] = p;
      iter -> da[iter -> k] = 0;
      iter -> k++;
      p = p -> link[0];
    }
  return iter; 
}

struct IndexIterator *
indexGetEndIterator (struct Index *self)
{
  struct IndexIterator *iter = ymalloc (sizeof (struct IndexIterator));
  struct IndexNode *p;
  iter -> index = self;
  iter -> k = 0;
  p = self -> root;
  while (p != NULL)
    {
      iter -> pa[iter -> k] = p;
      iter -> da[iter -> k] = 1;
      iter -> k++;
      p = p -> link[1];
    }
  return iter; 
}

void
indexiteratorDestroy (struct IndexIterator *self)
{
  yfree (self);
}

int
indexiteratorHasValue (struct IndexIterator *self)
{
  return (self -> k != 0) ? 1 : 0;
}

void *
indexiteratorGet (struct IndexIterator *self)
{
  if (self -> k == 0)
    return NULL;

  return (self -> pa[self -> k-1] -> obj);
}

static void
indexiteratorMove (struct IndexIterator *self, unsigned char dir)
{
  struct IndexNode *p;
  unsigned char ldir = (dir == 0) ? 1 : 0;
  unsigned char rdir = (dir == 0) ? 0 : 1;
  if (self -> k == 0)
    {
      p = self -> index -> root;
      while (p != NULL)
        {
          self -> pa[self -> k] = p;
          self -> da[self -> k] = ldir;
          self -> k++;
          p = p -> link[ldir];
        }
    }
  else
    {
      p = self -> pa[self -> k-1];
      if (p -> link[rdir] != NULL)
        {
          p = p -> link[rdir];
          self -> pa[self -> k] = p;
          self -> da[self -> k] = rdir;
          self -> k++;
          p = p -> link[ldir];
          while (p != NULL)
            {
              self -> pa[self -> k] = p;
              self -> da[self -> k] = ldir;
              self -> k++;
              p = p -> link[ldir];
            } 
        }
      else
        {
          while (self -> da[self -> k-1] == rdir &&
                 self -> k > 1)
            {
              self -> k--;
            }
          self -> k--;
        }
    } 
}

void
indexiteratorNext (struct IndexIterator *self)
{
  indexiteratorMove (self, 1);
}


void 
indexiteratorPrevious (struct IndexIterator *self)
{
  indexiteratorMove (self, 0);
}

static int
indexVerifyConstraint1 (struct Index *self, struct IndexNode *node)
{
  int satisfy = 1;
  if (node == NULL)
    return satisfy;
  if (node -> colour == INDEX_RED)
    {
      if (node -> link[0] != NULL && node -> link[0] -> colour == INDEX_RED)
        {
          Y_ERROR ("C1 broken at %p (left)",
                  (void *)node);
          satisfy = 0;
        }
      if (node -> link[1] != NULL && node -> link[1] -> colour == INDEX_RED)
        {
          Y_ERROR ("C1 broken at %p (right)",
                  (void *)node);
          satisfy = 0;
        }
    }
  satisfy = satisfy && indexVerifyConstraint1 (self, node -> link[0]);
  satisfy = satisfy && indexVerifyConstraint1 (self, node -> link[1]);
  return satisfy;
}

static int
countBlackNodes (struct IndexNode *n, int *satisfy_p)
{
  int left_nodes, right_nodes;
  if (n == NULL) return 0;
  left_nodes =  countBlackNodes (n->link[0], satisfy_p);
  right_nodes = countBlackNodes (n->link[1], satisfy_p);
  if (left_nodes != right_nodes)
    {
      Y_WARN ("Index: Constraint 2 broken at %p (%d, %d)\n",
                 (void *)n, left_nodes, right_nodes);
      *satisfy_p = 0;
    }
  return left_nodes + (n->colour == INDEX_BLACK ? 1 : 0);
}

static int
indexVerifyConstraint2 (struct Index *self, struct IndexNode *node)
{
  int satisfy = 1;

  countBlackNodes (node, &satisfy);
  return satisfy;
}

int
indexVerifyConstraints (struct Index *self)
{
  return indexVerifyConstraint1 (self, self->root)
         && indexVerifyConstraint2 (self, self->root);
}


/* arch-tag: e46b79b7-cb46-4de1-9668-dac260aa522e
 */
