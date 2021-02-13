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

#ifndef LLIST_H
#define LLIST_H

struct llist;

#include <Y/y.h>
#include <stddef.h>
#include <sys/types.h>
#include <inttypes.h>

/** \file llist.h
 * \brief Linked list containing void * (struct llist)
 */

/** \defgroup llist Linked list containing void * (struct llist)
 * \ingroup datastructure
 * An llist is a doubly-linked list of nodes, where each node contains
 * a single void * data item.
 *
 * The address of each node in an llist does not change until that
 * node is deleted; this means that you can store a pointer to any
 * given node, and delete it later at your discretion, without having
 * to search the list again.
 *
 * @{
 */ 

struct llist;

/* These structs should be considered opaque. They are defined here so
 * the macros work
 */

/** \brief Opaque llist node struct
 *
 * An llist consists of an ordered group of nodes, with a start and an
 * end. Each node is represented by a struct llist_node.
 *
 * \internal
 * When next == NULL, this node is list->tail
 * When prev == NULL, this node is list->head
 */
struct llist_node
{
  /** \internal Pointer to the data item */
  void *data;
  /** \internal Pointer to the next node in the list, or NULL if this is the last node */
  struct llist_node *next;
  /** \internal Pointer to the previous node in the list, or NULL if this is the first node */
  struct llist_node *prev;
  /** \internal Pointer to the list this node is a member of */
  struct llist *list;
};

/** \brief Opaque llist struct
 *
 * An llist is a generic doubly-linked list that stores a group of
 * void pointers, in an arbitrary order.
 */
struct llist
{
  /** \internal Head of the list */
  struct llist_node *head;
  /** \internal Tail of the list */
  struct llist_node *tail;
  /** \internal Count of the nodes in this list */
  uint32_t length;
};

/* Returns an empty llist */
extern struct llist *new_llist(void) __attribute__((malloc));
/* Frees the list, discarding the data */
extern void free_llist(struct llist *);

/** \brief Destroy an llist and all the data it contains
 * \param LIST llist to destroy
 * \param DESTRUCTOR function that is called on each data item
 *
 * Calls the function on each data member, then
 * destroys the node, and finally destroys the list
 *
 * For simple lists, pass yfree as the function
 * This is a specialised foreach
 *
 * Note that the node destructor MUST NOT add to the head of the list,
 * or else the destructor will likely be called multiple times. It may
 * delete from the list or add to the tail, although beware of
 * infinite loops.
 */
#define llist_destroy(LIST, DESTRUCTOR) \
 do \
   { \
   if (LIST) \
     { \
       struct llist_node *_Y__llist_temp; \
       while ((_Y__llist_temp = (LIST)->head)) \
         { \
           (DESTRUCTOR)((LIST)->head->data); \
           if (_Y__llist_temp == (LIST)->head) \
             { \
               (LIST)->head = (LIST)->head->next; \
               yfree(_Y__llist_temp); \
             } \
         } \
     } \
   yfree(LIST); \
   } while (0)

/* Call the function on each data node, with arguments (node_data, data)
 * The final void * is passed to the function as the second argument
 */
extern void llist_foreach(struct llist *, void (*)(void *, void *), void *);

/* Add the data item to the head of the list */
extern void llist_add_head(struct llist *, void *);
/* Add the data item to the tail of the list */
extern void llist_add_tail(struct llist *, void *);
/* Insert the data item after the given node */
extern void llist_insert_after(struct llist_node *, void *);
/* Insert the data item before the given node */
extern void llist_insert_before(struct llist_node *, void *);

/* Delete this node. The data is left untouched. */
extern void llist_delete_node(struct llist_node *);
/* Delete the first node with this data. The data is left untouched */
extern void llist_delete_data(struct llist *, const void *);

/* Copy a linked list (shallow copy) */
extern struct llist *llist_dup(struct llist *);

/* Find the first node with this data */
extern struct llist_node *llist_find_data(struct llist *, const void *);
/* Find the first node for which this function returns true when
 * called with (node_data, data)
 */
extern struct llist_node *llist_find_match(struct llist *, int (*)(const void *, const void *), const void *);

/* Helper for llist_find_match, compares char * with strcmp */
extern int string_match(const void *, const void *);

/** \brief Get the head of an llist
 * \return First node in the list, or NULL if the list is empty
 */
#define llist_head(A) ((A)->head)
/** \brief Get the tail of an llist
 * \return Last node in the list, or NULL if the list is empty
 */
#define llist_tail(A) ((A)->tail)
/** \brief Test if an llist is empty
 * \return True if the list is empty, false otherwise
 */
#define llist_empty(A) ((A)->head == NULL)
/** \brief Get the next node in an llist
 * \return Next node in the list, or NULL if this is the last node
 */
#define llist_node_next(A) ((A)->next)
/** \brief Get the previous node in an llist
 * \return Previous node in the list, or NULL if this is the first node
 */
#define llist_node_prev(A) ((A)->prev)
/** \brief Get the list this node belongs to
 */
#define llist_node_list(A) ((A) ? (A)->list : NULL)
/** \brief Get the data element stored in this node
 */
#define llist_node_data(A) ((A) ? (A)->data : NULL)
/** \brief Get the data element stored in this node as an lvalue
 * \note This must not be called on a null value
 */
#define llist_node_data_lvalue(A) ((A)->data)

/** \brief Get the length of an llist
 * \return Count of the nodes in the list
 */
#define llist_length(A) ((A)->length)

/* True if the llist is internally consistant */
/* NOTE: This does not guarantee the validity of the llist. In particular,
 * it is sane even if it is NULL
 */
extern int llist_check_sanity(struct llist *);

/** @}
 */

#endif

/* arch-tag: a7cae92b-ca0b-415f-81d8-4fa13f0300d9
 */
