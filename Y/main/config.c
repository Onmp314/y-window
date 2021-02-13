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
#include <Y/util/yutil.h>
#include <Y/util/index.h>
#include <Y/util/dbuffer.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <Y/main/config.h>
#include <Y/message/tuple.h>
#include <assert.h>
#include <ctype.h>

#include <Y/modules/module.h>
#include <Y/input/keymap.h>
#include <Y/text/font.h>

struct ConfigItem
{
  char *key;
  struct Tuple *t;
};

struct ConfigGroup
{
  char *name;
  struct Index *items;
};

struct Config
{
  char *filename;
  struct Index *groups;
};

static struct ConfigItem *
configItemCreate (const char *key, struct Tuple *t)
{
  struct ConfigItem *ci = ymalloc(sizeof(*ci));
  ci->key = ystrdup(key);
  ci->t = t;
  return ci;
}

static int
configItemKeyFunction (const void *name_v, const void *ci_v)
{
  const char *name = name_v;
  const struct ConfigItem *ci = ci_v;
  return strcmp (name, ci->key);
}

static int
configItemComparisonFunction (const void *ci1_v, const void *ci2_v)
{
  const struct ConfigItem *ci1 = ci1_v, *ci2 = ci2_v;
  return strcmp (ci1->key, ci2->key);
}

static void
configItemDestructorFunction (void *ci_v)
{
  struct ConfigItem *ci = ci_v;
  if (!ci)
    return;
  yfree(ci->key);
  tupleDestroy(ci->t);
  yfree(ci);
}

static struct ConfigGroup *
configGroupCreate (const char *name)
{
  struct ConfigGroup *cg = ymalloc(sizeof(*cg));
  cg->name = ystrdup(name);
  cg->items = indexCreate(configItemKeyFunction, configItemComparisonFunction);
  return cg;
}

static int
configGroupKeyFunction (const void *name_v, const void *cg_v)
{
  const char *name = name_v;
  const struct ConfigGroup *cg = cg_v;
  return strcmp (name, cg->name);
}

static int
configGroupComparisonFunction (const void *cg1_v, const void *cg2_v)
{
  const struct ConfigGroup *cg1 = cg1_v, *cg2 = cg2_v;
  return strcmp (cg1->name, cg2->name);
}

static void
configGroupDestructorFunction (void *cg_v)
{
  struct ConfigGroup *cg = cg_v;
  if (!cg)
    return;
  indexDestroy(cg->items, configItemDestructorFunction);
  yfree(cg->name);
  yfree(cg);
}

static struct Config *
configCreate (const char *filename)
{
  struct Config *conf = ymalloc(sizeof(*conf));
  conf->filename = ystrdup(filename);
  conf->groups = indexCreate(configGroupKeyFunction, configGroupComparisonFunction);
  return conf;
}

void
configDestroy (struct Config *conf)
{
  if (!conf)
    return;
  indexDestroy(conf->groups, configGroupDestructorFunction);
  yfree(conf->filename);
  yfree(conf);
}

static bool
parseQuotedString (const char **c, struct Value *v)
{
  const char *p = *c;
  assert(p[0] == '"');
  p++;

  /* Make a working buffer */
  size_t len = strlen(p);
  char tmp[len + 1];
  size_t tlen = 0;

  /* Classical quoted-string handling with \ escapes */
  while (*p)
    {
      switch (*p)
        {
        case '"':
          *c = p + 1;
          tmp[tlen] = '\0';
          if (v)
            *v = tb_string(tmp);
          return true;

        case '\\':
          p++;
          if (*p == '\0')
            return false;
          switch (*p)
            {
            case 'n':
              tmp[tlen++] = '\n';
              break;
            default:
              tmp[tlen++] = *p;
              break;
            }
          break;

        default:
          tmp[tlen++] = *p;
          break;
        }
      p++;
    }

  /* Falling off the end indicates the string was unterminated. Fail. */
  return false;
}

static bool
parseSignedInteger (const char **c, struct Value *v)
{
  char *end;
  long int i = strtol(*c, &end, 0);

  /* Must be terminated with whitespace */
  if (!isspace(*c))
    return false;

  *c = end;
  if (v)
    {
      v->type = t_int32;
      v->int32 = i;
    }
  return true;
}

static bool
parseUnsignedInteger (const char **c, struct Value *v)
{
  char *end;
  unsigned long int i = strtoul(*c, &end, 0);

  /* Must be terminated with whitespace */
  if (!isspace(*c))
    return false;

  *c = end;
  if (v)
    {
      v->type = t_uint32;
      v->uint32 = i;
    }
  return true;
}

static bool
parseValue (const char **c, struct Value *v)
{
  if ((*c)[0] == '"')
    return parseQuotedString(c, v);
  else if ((*c)[0] == '+' || (*c)[0] == '-')
    return parseSignedInteger(c, v);
  else if (isdigit((*c)[0]))
    return parseUnsignedInteger(c, v);

  /* Otherwise it's an unquoted string, terminated by whitespace or EOL */
  const char *start = *c;
  const char *end = *c;
  while (*end && !isspace(*end))
    end++;
  *c = end;

  if (v)
    {
      v->type = t_string;
      v->string.len = end - start;
      v->string.data = ymalloc(v->string.len + 1);
      memcpy(v->string.data, start, v->string.len);
      v->string.data[v->string.len] = '\0';
    }
  return true;
}

