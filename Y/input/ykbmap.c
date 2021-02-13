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

#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include <Y/y.h>
#include <Y/input/ykbmap_p.h>
#include <Y/input/ykbmap.h>
#include <Y/util/yutil.h>
#include <Y/util/dbuffer.h>

static int
ykbMapEdgeComparisonFunction (const void *n1_v, const void *n2_v)
{
  const struct ykbMapEdge *n1 = n1_v;
  const struct ykbMapEdge *n2 = n2_v;
  if (n1->keycode == n2->keycode)
    {
      if (n1->modifiers == n2->modifiers)
        {
          if (n1->direction == n2->direction)
            return 0;
          else if (n1->direction < n2->direction)
            return -1;
          else
            return 1;
        }
      else if (n1->modifiers < n2->modifiers)
        return -1;
      else
        return 1;
    }
  else if (n1->keycode < n2->keycode)
    return -1;
  else
    return 1; 
}

static int
ykbMapMaskEdgeListComparisonFunction (const void *n1_v, const void *n2_v)
{
  const struct ykbMapMaskEdgeList *n1 = n1_v;
  const struct ykbMapMaskEdgeList *n2 = n2_v;
  if (n1->keycode == n2->keycode)
    {
      if (n1->direction == n2->direction)
        return 0;
      else if (n1->direction < n2->direction)
        return -1;
      else
        return 1;
    }
  else if (n1->keycode < n2->keycode)
    return -1;
  else
    return 1; 
}

static int
ykbKeyDescriptionComparisonFunction (const void *n1_v, const void *n2_v)
{
  const struct ykbKeyDescription *n1 = n1_v;
  const struct ykbKeyDescription *n2 = n2_v;
  if (n1->keycode == n2->keycode)
    return 0;
  else if (n1->keycode < n2->keycode)
    return -1;
  else
    return 1; 
}

static struct ykbAction *
ykbActionCreate(void)
{
  struct ykbAction *action = ymalloc(sizeof(*action));
  action->str = NULL;
  action->modifiers = 0;
  action->map = NULL;
  return action;
}

static void
ykbActionDestroy(struct ykbAction *action)
{
  if (!action)
    return;
  yfree(action->str);
  yfree(action);
}

static void
ykbActionDestroy_v(void *action)
{
  ykbActionDestroy(action);
}

static struct ykbMapNode *
ykbMapNodeAdd(struct ykbMap *map)
{
  struct ykbMapNode *node = ymalloc(sizeof(*node));
  node->edges = indexCreate(&ykbMapEdgeComparisonFunction, &ykbMapEdgeComparisonFunction);
  node->mask_edge_lists = indexCreate(&ykbMapMaskEdgeListComparisonFunction, &ykbMapMaskEdgeListComparisonFunction);
  llist_add_tail (map->nodes, node);
  return node;
}

static struct ykbMapEdge *
ykbMapEdgeAdd(struct ykbMapNode *from, struct ykbMapNode *to, bool direction, uint16_t keycode, uint16_t modifiers)
{
  struct ykbMapEdge *edge = ymalloc(sizeof(*edge));
  edge->direction = direction;
  edge->keycode = keycode;
  edge->modifiers = modifiers;
  edge->destination = to;
  edge->actions = new_llist ();
  indexAdd(from->edges, edge);
  return edge;
}

static struct ykbMapMaskEdgeList *
ykbMapMaskEdgeListAdd(struct ykbMapNode *from, bool direction, uint16_t keycode)
{
  struct ykbMapMaskEdgeList *list = ymalloc(sizeof(*list));
  list->direction = direction;
  list->keycode = keycode;
  list->mask_edges = new_llist ();
  indexAdd(from->mask_edge_lists, list);
  return list;
}

static struct ykbMapMaskEdge *
ykbMapMaskEdgeAdd(struct ykbMapNode *from, struct ykbMapNode *to, bool direction, uint16_t keycode, uint16_t mask, uint16_t modifiers)
{
  struct ykbMapMaskEdge *edge = ymalloc(sizeof(*edge));
  edge->mask = mask;
  edge->edge = ymalloc(sizeof(*edge->edge));
  edge->edge->direction = direction;
  edge->edge->keycode = keycode;
  edge->edge->modifiers = modifiers;
  edge->edge->destination = to;
  edge->edge->actions = new_llist ();
  struct ykbMapMaskEdgeList key = {.direction = direction, .keycode = keycode, .mask_edges = NULL};
  struct ykbMapMaskEdgeList *list = indexFind(from->mask_edge_lists, &key);
  if (!list)
    list = ykbMapMaskEdgeListAdd(from, direction, keycode);
  llist_add_tail (list->mask_edges, edge);
  return edge;
}

