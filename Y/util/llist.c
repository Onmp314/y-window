/************************************************************************
 *   Copyright (C) Andrew Suffield <asuffield@freenode.net>
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

#include <Y/util/llist.h>
#include <Y/util/yutil.h>
#include <Y/util/log.h>

#include <string.h>

struct llist *
new_llist(void)
{
  struct llist *l = ymalloc(sizeof(struct llist));
  l->head = l->tail = NULL;
  l->length = 0;
  return l;
}

void
free_llist(struct llist *l)
{
  struct llist_node *n;
  if (!l)
    return;

  while ((n = l->head))
    {
      l->head = n->next;
      yfree(n);
    }
  yfree(l);
}

void
llist_foreach(struct llist *l, void (*f)(void *, void *), void *data)
{
  struct llist_node *n, *next;

  for (n = l->head; n; n = next)
    {
      next = n->next;
      (*f)(n->data, data);
    }
}

void
llist_add_head(struct llist *l, void *data)
{
  struct llist_node *n;

  n = ymalloc(sizeof(struct llist_node));
  n->data = data;
  if (l->head)
    l->head->prev = n;
  if (!l->tail)
    l->tail = n;
  n->list = l;
  n->next = l->head;
  n->prev = NULL;
  l->head = n;
  l->length++;
}

void
llist_add_tail(struct llist *l, void *data)
{
  struct llist_node *n;

  n = ymalloc(sizeof(struct llist_node));
  n->data = data;
  n->list = l;
  if (l->tail)
    l->tail->next = n;
  if (!l->head)
    l->head = n;
  n->next = NULL;
  n->prev = l->tail;
  l->tail = n;
  l->length++;
}

void llist_insert_after(struct llist_node *t, void *data)
{
  struct llist_node *n;
  n = ymalloc(sizeof(struct llist_node));
  n->data = data;
  n->list = t->list;
  n->next = t->next;
  n->prev = t;
  t->next = n;
  /* If there is an item after this... */
  if (n->next)
    /* then we come before it */
    n->next->prev = n;
  else
    /* otherwise we are the last item */
    n->list->tail = n;
  t->list->length++;
}

void
llist_insert_before(struct llist_node *t, void *data)
{
  struct llist_node *n;
  n = ymalloc(sizeof(struct llist_node));
  n->data = data;
  n->list = t->list;
  n->next = t;
  n->prev = t->prev;
  t->prev = n;
  if (n->prev)
    n->prev->next = n;
  else
    n->list->head = n;
  t->list->length++;
}

void
llist_delete_node(struct llist_node *n)
{
  if (!n)
    return;
  if (n->prev)
    n->prev->next = n->next;
  else
    n->list->head = n->next;
  if (n->next)
    n->next->prev = n->prev;
  else
    n->list->tail = n->prev;
  n->list->length--;
  yfree(n);
}

struct llist_node *
llist_find_data(struct llist *l, const void *d)
{
  struct llist_node *n;

  for (n = l->head; n; n = n->next)
    {
      if (n->data == d)
	return n;
    }
  return NULL;
}

struct llist_node *
llist_find_match(struct llist *l, int (*match_func)(const void *, const void *), const void *data)
{
  struct llist_node *n;

  for (n = l->head; n; n = n->next)
    {
      if ((*match_func)(n->data, data))
	return n;
    }
  return NULL;
}

void
llist_delete_data(struct llist *l, const void *d)
{
  struct llist_node *n;

  for (n = l->head; n; n = n->next)
    {
      if (n->data == d)
	{
	  if (n->prev)
	    n->prev->next = n->next;
	  else
	    n->list->head = n->next;
	  if (n->next)
	    n->next->prev = n->prev;
	  else
	    n->list->tail = n->prev;
	  yfree(n);
          l->length--;
	  break;
	}
    }
}

struct llist *
llist_dup(struct llist *l)
{
  struct llist *l2;
  struct llist_node *n;

  if (!l)
    return NULL;
  l2 = new_llist();

  for (n = l->head; n; n = n->next)
    llist_add_head(l2, n->data);

  return l2;
}

int
string_match(const void *a, const void *b)
{
  const char *x = a, *y = b;
  return strcmp(x, y) == 0;
}

/* True if target->next does not occur in the list from head to target
 * False if there is not a properly linked sequence from head to target
 */
static int
llist_check_sequence_sanity(struct llist_node *head, struct llist_node *target)
{
  struct llist_node *prev = NULL;
  if (target == target->next)
    return 0;
  while (head != target)
    {
      if (head == target->next)
	return 0;
      if (!head)
	return 0;
      if (head->prev != prev)
	return 0;
      prev = head;
      head = head->next;
    }
  return 1;
}

/* Check that:
 *  if l->head == NULL then l->tail == NULL
 *  there is a doubly-linked list from head to tail
 *  all members have n->list == l 
 */
int
llist_check_sanity(struct llist *l)
{
  struct llist_node *n;
  /* A null buffer is a sane (but invalid) llist */
  if (!l)
    return 1;
  if (l->head && !l->tail)
    {
      Y_WARN("Sanity check failed on llist at %p: head has a value, tail is NULL", (void*)l);
      return 0;
    }
  if (!l->head && l->tail)
    {
      Y_WARN("Sanity check failed on llist at %p: head is NULL, tail has a value", (void*)l);
      return 0;
    }
  if (l->head && l->head->prev)
    {
      Y_WARN("Sanity check failed on llist at %p: head has a predecessor", (void*)l);
      return 0;
    }
  if (l->tail && l->tail->next)
    {
      Y_WARN("Sanity check failed on llist at %p: tail has a sucessor", (void*)l);
      return 0;
    }
  uint32_t length = 0;
  for (n = l->head; n; n = n->next)
    {
      length++;
      if (!llist_check_sequence_sanity(l->head, n))
	{
	  Y_WARN("Sanity check failed on llist at %p: sequence is not sane", (void*)l);
	  return 0;
	}
      if (n->list != l)
	{
	  Y_WARN("Sanity check failed on llist at %p: member node has differing list value %p",
	      (void*)l, (void*) n->list);
	  return 0;
	}
      if (n == l->tail)
        {
          /* We have already verified that n->next == NULL */
          if (length != l->length)
            {
              Y_WARN("Sanity check failed on llist at %p: length was %d, expecting %d",
                                                (void *)l, length, l->length);
              return 0;
            }
          return 1;
        }
    }
  if (l->tail)
    {
      /* If we got here we hit a NULL without reaching tail */
      Y_WARN("Sanity check failed on llist at %p: sequence finished before reaching tail",
	  (void*)l);
      return 0;
    }
  return 1;
}

/* arch-tag: 921a3b09-8fd1-4071-84de-8833c4644f71
 */
