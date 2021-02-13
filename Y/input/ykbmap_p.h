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

#ifndef Y_INPUT_YKBMAP_P_H
#define Y_INPUT_YKBMAP_P_H

#include <inttypes.h>
#include <Y/util/llist.h>
#include <Y/util/index.h>
#include <stdbool.h>

/* A keymap is effectively a DFA, where an edge is a (keycode, stroke
 * direction, modifier set) tuple and a list of ykbAction. When the
 * DFA transitions along an edge, its action list is executed.
 */

/* Some thoughts on extended input methods:
 *
 * They operate only on string and event actions. All other actions
 * are unaffected. Processing proceeds as usual, but while an extended
 * input method is active, all string and event actions are sent to
 * the extended method instead of being dispatched directly. The
 * extended input method is responsible for dispatching strings and
 * events as appropriate, and for informing YKB when extended input
 * has completed.
 *
 * You probably want to combine ykbaBeginExtended with ykbaSetKeymap
 * in most cases, since at least some key sequences will behave
 * differently while the extended input method is active.
 */

enum ykbActionType
  {
    /* modifiers is a bitmask to | with the modifier state */
    ykbaSetModifiers,
    /* modifiers is a bitmask to & with the modifier state */
    ykbaMaskModifiers,
    /* modifiers is a bitmask to ^ with the modifier state */
    ykbaToggleModifiers,
    /* These are like the previous pair, but they change the sticky modifier state
     *
     * Effective modifier state is (regular | sticky). Otherwise there
     * is no difference.
     *
     * This exists so that, for example:
     *
     * shift-down sets the shift modifier
     * shift-up masks out the shift modifier
     * caps-lock toggles the sticky shift modifier
     *
     * And any sequence of the above three events will do the "right"
     * thing.
     */
    ykbaSetStickyModifiers,
    ykbaMaskStickyModifiers,
    ykbaToggleStickyModifiers,
    /* str is the name of an extended input method to trigger */
    ykbaBeginExtended,
    /* no arguments, abort the current extended input method (if any) */
    ykbaAbortExtended,
    /* map is the map to use in future */
    ykbaSetKeymap,
    /* str is a UTF-8 string */
    ykbaString,
    /* str is an ASCII event name */
    ykbaEvent
  };

struct ykbMap;

struct ykbAction
{
  enum ykbActionType type;
  /* Only some of these fields are valid, depending on the value of type */
  char *str;
  uint16_t modifiers;
  struct ykbMap *map;
};

/* These next few structures represent the DFA itself */

struct ykbMapNode;

/* A mask edge is considered to match x if all of these are true:
 *   x->direction         == edge->direction
 *   x->keycode           == edge->keycode
 *  (x->modifiers & mask) == edge->modifiers
 */
struct ykbMapMaskEdge
{
  uint16_t mask;
  struct ykbMapEdge *edge;
};

struct ykbMapMaskEdgeList
{
  bool direction;
  uint16_t keycode;
  /* List of ykbMapMaskEdge */
  struct llist *mask_edges;
};

struct ykbMapEdge
{
  /* true for down, false for up */
  bool direction;
  uint16_t keycode;
  uint16_t modifiers;
  /* NULL means return to the start node */
  struct ykbMapNode *destination;
  struct llist *actions;

  /* When modifiers == 0, we have a list of masks (these are
   * unsortable). If we can't find an exact match for our current
   * modifier set, we fetch the edge with modifiers == 0 and search
   * this list for an edge which matches our state.
   */
  struct llist *modifier_masks;
};

struct ykbMapNode
{
  /* Index of ykbMapEdge */
  struct Index *edges;
  /* Index of ykbMapMaskEdgeList */
  struct Index *mask_edge_lists;
};

extern struct ykbMapEdge *ykbMapLookup(struct ykbMapNode *node, bool direction, uint16_t keycode, uint16_t modifiers);

struct ykbKeyDescription
{
  uint16_t keycode;
  char *str;
};

struct ykbModifierDescription
{
  uint16_t modifiers;
  uint16_t mask;
  char *str;
};

struct ykbMap
{
  struct ykbMapNode *start;
  struct llist *nodes;

  struct Index *keycode_descriptions;
  struct llist *modifier_descriptions;
};

extern struct ykbKeyDescription *ykbKeyLookup(struct ykbMap *map, uint16_t keycode);

/* A mapset is simply a set of maps, one of which is the initial one;
 * it is a closed unit of allocation, within which all ykbaSetKeymap
 * references must exist
 */

struct ykbMapSet
{
  struct ykbMap *initial;
  struct llist *maps;
};

#endif /* header guard */

/* arch-tag: 2e7f8d17-4c41-448b-8ec1-369f21bac22b
 */
