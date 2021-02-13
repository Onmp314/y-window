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

#include <Y/y.h>
#include <Y/main/control.h>
#include <Y/widget/widget.h>
#include <Y/input/ykb.h>
#include <Y/input/ykbmap.h>
#include <Y/input/ykbmap_p.h>
#include <Y/util/yutil.h>

#include <stddef.h>
#include <assert.h>
#include <stdio.h>

static struct ykbMapSet *mapset = NULL;
static struct ykbMap *map = NULL;
static struct ykbMapNode *state = NULL;
static struct Widget *focus = NULL;
static uint16_t modifiers = 0;
static uint16_t sticky_modifiers = 0;

/* Values in msec */
static uint32_t repeat_delay = 500;
static uint32_t repeat_rate = 50;

static int repeat_timer_id = 0;
static uint16_t repeat_keycode = 0;

static struct ykbMapSet *config_mapset = NULL;

void
ykbInitialise (struct Config *serverConfig)
{
  struct TupleType layoutType = {.count = 1, .list = (enum Type []) {t_string}};
  struct Tuple *layoutTuple = configGet(serverConfig, "keymap", "layout", &layoutType);

  if (!layoutTuple)
    {
      Y_WARN("No keymap:layout specified in config file; not loading a keymap");
      return;
    }

  assert(layoutTuple->count >= 1);
  if (layoutTuple->error)
    {
      Y_WARN("Error retrieving keymap:layout from config file: %s", layoutTuple->list[0].string.data);
      tupleDestroy(layoutTuple);
      return;
    }

  const char *layout = layoutTuple->list[0].string.data;
  static const char keymapfmt[] = "%s/keymaps/%s.ykb";
  char path[strlen(keymapfmt) + strlen(yConfigDir) + strlen(layout) + 1];
  snprintf(path, sizeof(path), keymapfmt, yConfigDir, layout);
  config_mapset = ykbLoadMapSet(path);
  ykbUseMapSet(config_mapset);
  tupleDestroy(layoutTuple);
}

void
ykbFinalise(void)
{
  ykbUseMapSet(NULL);
  ykbMapSetDestroy(config_mapset);
}

void
ykbSetFocus (struct Widget *w)
{
  focus = w;
}

static void
ykbDispatchString(const char *str)
{
  Y_DEBUG("YKB dispatch string '%s'", str);
  if (!focus)
    return;
  widgetykbString(focus, str, modifiers | sticky_modifiers);
}

static void
ykbDispatchEvent(const char *event)
{
  Y_DEBUG("YKB dispatch event '%s'", event);
  if (!focus)
    return;
  widgetykbEvent(focus, event, modifiers | sticky_modifiers);
}

static void
ykbDispatchStroke(bool direction, uint16_t keycode)
{
  if (!focus)
    return;
  widgetykbStroke(focus, direction, keycode, modifiers | sticky_modifiers);
}

static void
ykbAct(struct ykbAction *action)
{
  switch (action->type)
    {
    case ykbaSetModifiers:
      modifiers |= action->modifiers;
      break;
    case ykbaMaskModifiers:
      modifiers &= action->modifiers;
      break;
    case ykbaToggleModifiers:
      modifiers ^= action->modifiers;
      break;
    case ykbaSetStickyModifiers:
      sticky_modifiers |= action->modifiers;
      break;
    case ykbaMaskStickyModifiers:
      sticky_modifiers &= action->modifiers;
      break;
    case ykbaToggleStickyModifiers:
      sticky_modifiers ^= action->modifiers;
      break;
    case ykbaBeginExtended:
      break;
    case ykbaAbortExtended:
      break;
    case ykbaSetKeymap:
      map = action->map;
      break;
    case ykbaString:
      ykbDispatchString(action->str);
      break;
    case ykbaEvent:
      ykbDispatchEvent(action->str);
      break;
    }
}

static void
ykbSetState(struct ykbMapNode *to)
{
  state = to;
}