/* Note that we don't destroy the destination node; there is diamond
 * structure here, so we store all the nodes in a list in the map and
 * destroy those at the top level
 */
static void
ykbMapEdgeDestroy(struct ykbMapEdge *edge)
{
  if (!edge)
    return;
  llist_destroy (edge->actions, ykbActionDestroy);
  yfree(edge);
}

static void
ykbMapEdgeDestroy_v(void *edge)
{
  ykbMapEdgeDestroy(edge);
}

static void
ykbMapMaskEdgeDestroy(struct ykbMapMaskEdge *edge)
{
  if (!edge)
    return;
  ykbMapEdgeDestroy(edge->edge);
  yfree(edge);
}

static void
ykbMapMaskEdgeDestroy_v(void *edge)
{
  ykbMapMaskEdgeDestroy(edge);
}

static void
ykbMapMaskEdgeListDestroy(struct ykbMapMaskEdgeList *list)
{
  if (!list)
    return;
  llist_destroy (list->mask_edges, ykbMapMaskEdgeDestroy);
  yfree(list);
}

static void
ykbMapMaskEdgeListDestroy_v(void *list)
{
  ykbMapMaskEdgeListDestroy(list);
}

static void
ykbMapNodeDestroy(struct ykbMapNode *node)
{
  if (!node)
    return;
  indexDestroy(node->edges, ykbMapEdgeDestroy_v);
  indexDestroy(node->mask_edge_lists, ykbMapMaskEdgeListDestroy_v);
  yfree(node);
}

static void
ykbMapNodeDestroy_v(void *node)
{
  ykbMapNodeDestroy(node);
}

static struct ykbMapEdge *
ykbMapFindEdge(struct ykbMapNode *node, bool direction, uint16_t keycode, uint16_t modifiers)
{
  struct ykbMapEdge key = {.direction = direction, .keycode = keycode, .modifiers = modifiers};
  struct ykbMapEdge *edge = indexFind(node->edges, &key);
  return edge;
}

/* This function finds an *exact* match, not a mask match */
static struct ykbMapEdge *
ykbMapFindMaskEdge(struct ykbMapNode *node, bool direction, uint16_t keycode, uint16_t mask, uint16_t modifiers)
{
  struct ykbMapMaskEdgeList mask_key = {.direction = direction, .keycode = keycode};
  struct ykbMapMaskEdgeList *list = indexFind(node->mask_edge_lists, &mask_key);
  if (!list)
    return NULL;

  struct ListIterator *i;
  for (struct llist_node *n = llist_head (list->mask_edges);
       n != NULL;
       n = llist_node_next (n))
    {
      struct ykbMapMaskEdge *mask_edge = llist_node_data (n);
      if (mask == mask_edge->mask && modifiers == mask_edge->edge->modifiers)
        return mask_edge->edge;
    }

  return NULL;
}

/* This function finds either an exact match, or a mask match */
struct ykbMapEdge *
ykbMapLookup(struct ykbMapNode *node, bool direction, uint16_t keycode, uint16_t modifiers)
{
  /* First we try for an exact match */
  struct ykbMapEdge *edge = ykbMapFindEdge(node, direction, keycode, modifiers);
  if (edge)
    return edge;

  /* Then we try for a mask match */
  struct ykbMapMaskEdgeList mask_key = {.direction = direction, .keycode = keycode};
  struct ykbMapMaskEdgeList *list = indexFind(node->mask_edge_lists, &mask_key);
  if (!list)
    return NULL;

  /* We have a list; try them each in turn */
  for (struct llist_node *n = llist_head (list->mask_edges); 
       n != NULL;
       n = llist_node_next (n))
    {
      struct ykbMapMaskEdge *mask_edge = llist_node_data (n);
      edge = mask_edge->edge;
      /* This is the definition of a mask match */
      if ((modifiers & mask_edge->mask) == edge->modifiers)
        {
          assert(direction == edge->direction);
          assert(keycode == edge->keycode);
          return edge;
        }
    }

  return NULL;
}

