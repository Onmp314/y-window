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

#include <Y/util/yutil.h>
#include <Y/util/rbtree.h>

#include <assert.h>

struct rbtree *
new_rbtree(rbtree_compare_func *key_compare, rbtree_compare_func *obj_compare)
{
  assert(key_compare && obj_compare);

  struct rbtree *tree = ymalloc(sizeof(*tree));
  tree->nil.tree = tree;
  tree->nil.colour = false;
  tree->nil.obj = NULL;
  tree->nil.left = NULL;
  tree->nil.right = NULL;
  tree->root = &tree->nil;
  tree->key_compare = key_compare;
  tree->obj_compare = obj_compare;
  tree->size = 0;
  return tree;
}

static struct rbtree_node *
rbtree_subtree_dup(const struct rbtree_node *node, struct rbtree *new_tree, struct rbtree_node *new_parent)
{
  struct rbtree_node *new_node = ymalloc(sizeof(*new_node));
  new_node->tree = new_tree;
  new_node->obj = node->obj;
  new_node->colour = node->colour;
  new_node->parent = new_parent;
  if (node->left == &node->tree->nil)
    new_node->left = &new_tree->nil;
  else
    new_node->left = rbtree_subtree_dup(node->left, new_tree, new_node);
  if (node->right == &node->tree->nil)
    new_node->right = &new_tree->nil;
  else
    new_node->right = rbtree_subtree_dup(node->right, new_tree, new_node);
  return new_node;
}

struct rbtree *
rbtree_dup(const struct rbtree *tree)
{
  struct rbtree *new_tree = ymalloc(sizeof(*new_tree));
  new_tree->key_compare = tree->key_compare;
  new_tree->obj_compare = tree->obj_compare;
  new_tree->size = tree->size;
  new_tree->root = rbtree_subtree_dup(tree->root, new_tree, NULL);
  return new_tree;
}

static void
free_rbtree_subtree(struct rbtree_node *node)
{
  assert(node);
  if (node->left != &node->tree->nil)
    free_rbtree_subtree(node->left);
  if (node->right != &node->tree->nil)
    free_rbtree_subtree(node->right);
  yfree(node);
}

void
free_rbtree(struct rbtree *tree)
{
  if (!tree)
    return;
  if (tree->root != &tree->nil)
    free_rbtree_subtree(tree->root);
  yfree(tree);
}

static void
rbtree_subtree_walk(struct rbtree_node *node, void *data, rbtree_visit_func *visitor)
{
  if (node == &node->tree->nil)
    return;

  rbtree_subtree_walk(node->left, data, visitor);
  (*visitor)(node->obj, data);
  rbtree_subtree_walk(node->right, data, visitor);
}

void
rbtree_walk(struct rbtree *tree, void *data, rbtree_visit_func *visitor)
{
  if (!tree)
    return;
  rbtree_subtree_walk(tree->root, data, visitor);
}

static struct rbtree_node *
rbtree_subtree_head(struct rbtree_node *node)
{
  if (!node)
    return NULL;
  if (node == &node->tree->nil)
    return node;
  while (node->left != &node->tree->nil)
    node = node->left;
  return node;
}

static struct rbtree_node *
rbtree_subtree_tail(struct rbtree_node *node)
{
  if (!node)
    return NULL;
  if (node == &node->tree->nil)
    return node;
  while (node->right != &node->tree->nil)
    node = node->right;
  return node;
}

struct rbtree_node *
rbtree_head(struct rbtree *tree)
{
  if (!tree)
    return NULL;
  struct rbtree_node *node = rbtree_subtree_head(tree->root);
  return node != &tree->nil ? node : NULL;
}

struct rbtree_node *
rbtree_tail(struct rbtree *tree)
{
  if (!tree)
    return NULL;
  struct rbtree_node *node = rbtree_subtree_tail(tree->root);
  return node != &tree->nil ? node : NULL;
}

struct rbtree_node *
rbtree_node_next(struct rbtree_node *node)
{
  if (!node)
    return NULL;
  if (node->right != &node->tree->nil)
    {
      struct rbtree_node *next = rbtree_subtree_head(node->right);
      return next != &node->tree->nil ? next : NULL;
    }
  struct rbtree_node *parent = node->parent;
  while (parent != &node->tree->nil && node == parent->right)
    {
      node = parent;
      parent = node->parent;
    }
  return parent != &node->tree->nil ? parent : NULL;
}

struct rbtree_node *
rbtree_node_prev(struct rbtree_node *node)
{
  if (!node)
    return NULL;
  if (node->left != &node->tree->nil)
    {
      struct rbtree_node *prev = rbtree_subtree_tail(node->left);
      return prev != &node->tree->nil ? prev : NULL;
    }
  struct rbtree_node *parent = node->parent;
  while (parent != &node->tree->nil && node == parent->left)
    {
      node = parent;
      parent = node->parent;
    }
  return parent != &node->tree->nil ? parent : NULL;
}

