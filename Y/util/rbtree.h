/************************************************************************
 *   Copyright (C) Andrew Suffield <asuffield@debian.org>
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

#ifndef Y_UTIL_RBTREE_H
#define Y_UTIL_RBTREE_H

#include <Y/util/yutil.h>

#include <inttypes.h>
#include <stdbool.h>

typedef int rbtree_compare_func(const void *key, const void *obj);
typedef void rbtree_visit_func(void *obj, void *data);

struct rbtree;

struct rbtree_node
{
  struct rbtree *tree;
  struct rbtree_node *left;
  struct rbtree_node *right;
  struct rbtree_node *parent;
  void *obj;
  /* true for red, false for black */
  bool colour;
};

struct rbtree
{
  struct rbtree_node *root;
  rbtree_compare_func *key_compare;
  rbtree_compare_func *obj_compare;
  struct rbtree_node nil;
  uint32_t size;
};

extern struct rbtree *new_rbtree(rbtree_compare_func *key_compare, rbtree_compare_func *obj_compare);
extern struct rbtree *rbtree_dup(const struct rbtree *tree);
extern void free_rbtree(struct rbtree *tree);

/* This is far from the most efficient way to walk a tree, but it is
 * the *safest* way to destroy a tree - the destructor can do almost
 * anything (as long as it does not create an infinite loop) to the
 * tree structure without risk.
 *
 * If for some strange reason you need a faster destructor (think
 * twice - speed and memory deallocation don't mix well) then consider
 * stashing an llist of objects and destroying that instead, and just
 * using free_rbtree() on your tree.
 */
#define rbtree_destroy(T, DESTRUCTOR)                   \
  do {                                                  \
    if (T)                                              \
      {                                                 \
        struct rbtree_node *_Y__rbtree_temp;            \
        while ((_Y__rbtree_temp = rbtree_head(T)))      \
          {                                             \
            (DESTRUCTOR)(_Y__rbtree_temp->obj);         \
            if (_Y__rbtree_temp == rbtree_head(T))      \
              rbtree_node_delete(_Y__rbtree_temp);      \
          }                                             \
      }                                                 \
    yfree(T);                                           \
  } while (0)

extern struct rbtree_node *rbtree_insert(struct rbtree *tree, void *obj);
extern struct rbtree_node *rbtree_find(struct rbtree *tree, const void *key);

extern struct rbtree_node *rbtree_head(struct rbtree *tree);
extern struct rbtree_node *rbtree_tail(struct rbtree *tree);

#define rbtree_size(T) ((T)->size)

extern void rbtree_walk(struct rbtree *tree, void *data, rbtree_visit_func *visitor);

extern void rbtree_node_delete(struct rbtree_node *node);

extern struct rbtree_node *rbtree_node_next(struct rbtree_node *node);
extern struct rbtree_node *rbtree_node_prev(struct rbtree_node *node);
#define rbtree_node_data(N) ((N) ? ((N)->obj) : NULL)

extern void rbtree_check_sanity(struct rbtree *tree);

#endif