static struct ykbKeyDescription *
ykbKeyDescriptionCreate(uint16_t keycode, char *str)
{
  struct ykbKeyDescription *desc = ymalloc(sizeof(*desc));
  desc->keycode = keycode;
  desc->str = ystrdup(str);
  return desc;
}

static void
ykbKeyDescriptionDestroy(struct ykbKeyDescription *desc)
{
  if (!desc)
    return;
  yfree(desc->str);
  yfree(desc);
}

static void
ykbKeyDescriptionDestroy_v(void *desc)
{
  ykbKeyDescriptionDestroy(desc);
}

static struct ykbModifierDescription *
ykbModifierDescriptionCreate(uint16_t modifiers, uint16_t mask, char *str)
{
  struct ykbModifierDescription *desc = ymalloc(sizeof(*desc));
  desc->modifiers = modifiers;
  desc->mask = mask;
  desc->str = ystrdup(str);
  return desc;
}

static void
ykbModifierDescriptionDestroy(struct ykbModifierDescription *desc)
{
  if (!desc)
    return;
  yfree(desc->str);
  yfree(desc);
}

static struct ykbMap *
ykbMapAdd(struct ykbMapSet *mapset)
{
  struct ykbMap *map = ymalloc(sizeof(*map));
  map->start = NULL;
  map->nodes = new_llist ();
  map->keycode_descriptions = indexCreate(ykbKeyDescriptionComparisonFunction, ykbKeyDescriptionComparisonFunction);
  map->modifier_descriptions = new_llist();
  llist_add_tail (mapset->maps, map);
  return map;
}

static void
ykbMapDestroy(struct ykbMap *map)
{
  if (!map)
    return;
  llist_destroy (map->nodes, ykbMapNodeDestroy);
  llist_destroy (map->modifier_descriptions, ykbModifierDescriptionDestroy);
  indexDestroy (map->keycode_descriptions, ykbKeyDescriptionDestroy_v);
  yfree(map);
}

static void
ykbMapDestroy_v(void *map)
{
  ykbMapDestroy(map);
}

struct ykbKeyDescription *
ykbKeyLookup(struct ykbMap *map, uint16_t keycode)
{
  struct ykbKeyDescription key = {.keycode = keycode};
  struct ykbKeyDescription *desc = indexFind(map->keycode_descriptions, &key);
  return desc;
}

static struct ykbMapSet *
ykbMapSetCreate(void)
{
  struct ykbMapSet *mapset = ymalloc(sizeof(*mapset));
  mapset->maps = new_llist ();
  mapset->initial = NULL;
  return mapset;
}

void
ykbMapSetDestroy(struct ykbMapSet *mapset)
{
  if (!mapset)
    return;
  llist_destroy (mapset->maps, ykbMapDestroy);
  yfree(mapset);
}

struct keyName
{
  uint16_t value;
  char *name;
};

static struct keyName *
keyNameCreate(uint16_t value, char *name)
{
  struct keyName *n = ymalloc(sizeof(*n));
  n->value = value;
  n->name = ystrdup(name);
  return n;
}

static void
keyNameDestroy(struct keyName *n)
{
  if (!n)
    return;
  yfree(n->name);
  yfree(n);
}

static void
keyNameDestroy_v(void *n)
{
  keyNameDestroy(n);
}

static int
keyNameComparisonFunction(const void *n1_v, const void *n2_v)
{
  const struct keyName *n1 = n1_v;
  const struct keyName *n2 = n2_v;
  return strcmp(n1->name, n2->name);
}

static void
parseName(const char *filename, int lineNumber, char *line, struct Index *names)
{
  /* Skip leading whitespace */
  char *c = line;
  while (isspace(*c))
    c++;

  char *name = c;

  char *nameend;
  for (nameend = c; *nameend; nameend++)
    if (isspace(*nameend))
      break;

  if (*nameend)
    c = nameend + 1;
  else
    c = nameend;

  /* Either way, terminate it */
  *nameend = '\0';

  while (isspace(*c))
    c++;

  if (name[0] == '\0')
    {
      Y_WARN ("%s:%d: Parse error (was expecting a key name)", filename, lineNumber);
      return;
    }

  char *keycode_s = c;

  if (keycode_s[0] == '\0')
    {
      Y_WARN ("%s:%d: Parse error (was expecting a keycode)", filename, lineNumber);
      return;
    }

  char *end;
  uint16_t keycode = strtoul(keycode_s, &end, 0);

  if (*end != '\0')
    {
      Y_WARN ("%s:%d: Parse error (was expecting a numeric keycode)", filename, lineNumber);
      return;
    }

  indexAdd(names, keyNameCreate(keycode, name));
}

