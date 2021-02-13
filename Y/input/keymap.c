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

#include <Y/y.h>
#include <Y/main/config.h>
#include <Y/const.h>
#include <Y/input/keymap.h>

#include <Y/util/yutil.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

struct KeyMapping
{
  int modifierState;
  enum YKeyCode key;
  struct KeyMapping *nextMapping;
};

static struct KeyMapping **keymapMappings = NULL;

void
keymapInitialise (struct Config *serverConfig)
{
  keymapMappings = ymalloc (sizeof (struct KeyMapping *) * YK_LAST);
  memset (keymapMappings, 0, sizeof (struct KeyMapping *) * YK_LAST);

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
  keymapLoad(layout);
  tupleDestroy(layoutTuple);
}

void
keymapFinalise ()
{
  int i;
  for (i=0; i < YK_LAST; ++i)
    {
      struct KeyMapping *mapping = keymapMappings[i];
      while (mapping != NULL)
        {
          struct KeyMapping *next = mapping -> nextMapping;
          yfree (mapping);
          mapping = next;
        } 
    }
  yfree (keymapMappings);
}

static void
keymapParseLine (char *line)
{
  char *rhs;
  int comment;
  char *cp, *np;
  int keycode, modifiers, mapsto;
  struct KeyMapping *mapping;

  if (line[0] == '#')
    return;
  /* remove all internal comments (delimted by [] characters) */
  rhs = line;
  comment = 0;
  while (*rhs != '\0')
    {
      char c = *rhs;
      if (comment)
        *rhs = ' ';
      if (c == '[')
        comment = 1;
      if (c == ']')
        comment = 0;
      ++rhs; 
    }

  rhs = strchr (line, '=');
  if (rhs == NULL)
    return;
  *(rhs++) = '\0';

  cp = strstr (line, "keycode ");
  if (cp == NULL)
    return;

  cp += 8;
  keycode = strtoul (cp, NULL, 0);
  if (keycode == 0 || keycode >= YK_LAST)
    return; 

  cp = rhs;

  while (cp != NULL)
    {
      rhs = cp;
      cp = strchr (rhs, ',');
      if (cp != NULL)
        *(cp++) = '\0';

      modifiers = 0;
      mapsto = 0;

      /* parse modifiers of the form SHIFT ALT CTRL META MODE NUMLOCK (id) */
      np = strchr (rhs, '(');
      if (np == NULL)
        continue;      

      mapsto = strtoul (np + 1, NULL, 0);
      if (mapsto == 0)
        continue;

      if (strstr (rhs, "SHIFT") != NULL)
        modifiers |= YMOD_SHIFT;
      if (strstr (rhs, "ALT") != NULL)
        modifiers |= YMOD_ALT;
      if (strstr (rhs, "CTRL") != NULL)
        modifiers |= YMOD_CTRL;
      if (strstr (rhs, "META") != NULL)
        modifiers |= YMOD_META;
      if (strstr (rhs, "MODE") != NULL)
        modifiers |= YMOD_MODE;
      if (strstr (rhs, "NUMLOCK") != NULL)
        modifiers |= YMOD_NUMLOCK;

      mapping = ymalloc (sizeof (struct KeyMapping));
      mapping -> modifierState = modifiers;
      mapping -> key = mapsto;
      mapping -> nextMapping = keymapMappings[keycode];
      keymapMappings[keycode] = mapping;

    }

}

void
keymapLoad (const char *keymap)
{
  int l1 = strlen (yConfigDir);
  int l2 = strlen (keymap);
  char path[l1 + l2 + 18]; /* /keymaps/ .Ykeymap */
  FILE *file;
  int filed;
  char buffer[1024];
  size_t ret;
  int position;
  strcpy (path, yConfigDir);
  strcpy (path + l1, "/keymaps/");
  strcpy (path + l1 + 9, keymap);
  strcpy (path + l1 + l2 + 9, ".Ykeymap");

  filed = open (path, O_RDONLY | O_NOCTTY);

  if (filed < 0)
    {
      Y_ERROR ("Error loading keymap %s (at %s): %s", keymap, path, strerror (errno));
      return;
    }

  file = fdopen (filed, "r");

  position = 0;
  do
    {
      ret = fread (buffer + position, sizeof (char), 1, file);
      if (ret > 0)
        {
          if (buffer[position] == '\n')
            {
              buffer[position] = '\0';
              keymapParseLine (buffer);
              position = 0;
            }
          else
            {
              position++;
              if (position >= 1023)
                position = 1023;
            }
        }
    }
  while (ret > 0);

  if (ferror (file))
    Y_ERROR ("Error reading keymap file %s: %s", path, strerror (errno));

  fclose (file);
}

enum YKeyCode
keymapMapKey (enum YKeyCode key, int modifierState)
{
  struct KeyMapping * map;

  if (key >= YK_LAST)
    return YK_UNKNOWN;

  map = keymapMappings[key];
  while (map != NULL)
    {
      if (map -> modifierState == modifierState)
        return map -> key;
      map = map -> nextMapping;
    }

  return key;
}

/* arch-tag: 0a19aef9-6ba6-4cb2-9ab6-13286e604a56
 */