static void
ykbTransition(struct ykbMapEdge *edge)
{
  if (!edge)
    return;

  ykbSetState(edge->destination ? edge->destination : map->start);
  for (struct llist_node *n = llist_head (edge->actions);
       n != NULL;
       n = llist_node_next (n))
    {
      struct ykbAction *action = llist_node_data (n);
      ykbAct(action);
    }
}

static void
ykbKeyStroke(bool direction, uint16_t keycode)
{
  ykbDispatchStroke(direction, keycode);

  if (!state)
    return;

  struct ykbMapEdge *edge = ykbMapLookup(state, direction, keycode, modifiers | sticky_modifiers);
  if (!edge)
    {
      /* This keystroke doesn't form a valid sequence */

      /* If we're at the start, discard this keystroke */
      if (state == map->start)
        return;

      /* Otherwise, go back to the start and try again */
      ykbSetState(map->start);
      edge = ykbMapLookup(state, direction, keycode, modifiers | sticky_modifiers);

      /* If there's nothing at the start either, discard it */
      if (!edge)
        return;
    }

  ykbTransition(edge);
}

static void
ykbRepeat (void *data)
{
  Y_DEBUG("YKB key repeat %d", repeat_keycode);
  ykbKeyStroke(true, repeat_keycode);

  repeat_timer_id = controlTimerDelay(0, repeat_rate, NULL, &ykbRepeat);
}

void
ykbKeyDown (uint16_t keycode)
{
  char *desc = ykbDescribe(keycode, modifiers);
  Y_DEBUG("YKB key down %d (%s)", keycode, desc);
  yfree(desc);
  ykbKeyStroke(true, keycode);

  repeat_keycode = keycode;
  if (repeat_timer_id != 0)
    controlCancelTimerDelay(repeat_timer_id);
  repeat_timer_id = controlTimerDelay(0, repeat_delay, NULL, &ykbRepeat);
}

void
ykbKeyUp (uint16_t keycode)
{
  Y_DEBUG("YKB key up %d", keycode);
  ykbKeyStroke(false, keycode);

  if (repeat_timer_id != 0 && repeat_keycode == keycode)
    {
      controlCancelTimerDelay(repeat_timer_id);
      repeat_timer_id = 0;
      repeat_keycode = 0;
    }
}

void
ykbUseMapSet(struct ykbMapSet *mapset_)
{
  mapset = mapset_;
  map = mapset ? mapset->initial : NULL;
  ykbSetState(map ? map->start : NULL);
}

/* Note that this function allocates a new string; the caller must
 * free it
 */
char *
ykbDescribe(uint16_t keycode, uint16_t modifiers)
{
  struct ykbKeyDescription *key_desc = ykbKeyLookup(map, keycode);
  if (!key_desc)
    return NULL;

  /* Length of the description string we're going to create */
  size_t len = strlen(key_desc->str);

  struct llist *mods = new_llist();

  for (struct llist_node *n = llist_head(map->modifier_descriptions); n; n = llist_node_next(n))
    {
      struct ykbModifierDescription *mod_desc = llist_node_data(n);
      if ((modifiers & mod_desc->mask) == mod_desc->modifiers)
        {
          len += strlen(mod_desc->str) + 1;
          llist_add_tail(mods, mod_desc);
        }
    }

  char *str = ymalloc(len + 1);
  char *c = str;

  for (struct llist_node *n = llist_head(mods); n; n = llist_node_next(n))
    {
      struct ykbModifierDescription *mod_desc = llist_node_data(n);
      size_t l = strlen(mod_desc->str);
      memcpy(c, mod_desc->str, l);
      c += l;
      *c++ = '-';
    }

  size_t l = strlen(key_desc->str);
  memcpy(c, key_desc->str, l);
  c += l;
  *c = '\0';
  assert(c == (str + len));

  return str;
}

/* arch-tag: 3f5d46aa-f7bf-4111-b217-9a9cdcfed7c4
 */