static void
parseModifierName(const char *filename, int lineNumber, char *line, struct Index *names)
{
  /* Skip leading whitespace */
  char *c = line;
  while (isspace(*c))
    c++;

  char *name = c;

  char *nameend;
  for (nameend = c; *nameend; nameend++)
    if (isspace(*nameend))
      break;

  if (*nameend)
    c = nameend + 1;
  else
    c = nameend;

  /* Either way, terminate it */
  *nameend = '\0';

  while (isspace(*c))
    c++;

  if (name[0] == '\0')
    {
      Y_WARN ("%s:%d: Parse error (was expecting a key name)", filename, lineNumber);
      return;
    }

  char *modifiers_s = c;

  if (modifiers_s[0] == '\0')
    {
      Y_WARN ("%s:%d: Parse error (was expecting a modifier value)", filename, lineNumber);
      return;
    }

  char *end;
  uint16_t modifiers = strtoul(modifiers_s, &end, 0);

  if (*end != '\0')
    {
      Y_WARN ("%s:%d: Parse error (was expecting a numeric modifier value)", filename, lineNumber);
      return;
    }

  indexAdd(names, keyNameCreate(modifiers, name));
}

static bool
parseModifierValue(char *str, struct Index *modifier_names, uint16_t *value)
{
  char *end;
  uint16_t v = strtoul(str, &end, 0);
  if (*end != '\0')
    {
      /* If it's not just a number, try looking it up as a name */
      bool invert = false;
      if (str[0] == '!')
        {
          invert = true;
          str++;
        }

      struct keyName key = {.value = 0, .name = str};
      struct keyName *name = indexFind(modifier_names, &key);
      if (!name)
        return false;
      v = invert ? ~name->value : name->value;
    }

  *value = v;
  return true;
}

static bool
parseModifiers(char *str, struct Index *modifier_names, uint16_t *mask, uint16_t *modifiers)
{
  if (str[0] == '*' && str[1] == '\0')
    {
      /* This matches anything */
      *mask = 0;
      *modifiers = 0;
      return true;
    }

  char *c = str;
  char *slash = strchr(c, '/');
  if (slash)
    {
      *slash = '\0';
      if (!parseModifierValue(c, modifier_names, mask))
        return false;
      c = slash + 1;
    }

  if (!parseModifierValue(c, modifier_names, modifiers))
    return false;

  return true;
}

static bool
parseKeycode(char *str, struct Index *keycode_names, uint16_t *keycode)
{
  char *end;
  uint16_t v = strtoul(str, &end, 0);

  if (*end != '\0')
    {
      /* Otherwise it's a key name */
      struct keyName key = {.value = 0, .name = str};
      struct keyName *name = indexFind(keycode_names, &key);
      if (!name)
        return false;
      v = name->value;
    }

  *keycode = v;
  return true;
}