static struct Tuple *
parseTuple (const char *filename, int lineNumber, const char *str)
{
  /* First we validate and count */
  uint32_t count = 0;

  const char *c = str;
  /* This loop iterates over values in the string */
  while (*c)
    {
      /* Skip leading whitespace */
      while (isspace(*c))
        c++;

      if (*c == '\0')
        break;

      if (!parseValue(&c, NULL))
        {
          Y_WARN ("%s:%d: Parse error", filename, lineNumber);
          return NULL;
        }

      /* If we got this far, we were okay for that value */
      count++;
    }

  /* Then we build a tuple */
  c = str;
  struct Tuple *t = tupleCreate(count);
  uint32_t i = 0;

  while (*c)
    {
      /* Skip leading whitespace */
      while (isspace(*c))
        c++;

      if (*c == '\0')
        break;

      if (!parseValue(&c, &t->list[i++]))
        {
          /* This can't happen, it worked before */
          abort();
        }
    }

  assert(i == count);

  return t;
}

struct Config *
configRead (const char *filename)
{
  FILE *f = fopen(filename, "r");
  if (!f)
    {
      Y_ERROR ("Error opening configuration file %s: %s", filename, strerror (errno));
      return NULL;
    }

  /* Read the whole thing into a dbuffer to simplify parsing (config
   * files just aren't *that* big)
   */
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

  fclose(f);

  struct Config *conf = configCreate(filename);

  struct ConfigGroup *currentGroup = NULL;

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
      bool hasTrailingColon = false;
      bool hasInvalidGroup = false;
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
            {
              hasNonWhitespace = true;
              /* Reset the trailing colon condition if we see any
               * non-whitespace
               */
              hasTrailingColon = false;
            }

          if (line[i] == ':')
            hasTrailingColon = true;

          if (!isalnum(line[i]) && !isspace(line[i]) && line[i] != ':')
            {
              /* Group headings may be comprised only of
               * alphanumerics, whitespace, and colons
               */
              hasInvalidGroup = true;
            }
        }

      /* Lines comprised entirely of whitespace are not interesting */
      if (!hasNonWhitespace)
        continue;

      /* Neither are comments */
      if (isComment)
        continue;

      if (hasTrailingColon && !hasInvalidGroup)
        {
          /* Treat it as a group heading */
          char *colon = strchr(line, ':');

          bool invalid = false;

          /* Nothing is allowed after the colon (except whitespace) */
          for (char *c = colon + 1; *c; c++)
            if (!isspace(*c))
              {
                invalid = true;
                break;
              }

          if (invalid)
            {
              Y_WARN ("%s:%d: Parse error (looks like a group heading but isn't)", filename, lineNumber);
              continue;
            }

          /* Clip the colon and any trailing whitespace */
          *colon = '\0';

          /* Skip any leading whitespace */
          char *start = line;
          while (isspace(*start))
            start++;

          /* Whatever remains is the group name; find or allocate the group */
          currentGroup = indexFind(conf->groups, start);
          if (!currentGroup)
            {
              currentGroup = configGroupCreate(start);
              indexAdd(conf->groups, currentGroup);
            }

          continue;
        }

      /* Nothing is allowed before the first group heading */
      if (!currentGroup)
        {
          Y_WARN ("%s:%d: Parse error (was expecting a group heading)", filename, lineNumber);
          continue;
        }

      /* This should be a regular config item */

      /* Skip leading whitespace */
      char *start = line;
      while (isspace(*start))
        start++;

      /* First word is the key name */
      char *key = start;

      /* Key name ends with the first whitespace or the end of the line */
      char *keyend;
      for (keyend = start; *keyend; keyend++)
        if (isspace(*keyend))
          break;

      /* There might be something after the key name */
      if (*keyend)
        start = keyend + 1;
      else
        start = keyend;

      /* Either way, terminate it */
      *keyend = '\0';

      /* Duplicate? */
      if (indexFind(currentGroup->items, key))
        {
          Y_WARN ("%s:%d: Duplicate config key '%s' (ignoring)", filename, lineNumber, key);
          continue;
        }

      /* Parse whatever is left */
      struct Tuple *t = parseTuple(filename, lineNumber, start);
      if (!t)
        continue;

      struct ConfigItem *ci = configItemCreate(key, t);
      indexAdd(currentGroup->items, ci);
    }

  free_dbuffer(buf);

  return conf;
}

struct Tuple *
configGet(const struct Config *conf, const char *group, const char *key, const struct TupleType *type)
{
  if (!conf || !group || !key)
    return NULL;

  struct ConfigGroup *cg = indexFind(conf->groups, group);
  if (!cg)
    return NULL;

  struct ConfigItem *ci = indexFind(cg->items, key);
  if (!ci)
    return NULL;

  return tupleStaticCast(ci->t, type);
}

struct ConfigKeyIterator *
configGetKeyIterator(const struct Config *conf, const char *group)
{
  if (!conf || !group)
    return NULL;

  struct ConfigGroup *cg = indexFind(conf->groups, group);
  if (!cg)
    return NULL;

  return (struct ConfigKeyIterator *)indexGetStartIterator(cg->items);
}

void
configKeyIteratorNext(struct ConfigKeyIterator *i)
{
  indexiteratorNext((struct IndexIterator *)i);
}

void
configKeyIteratorDestroy(struct ConfigKeyIterator *i)
{
  indexiteratorDestroy((struct IndexIterator *)i);
}

bool
configKeyIteratorHasValue(struct ConfigKeyIterator *i)
{
  return indexiteratorHasValue((struct IndexIterator *)i);
}

const char *
configKeyIteratorName(struct ConfigKeyIterator *i)
{
  struct ConfigItem *ci = indexiteratorGet((struct IndexIterator *)i);
  return ci->key;
}

struct Tuple *
configKeyIteratorValue(struct ConfigKeyIterator *i, const struct TupleType *type)
{
  struct ConfigItem *ci = indexiteratorGet((struct IndexIterator *)i);
  return tupleStaticCast(ci->t, type);
}

/* arch-tag: 2145afe4-a40d-452c-a9a9-7f53d5ed8475
 */