struct rbtree_node *
rbtree_find(struct rbtree *tree, const void *key)
{
  if (!tree)
    return NULL;
  struct rbtree_node *node = tree->root;

  while (node != &node->tree->nil)
    {
      int cmp = (*tree->key_compare)(key, node->obj);
      if (cmp == 0)
        return node;
      if (cmp < 0)
        node = node->left;
      else
        node = node->right;
    }

  return NULL;
}

static void
rbtree_subtree_left_rotate(struct rbtree_node *x)
{
  struct rbtree_node *y = x->right;

  assert(y != &y->tree->nil);

  /* Hook the left subtree of y to x, in the position currently
   * occupied by y
   */
  x->right = y->left;
  if (y->left != &y->tree->nil)
    y->left->parent = x;

  /* Hoist y */
  y->parent = x->parent;
  if (x->parent != &x->tree->nil)
    {
      if (x == x->parent->left)
        x->parent->left = y;
      else
        x->parent->right = y;
    }
  else
    x->tree->root = y;
  x->parent = y;

  /* Hook x to y, in the position previously occupied by the subtree we moved earlier */
  y->left = x;
}

static void
rbtree_subtree_right_rotate(struct rbtree_node *x)
{
  struct rbtree_node *y = x->left;

  assert(y != &y->tree->nil);

  /* Hook the right subtree of y to x, in the position currently
   * occupied by y
   */
  x->left = y->right;
  if (y->right != &y->tree->nil)
    y->right->parent = x;

  /* Hoist y */
  y->parent = x->parent;
  if (x->parent != &x->tree->nil)
    {
      if (x == x->parent->left)
        x->parent->left = y;
      else
        x->parent->right = y;
    }
  else
    x->tree->root = y;
  x->parent = y;

  /* Hook x to y, in the position previously occupied by the subtree we moved earlier */
  y->right = x;
}

struct rbtree_node *
rbtree_insert(struct rbtree *tree, void *obj)
{
  if (!tree)
    return NULL;

  /* First we do a classic binary tree insert */

  struct rbtree_node *parent = &tree->nil;
  struct rbtree_node *node = tree->root;

  int cmp = 0;
  while (node != &tree->nil)
    {
      parent = node;
      cmp = (*tree->obj_compare)(obj, node->obj);
      if (cmp < 0)
        node = node->left;
      else
        node = node->right;
    }

  struct rbtree_node *new_node = ymalloc(sizeof(*new_node));
  new_node->tree = tree;
  new_node->obj = obj;
  new_node->parent = parent;
  new_node->left = &tree->nil;
  new_node->right = &tree->nil;
  tree->size++;

  if (parent != &tree->nil)
    {
      /* Note that cmp still contains the comparison of obj with
       * parent->obj, from the last pass through the loop above
       */
      if (cmp < 0)
        parent->left = new_node;
      else
        parent->right = new_node;
    }
  else
    tree->root = new_node;

  /* And then we do the red-black magic */
  new_node->colour = true;

  /* Understanding this loop requires understanding the theory of
   * red-black trees
   */
  node = new_node;
  while (node != tree->root && node->parent->colour)
    {
      if (node->parent == node->parent->parent->left)
        {
          struct rbtree_node *y = node->parent->parent->right;
          if (y->colour)
            {
              node->parent->colour = false;
              y->colour = false;
              node->parent->parent->colour = true;
              node = node->parent->parent;
            }
          else
            {
              if (node == node->parent->right)
                {
                  node = node->parent;
                  rbtree_subtree_left_rotate(node);
                }
              node->parent->colour = false;
              node->parent->parent->colour = true;
              rbtree_subtree_right_rotate(node->parent->parent);
            }
        }
      else
        {
          struct rbtree_node *y = node->parent->parent->left;
          if (y->colour)
            {
              node->parent->colour = false;
              y->colour = false;
              node->parent->parent->colour = true;
              node = node->parent->parent;
            }
          else
            {
              if (node == node->parent->left)
                {
                  node = node->parent;
                  rbtree_subtree_right_rotate(node);
                }
              node->parent->colour = false;
              node->parent->parent->colour = true;
              rbtree_subtree_left_rotate(node->parent->parent);
            }
        }
    }
  tree->root->colour = false;

  return new_node;
}