static void
parseDescribe(const char *filename, int lineNumber, char *line, struct ykbMap *map, struct Index *keycode_names, struct Index *modifier_names)
{
  /* Skip leading whitespace */
  char *c = line;
  while (isspace(*c))
    c++;

  char *type = c;

  if (type[0] == '\0')
    {
      Y_WARN ("%s:%d: Parse error (was expecting 'keycode' or 'modifier')", filename, lineNumber);
      return;
    }

  char *typeend;
  for (typeend = c; *typeend; typeend++)
    if (isspace(*typeend))
      break;

  if (*typeend)
    c = typeend + 1;
  else
    c = typeend;

  /* Either way, terminate it */
  *typeend = '\0';

  bool modifier_type;
  if (strcasecmp(type, "keycode") == 0)
    {
      modifier_type = false;
    }
  else if (strcasecmp(type, "modifier") == 0)
    {
      modifier_type = true;
    }
  else
    {
      Y_WARN ("%s:%d: Type should be 'keycode' or 'modifier', but got '%s'", filename, lineNumber, type);
      return;
    }

  while (isspace(*c))
    c++;

  char *name = c;

  if (name[0] == '\0')
    {
      Y_WARN ("%s:%d: Parse error (was expecting a %s name)", filename, lineNumber, type);
      return;
    }

  char *nameend;
  for (nameend = c; *nameend; nameend++)
    if (isspace(*nameend))
      break;

  if (*nameend)
    c = nameend + 1;
  else
    c = nameend;

  /* Either way, terminate it */
  *nameend = '\0';

  while (isspace(*c))
    c++;

  char *desc = c;
  char *descend;
  for (descend = c; *descend; descend++)
    if (isspace(*descend))
      break;

  if (*descend)
    c = descend + 1;
  else
    c = descend;

  /* Either way, terminate it */
  *descend = '\0';

  while (isspace(*c))
    c++;

  if (modifier_type)
    {
      uint16_t modifiers;
      uint16_t mask;
      if (!parseModifiers(name, modifier_names, &modifiers, &mask))
        {
          Y_WARN ("%s:%d: Parse error (was expecting a modifier)", filename, lineNumber);
          return;
        }

      llist_add_tail(map->modifier_descriptions, ykbModifierDescriptionCreate(modifiers, mask, desc));
    }
  else
    {
      uint16_t keycode;
      if (!parseKeycode(name, keycode_names, &keycode))
        {
          Y_WARN ("%s:%d: Parse error (was expecting a keycode)", filename, lineNumber);
          return;
        }

      indexAdd(map->keycode_descriptions, ykbKeyDescriptionCreate(keycode, desc));
    }
}

static struct ykbAction *
parseModifierAction(const char *filename, int lineNumber, enum ykbActionType type, char *arg, struct Index *modifier_names)
{
  uint16_t mask = 0;
  mask = ~mask;
  uint16_t modifiers;
  if (!parseModifiers(arg, modifier_names, &mask, &modifiers))
    {
      Y_WARN ("%s:%d: Parse error (was expecting a modifier value)", filename, lineNumber);
      return NULL;
    }
  if ((uint16_t)~mask != 0)
    {
      Y_WARN ("%s:%d: Modifier masks are not valid in actions", filename, lineNumber);
      return NULL;
    }
  struct ykbAction *action = ykbActionCreate();
  action->type = type;
  action->modifiers = modifiers;
  return action;
}

static struct ykbAction *
parseAction(const char *filename, int lineNumber, const char *name, char *arg, struct Index *modifier_names)
{
  if (strcasecmp(name, "setModifiers") == 0)
    return parseModifierAction(filename, lineNumber, ykbaSetModifiers, arg, modifier_names);
  if (strcasecmp(name, "maskModifiers") == 0)
    return parseModifierAction(filename, lineNumber, ykbaMaskModifiers, arg, modifier_names);
  if (strcasecmp(name, "toggleModifiers") == 0)
    return parseModifierAction(filename, lineNumber, ykbaToggleModifiers, arg, modifier_names);
  if (strcasecmp(name, "setStickyModifiers") == 0)
    return parseModifierAction(filename, lineNumber, ykbaSetStickyModifiers, arg, modifier_names);
  if (strcasecmp(name, "maskStickyModifiers") == 0)
    return parseModifierAction(filename, lineNumber, ykbaMaskStickyModifiers, arg, modifier_names);
  if (strcasecmp(name, "toggleStickyModifiers") == 0)
    return parseModifierAction(filename, lineNumber, ykbaToggleStickyModifiers, arg, modifier_names);

  if (strcasecmp(name, "beginExtended") == 0)
    {
      struct ykbAction *action = ykbActionCreate();
      action->type = ykbaBeginExtended;
      action->str = ystrdup(arg);
      return action;
    }

  if (strcasecmp(name, "abortExtended") == 0)
    {
      struct ykbAction *action = ykbActionCreate();
      action->type = ykbaBeginExtended;
      return action;
    }

  if (strcasecmp(name, "setKeymap") == 0)
    {
      /* NYI */
      return NULL;
    }

  if (strcasecmp(name, "string") == 0)
    {
      struct ykbAction *action = ykbActionCreate();
      action->type = ykbaString;
      action->str = ystrdup(arg);
      return action;
    }

  if (strcasecmp(name, "event") == 0)
    {
      struct ykbAction *action = ykbActionCreate();
      action->type = ykbaEvent;
      action->str = ystrdup(arg);
      return action;
    }

  Y_WARN ("%s:%d: Unrecognised action '%s'", filename, lineNumber, name);
  return NULL;
}

/* Syntax example:
 *
 * seq 1 x 3[4] string(foo)
 *
 * keycode 1, keycode named by x, keycode 3 with modifier set 4, emit
 * string 'foo'
 */
static void
parseSeq(const char *filename, int lineNumber, char *line, struct ykbMap *map, struct ykbMapNode *parent,
         struct Index *keycode_names, struct Index *modifier_names)
{
  char *c = line;

  struct ykbMapNode *node = parent;
  struct ykbMapEdge *last = NULL;

  /* Parse one word at a time */
  while (*c)
    {
      while (isspace(*c))
        c++;

      char *word = c;

      char *wordend;
      for (wordend = c; *wordend; wordend++)
        if (isspace(*wordend))
          break;

      /* If it contains a ( before the end of the word, it's an
       * action; note that actions may contain spaces in the
       * parenthesised region
       */
      char *parens = strchr(word, '(');
      if (parens && parens < wordend)
        {
          *parens = '\0';

          /* Now we have a miniature escaped-string parser for the
           * thing in the parentheses. This allows:
           *
           * string(x) -> 'x'
           * string(\)) -> ')'
           * string(\\) -> '\'
           */
          char *p = parens + 1;
          size_t len = strlen(p);
          char arg[len + 1];
          char *argp = arg;
          parens = NULL;
          for (; *p; p++)
            {
              if (*p == '\\')
                {
                  p++;
                  *argp++ = *p;
                }
              else if (*p == ')')
                {
                  *argp = '\0';
                  parens = p;
                  break;
                }
              else
                *argp++ = *p;
            }

          if (!parens)
            {
              Y_WARN ("%s:%d: Parse error (was expecting a close parenthesis)", filename, lineNumber);
              return;
            }

          /* And the next word begins after the close parenthesis */
          c = parens + 1;

          if (!last)
            {
              Y_WARN ("%s:%d: Actions not allowed before any keycodes", filename, lineNumber);
              return;
            }

          struct ykbAction *action = parseAction(filename, lineNumber, word, arg, modifier_names);
          if (action)
            llist_add_tail (last->actions, action);

          continue;
        }

      /* Now tie off the word */
      if (*wordend)
        c = wordend + 1;
      else
        c = wordend;

      /* Either way, terminate it */
      *wordend = '\0';

      /* Default is down */
      bool direction = true;

      /* If it starts with a ^, it's an up-stroke instead */
      if (word[0] == '^')
        {
          direction = false;
          word++;
        }

      /* If it contains a [, it's a keycode with modifiers */
      uint16_t mask = 0;
      mask = ~mask;
      uint16_t modifiers = 0;
      char *bracket = strchr(word, '[');
      if (bracket)
        {
          *bracket = '\0';
          char *arg = bracket + 1;
          bracket = strchr(arg, ']');
          if (!bracket)
            {
              Y_WARN ("%s:%d: Parse error (was expecting a close bracket)", filename, lineNumber);
              return;
            }
          *bracket = '\0';

          if (!parseModifiers(arg, modifier_names, &mask, &modifiers))
            {
              Y_WARN ("%s:%d: Parse error (was expecting a modifier mask)", filename, lineNumber);
              return;
            }
        }

      /* If it parses as a number, it's a raw keycode */
      uint16_t keycode;
      if (!parseKeycode(word, keycode_names, &keycode))
        {
          Y_WARN ("%s:%d: Unrecognised key name '%s'", filename, lineNumber, word);
          return;
        }

      if (last)
        {
          if (last->destination)
            node = last->destination;
          else
            {
              struct ykbMapNode *next = ykbMapNodeAdd(map);
              last->destination = next;
              node = next;
            }
        }

      if ((uint16_t)~mask == 0)
        {
          /* mask is all 1s, so no mask */
          struct ykbMapEdge *edge = ykbMapFindEdge(node, direction, keycode, modifiers);
          if (edge)
            last = edge;
          else
            last = ykbMapEdgeAdd(node, NULL, direction, keycode, modifiers);
        }
      else
        {
          /* We have a mask for this one */
          struct ykbMapEdge *edge = ykbMapFindMaskEdge(node, direction, keycode, mask, modifiers);
          if (edge)
            last = edge;
          else
            last = ykbMapMaskEdgeAdd(node, NULL, direction, keycode, mask, modifiers)->edge;
        }
    }
}