void
rbtree_node_delete(struct rbtree_node *node)
{
  if (!node)
    return;

  /* y is either the node or its successor; y has at most one child */
  struct rbtree_node *y;
  if (node->left == &node->tree->nil || node->right == &node->tree->nil)
    y = node;
  else
    y = rbtree_node_next(node);

  /* x is the child of y, or &y->tree->nil if there isn't one */
  struct rbtree_node *x;
  if (y->left != &y->tree->nil)
    x = y->left;
  else
    x = y->right;

  /* Push x into the slot occupied by y, splicing out y in the process */
  x->parent = y->parent;
  if (y->parent != &y->tree->nil)
    {
      if (y == y->parent->left)
        y->parent->left = x;
      else
        y->parent->right = x;
    }
  else
    y->tree->root = x;

  /* If y is not the node we're deleting, splice it in place of that
   * node
   *
   * The traditional code would call for us to simply copy y->obj, but
   * that would invalidate the wrong pointer - there might be external
   * references to this node, and we must preserve its address.
   */
  if (y != node)
    {
      /* Update y */
      y->parent = node->parent;
      y->left = node->left;
      y->right = node->right;

      /* Update the children and the parent */
      if (y->left != &y->tree->nil)
        y->left->parent = y;
      if (y->right != &y->tree->nil)
        y->right->parent = y;
      if (y->parent != &y->tree->nil)
        {
          if (node == y->parent->left)
            y->parent->left = y;
          else
            y->parent->right = y;
        }
      else
        y->tree->root = y;
    }

  /* node has now been spliced out of the tree */
  yfree(node);
  y->tree->size--;

  /* If y is red, we're done. Otherwise we need to make a fixup pass */
  if (y->colour)
    return;

  while (x != y->tree->root && !x->colour)
    {
      if (x == x->parent->left)
        {
          struct rbtree_node *w = x->parent->right;
          if (w->colour)
            {
              w->colour = false;
              x->parent->colour = true;
              rbtree_subtree_left_rotate(x->parent);
              w = x->parent->right;
            }
          if (!w->left->colour && !w->right->colour)
            {
              w->colour = true;
              x = x->parent;
            }
          else
            {
              if (!w->right->colour)
                {
                  w->left->colour = false;
                  w->colour = true;
                  rbtree_subtree_right_rotate(w);
                  w = x->parent->right;
                }
              w->colour = x->parent->colour;
              x->parent->colour = false;
              w->right->colour = false;
              rbtree_subtree_left_rotate(x->parent);
              x = y->tree->root;
            }
        }
      else
        {
          struct rbtree_node *w = x->parent->left;
          if (w->colour)
            {
              w->colour = false;
              x->parent->colour = true;
              rbtree_subtree_right_rotate(x->parent);
              w = x->parent->left;
            }
          if (!w->right->colour && !w->left->colour)
            {
              w->colour = true;
              x = x->parent;
            }
          else
            {
              if (!w->left->colour)
                {
                  w->right->colour = false;
                  w->colour = true;
                  rbtree_subtree_left_rotate(w);
                  w = x->parent->left;
                }
              w->colour = x->parent->colour;
              x->parent->colour = false;
              w->left->colour = false;
              rbtree_subtree_right_rotate(x->parent);
              x = y->tree->root;
            }
        }
    }
  x->colour = false;
}

static int
rbtree_subtree_check_black_height(struct rbtree_node *node)
{
  if (node == &node->tree->nil)
    return 0;
  int left = rbtree_subtree_check_black_height(node->left);
  int right =rbtree_subtree_check_black_height(node->right);
  if (left != right)
    abort();
  return left + !node->colour;
}

void
rbtree_check_sanity(struct rbtree *tree)
{
  if (!tree)
    return;

  if (!tree->key_compare || !tree->obj_compare)
    abort();

  /* Red-Black tree property 2: nil is black */
  if (tree->nil.colour)
    abort();

  if (tree->nil.tree != tree)
    abort();

  if (tree->nil.left != NULL)
    abort();

  if (tree->nil.right != NULL)
    abort();

  uint32_t size = 0;

  /* Iterate the tree */
  struct rbtree_node *prev = NULL, *next = NULL, *tail = rbtree_tail(tree);
  for (struct rbtree_node *node = rbtree_head(tree); node; node = next)
    {
      if (node->tree != tree)
        abort();

      /* We should never see a nil while iterating */
      if (node == &tree->nil)
        abort();

      /* node == tree-root iff node->parent == &tree->nil */
      if (node == tree->root)
        {
          if (node->parent != &tree->nil)
            abort();
        }
      else
        {
          if (node->parent == &tree->nil)
            abort();
        }

      /* Invertability of the iterate functions */
      if (prev != rbtree_node_prev(node))
        abort();

      /* Check the iteration sequence */
      if (prev)
        {
          if (tree->obj_compare(prev->obj, node->obj) > 0)
            abort();

          /* And the other way around, to make sure obj_compare is stable */
          if (tree->obj_compare(node->obj, prev->obj) < 0)
            abort();
        }

      /* The binary tree property */
      if (node->left != &tree->nil)
        {
          if (tree->obj_compare(node->left->obj, node->obj) > 0)
            abort();
          if (tree->obj_compare(node->obj, node->left->obj) < 0)
            abort();
        }
      if (node->right != &tree->nil)
        {
          if (tree->obj_compare(node->obj, node->right->obj) > 0)
            abort();
          if (tree->obj_compare(node->right->obj, node->obj) < 0)
            abort();
        }

      /* Red-black tree property 3: red nodes have black children */
      if (node->colour)
        {
          if (node->left->colour)
            abort();
          if (node->right->colour)
            abort();
        }

      /* next == NULL iff node == tail */
      next = rbtree_node_next(node);
      if (next)
        {
          if (node == tail)
            abort();
        }
      else
        {
          if (node != tail)
            abort();
        }

      prev = node;
      size++;
    }

  if (size != tree->size)
    abort();

  rbtree_subtree_check_black_height(tree->root);

  return;
}