struct ykbMapSet *
ykbLoadMapSet(const char *filename)
{
  int fd = open(filename, O_RDONLY | O_NOCTTY);

  if (fd < 0)
    {
      Y_ERROR ("Error loading keymap %s: %s", filename, strerror (errno));
      return NULL;
    }

  FILE *f = fdopen(fd, "r");

  struct dbuffer *buf = new_dbuffer();

  for (;;)  
    {
      char data[1024];
      size_t len = fread(data, 1, sizeof(data), f);

      if (ferror(f))
        {
          Y_ERROR ("Error reading configuration file %s: %s", filename, strerror (errno));
          fclose(f);
          free_dbuffer(buf);
          return NULL;
        }

      if (len > 0)
        dbuffer_add(buf, data, len);

      if (feof(f))
        break;
    }

  fclose (f);

  struct ykbMapSet *mapset = ykbMapSetCreate();
  struct ykbMap *map = mapset->initial = ykbMapAdd(mapset);
  struct ykbMapNode *node = map->start = ykbMapNodeAdd(map);

  struct Index *keycode_names = indexCreate(keyNameComparisonFunction, keyNameComparisonFunction);
  struct Index *modifier_names = indexCreate(keyNameComparisonFunction, keyNameComparisonFunction);

  int lineNumber = 0;
  while (dbuffer_len(buf) > 0)
    {
      /* Find the end of the line */
      ssize_t pos = dbuffer_find_char(buf, '\n');

      /* If there isn't one, anything left in the buffer is the last line */
      size_t len = (pos < 0) ? dbuffer_len(buf) : (size_t)pos;

      /* Allocate and extract the line */
      char line[len + 1];
      size_t rlen = dbuffer_extract(buf, line, len);
      assert(rlen == len);
      line[len] = '\0';

      /* Take out the trailing \n (if there's anything left) */
      dbuffer_remove(buf, 1);

      lineNumber++;

      /* At this point we have no cleanup commitments; continue and
       * break can be used freely
       */

      /* Study the line briefly, to guess its nature */
      bool hasNonWhitespace = false;
      bool isComment = false;

      for (size_t i = 0; i < len; i++)
        {
          if (line[i] == '#' && !hasNonWhitespace)
            {
              /* A # preceeded only by whitespace is a comment */
              isComment = true;
              break;
            }

          if (!isspace(line[i]))
            hasNonWhitespace = true;
        }

      /* Lines comprised entirely of whitespace are not interesting */
      if (!hasNonWhitespace)
        continue;

      /* Neither are comments */
      if (isComment)
        continue;

      /* Skip leading whitespace */
      char *start = line;
      while (isspace(*start))
        start++;

      /* First word is the keyword */
      char *keyword = start;

      /* Keyword ends with the first whitespace or the end of the line */
      char *keywordend;
      for (keywordend = start; *keywordend; keywordend++)
        if (isspace(*keywordend))
          break;

      /* There might be something after the keyword */
      if (*keywordend)
        start = keywordend + 1;
      else
        start = keywordend;

      /* Either way, terminate it */
      *keywordend = '\0';

      if (strcasecmp(keyword, "name") == 0)
        parseName(filename, lineNumber, start, keycode_names);
      else if (strcasecmp(keyword, "modifier") == 0)
        parseModifierName(filename, lineNumber, start, modifier_names);
      else if (strcasecmp(keyword, "describe") == 0)
        parseDescribe(filename, lineNumber, start, map, keycode_names, modifier_names);
      else if (strcasecmp(keyword, "seq") == 0)
        parseSeq(filename, lineNumber, start, map, node, keycode_names, modifier_names);
      else
        {
          Y_WARN ("%s:%d: Unrecognised keyword '%s'", filename, lineNumber, keyword);
          continue;
        }

      continue;
    }

  indexDestroy(keycode_names, keyNameDestroy_v);
  indexDestroy(modifier_names, keyNameDestroy_v);
  free_dbuffer(buf);

  return mapset;
}

/* arch-tag: 4c4063cc-ae27-4857-96a2-6c051e5ae403
 */
