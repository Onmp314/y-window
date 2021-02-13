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

#include <message/tuple.h>
#include <object/object.h>
#include <assert.h>
#include <Y/util/yutil.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdbool.h>

#include "parse_support.h"

struct Value *
valueCreate(void)
{
  struct Value *v = ymalloc(sizeof(*v));
  v->type = t_undef;
  return v;
}

struct Value *
valueDup(const struct Value *from)
{
  struct Value *to = valueCreate();

  switch((enum Type)from->type)
    {
    case t_uint32:
      to->type = t_uint32;
      to->uint32 = from->uint32;
      break;
    case t_int32:
      to->type = t_int32;
      to->int32 = from->int32;
      break;
    case t_object:
      to->type = t_object;
      to->obj = from->obj;
      break;
    case t_string:
      to->type = t_string;
      to->string.len = from->string.len;
      to->string.data = ymalloc(from->string.len + 1);
      memcpy(to->string.data, from->string.data, from->string.len);
      to->string.data[from->string.len] = '\0';
      break;
    case t_undef:
      to->type = t_undef;
      break;
    case t_any:
    case t_list:
      abort();
    }
  return to;
}

void
valueDestroy(struct Value *v)
{
  if (!v)
    return;

  switch((enum Type)v->type)
    {
    case t_string:
      yfree(v->string.data);
      break;
    case t_uint32:
    case t_int32:
    case t_object:
    case t_undef:
    case t_list:
    case t_any:
      break;
    }

  yfree(v);
}

struct Tuple *
tupleCreate (uint32_t count)
{
  struct Tuple *t = ymalloc (sizeof (*t));
  t->error = false;
  t->count = count;
  if (count > 0)
    {
      t->list = ymalloc(sizeof(t->list[0]) * count);
      for (uint32_t i = 0; i < count; i++)
       t->list[i].type = t_undef;
    }
  else
    t->list = NULL;
  return t;
}

struct Tuple *
tupleDup (const struct Tuple *from)
{
  struct Tuple *t = ymalloc (sizeof (*t));
  t->error = from->error;
  t->count = from->count;
  if (from->count > 0)
    {
      t->list = ymalloc(sizeof(t->list[0]) * from->count);
      for (uint32_t i = 0; i < from->count; i++)
        switch((enum Type)from->list[i].type)
          {
          case t_uint32:
            t->list[i].type = t_uint32;
            t->list[i].uint32 = from->list[i].uint32;
            break;
          case t_int32:
            t->list[i].type = t_int32;
            t->list[i].int32 = from->list[i].int32;
            break;
          case t_object:
            t->list[i].type = t_object;
            t->list[i].obj = from->list[i].obj;
            break;
          case t_string:
            t->list[i].type = t_string;
            t->list[i].string.len = from->list[i].string.len;
            t->list[i].string.data = ymalloc(from->list[i].string.len + 1);
            memcpy(t->list[i].string.data, from->list[i].string.data, from->list[i].string.len);
            t->list[i].string.data[from->list[i].string.len] = '\0';
            break;
          case t_undef:
            t->list[i].type = t_undef;
            break;
          case t_any:
          case t_list:
            abort();
          }
    }
  else
    t->list = NULL;
  return t;
}

void
tupleDestroy (struct Tuple *t)
{
  if (t == NULL)
    return;

  for (uint32_t i = 0; i < t->count; i++)
    switch((enum Type)t->list[i].type)
      {
      case t_string:
        yfree(t->list[i].string.data);
        break;
      case t_uint32:
      case t_int32:
      case t_object:
      case t_undef:
      case t_list:
      case t_any:
        break;
      }
  yfree(t->list);
  yfree(t);
}

struct Tuple *
tupleBuild_(const struct Value *list)
{
  /* First thing in the list is a placeholder t_undef */
  list++;

  /* Count the elements */
  uint32_t count = 0;
  for (count = 0; list[count].type != t_undef; count++)
    ;

  /* Build the tuple */
  struct Tuple *t = tupleCreate(count);
  for (uint32_t i = 0; i < count; i++)
    memcpy(&t->list[i], &list[i], sizeof(list[i]));

  return t;
}

void
valueToString (const struct Value *m, char **str, size_t *slen)
{
  if (!m)
    {
      if (str)
        *str = NULL;
      *slen = 0;
      return;
    }

  *slen = 0;
  *slen += sizeof(m->type);
  switch((enum Type)m->type)
    {
    case t_string:
      *slen += m->string.len;
      break;
    case t_object:
    case t_uint32:
      *slen += sizeof(m->uint32);
      break;
    case t_int32:
      *slen += sizeof(m->int32);
      break;
    default:
      abort();
    }

  if (!str)
    return;

  *str = ymalloc(*slen);
  char *p = *str;

  uint32_t type;
  switch((enum Type)m->type)
    {
    case t_object:
      type = htonl(t_uint32);
      break;
    case t_string:
    case t_uint32:
    case t_int32:
      type = htonl(m->type);
      break;
    default:
      abort();
    }
  ADD_SCALAR(p, type);

  switch((enum Type)m->type)
    {
    case t_string:
      {
        ADD_DATA(p, m->string.data, m->string.len);
        break;
      }
    case t_uint32:
      {
        uint32_t uint32 = htonl(m->uint32);
        ADD_SCALAR(p, uint32);
        break;
      }
    case t_object:
      {
        uint32_t oid = htonl(objectGetID(m->obj));
        ADD_SCALAR(p, oid);
        break;
      }
    case t_int32:
      {
        int32_t int32 = htonl(m->int32);
        ADD_SCALAR(p, int32);
        break;
      }
    default:
      abort();
    }

  assert(p == (*str + *slen));
}

bool
valueFromString (const char *str, size_t slen, struct Value **m)
{
  if (!str || slen == 0)
    {
      if (m)
        *m = NULL;
      return 0;
    }

  struct Value tmp;
  /* Can't keep this in tmp, it must be const */
  const char *tmp_string_data = NULL;

  const char *p = str;
  size_t l = slen;

  uint32_t type;
  if (!GET_SCALAR(p, l, type))
    return false;
  tmp.type = ntohl(type);

  switch((enum Type)tmp.type)
    {
    case t_string:
      {
        tmp.string.len = l;
        tmp_string_data = p;
        if (!SKIP_DATA(p, l, tmp.string.len))
          return false;
        break;
      }
    case t_uint32:
      {
        uint32_t uint32;
        if (!GET_SCALAR(p, l, uint32))
          return false;
        tmp.uint32 = ntohl(uint32);
        break;
      }
    case t_int32:
      {
        int32_t int32;
        if (!GET_SCALAR(p, l, int32))
          return false;
        tmp.int32 = ntohl(int32);
        break;
      }
    default:
      return false;
    }

  if (l != 0)
    return false;

  if (!m)
    return true;

  *m = ymalloc(sizeof(**m));
  (*m)->type = tmp.type;
  switch((enum Type)tmp.type)
    {
    case t_string:
      /* We allocate one extra byte and stuff a NULL in there, so it
       * can be treated as ASCIIZ if appropriate
       */
      (*m)->string.data = ymalloc(tmp.string.len + 1);
      memcpy((*m)->string.data, tmp_string_data, tmp.string.len);
      (*m)->string.data[tmp.string.len] = '\0';
      (*m)->string.len = tmp.string.len;
      break;
    case t_uint32:
      (*m)->uint32 = tmp.uint32;
      break;
    case t_int32:
      (*m)->int32 = tmp.int32;
      break;
    default:
      return false;
    }

  return true;
}

/* This function is basically a matrix between the various
 * types. Here's a little ascii-art:
 *
 *                                 to
 *          |  string | int32 |  uint32 |  object |  list |
 *   -------|---------|-------|---------|---------|-------|
 * f string |    N    |   C   |    C    |         |       |
 * r int32  |         |   N   |    C    |         |       |
 * o uint32 |         |   C   |    N    |    C    |       |
 * m object |         |       |    C    |    N    |       |
 *   list   |         |       |         |         |   N   |
 *
 * Key: N is a no-op, C is a conversion, blank is a failure
 * Rows are the value type, columns are the target type
 *
 * undef is not present because this is an error; is should be handled
 * by tupleStaticCast instead. any matches everything in a type, and
 * needs no conversion. any is an error in a value.
 *
 * list casts might be supported in the future, but currently aren't
 *
 * out should be either v or NULL; this allows valueStaticCast to be
 * used to check types as well as cast them
 */

bool
valueStaticCast(const struct Value *v, struct Value *out, enum Type t)
{
  assert(out == NULL || out == v);
  switch(t)
    {
    case t_string:
      switch((enum Type)v->type)
        {
        case t_string:
          return true;
        case t_uint32:
        case t_int32:
        case t_object:
        case t_list:
          return false;
        case t_undef:
        case t_any:
          abort();
        }
      abort();
    case t_uint32:
      switch((enum Type)v->type)
        {
        case t_uint32:
          return true;
        case t_int32:
          if (v->int32 < 0)
            return false;
          if (out)
            {
              uint32_t tmp = v->int32;
              out->type = t_uint32;
              out->uint32 = tmp;
            }
          return true;
        case t_object:
          if (out)
            {
              uint32_t tmp = v->obj ? objectGetID(v->obj) : 0;
              out->type = t_uint32;
              out->uint32 = tmp;
            }
          return true;
        case t_string:
          if (out)
            {
              uint32_t tmp = strtoul (v->string.data, NULL, 0); 
              yfree (out->string.data);
              out->string.data = NULL;
              out->type = t_uint32;
              out->uint32 = tmp;
            }
          return true;
        case t_list:
          return false;
        case t_any:
        case t_undef:
          abort();
        }
      abort();
    case t_int32:
      switch((enum Type)v->type)
        {
        case t_int32:
          return true;
        case t_uint32:
          if (v->uint32 > INT32_MAX)
            return false;
          if (out)
            {
              int32_t tmp = v->uint32;
              out->type = t_int32;
              out->int32 = tmp;
            }
          return true;
        case t_object:
          return false;
        case t_string:
          if (out)
            {
              int32_t tmp = strtol (v->string.data, NULL, 0); 
              yfree (out->string.data);
              out->string.data = NULL;
              out->type = t_int32;
              out->int32 = tmp;
            }
          return true;
        case t_list:
          return false;
        case t_any:
        case t_undef:
          abort();
        }
      abort();
    case t_object:
      switch((enum Type)v->type)
        {
        case t_object:
          return true;
        case t_uint32:
          {
            struct Object *obj = objectFind(v->uint32);
            if (!obj)
              return false;
            if (out)
              {
                out->type = t_object;
                out->obj = obj;
              }
            return true;
          }
        case t_int32:
        case t_string:
        case t_list:
          return false;
        case t_any:
        case t_undef:
          abort();
        }
      abort();
    case t_list:
      return v->type == t_list;
    case t_any:
      return true;
    case t_undef:
    default:
      /* These should never happen */
      abort();
    }
}

static bool
tupleCheckType(const struct Tuple *t, const struct TupleType *type, bool perfect)
{
  assert(type);
  if (!t)
    {
      if (type->count == 0)
        return true;
      if (type->count == 1 && type->list[0] == t_list)
        return true;
      return false;
    }

  for (uint32_t i = 0; i < type->count; i++)
    {
      switch(type->list[i])
        {
        case t_string:
        case t_uint32:
        case t_int32:
        case t_object:
          /* For the fundamental types, check that it exists...
           */
          if (t->count < i)
            return false;
          /* ...and is the right type */
          if (perfect)
            {
              if (t->list[i].type != type->list[i])
                return false;
            }
          else
            {
              if (!valueStaticCast(&t->list[i], NULL, type->list[i]))
                return false;
            }
          break;
        case t_any:
          /* All this one has to do is exist */
          return t->count >= i;
        case t_undef:
          /* undef is not valid in a type */
          abort();
        case t_list:
          /* A list can only come at the end of a type */
          assert(i == type->count - 1);
          /* A list either matches anything or nothing */
          return !perfect;
        }
    }
  /* Lengths must match if we didn't finish on a list */
  return type->count == t->count;
}

const struct MethodType *
tupleMatchType(const struct Tuple *t, const struct MethodTypes *types)
{
  /* Can't do anything with this */
  if (types->count == 0)
    return NULL;

  /* If we have a perfect match, with no lists, we take it */
  for (uint32_t i = 0; i < types->count; i++)
    if (tupleCheckType(t, types->list[i].args, true))
      return &types->list[i];

  /* If there's a type which is just a single list, that's our
   * fallback if matching fails. fallback is NULL if there isn't one,
   * and we just return it if matching fails.
   */
  struct MethodType *fallback = NULL;
  for (uint32_t i = 0; i < types->count; i++)
    if (types->list[i].args->count == 1 && types->list[i].args->list[0] == t_list)
      {
        /* Two of these is an error */
        if (fallback)
          return NULL;
        fallback = &types->list[i];
      }

  /* We need to pick an imperfect match
   *
   * Start with all the things that could possibly match
   */
  uint32_t candidate_count = 0;
  uint32_t candidates[types->count];
  for (uint32_t i = 0; i < types->count; i++)
    if (tupleCheckType(t, types->list[i].args, false))
      {
        candidates[candidate_count++] = i;
      }

  /* Work from left to right in the tuple.
   */
  if (t)
    for (uint32_t pos = 0; pos < t->count; pos++)
      {
        /* If we have precisely one candidate then that's our solution
         */
        if (candidate_count == 1)
          return &types->list[candidates[0]];

        /* If we have no candidates then stop */
        if (candidate_count == 0)
          return fallback;

        uint32_t new_candidate_count = 0;
        uint32_t new_candidates[types->count];
        /* Build a list of perfect matches on the current value */
        for (uint32_t i = 0; i < candidate_count; i++)
          {
            const struct TupleType *type = types->list[candidates[i]].args;

            /* If it's too short, it's no good */
            if (type->count < pos)
              continue;
            if (t->list[pos].type == type->list[pos])
              new_candidates[new_candidate_count++] = candidates[i];
          }

        /* If we got any perfect matches, forget any other candidates and
         * proceed to the next value
         */
        if (new_candidate_count > 0)
          {
            candidate_count = new_candidate_count;
            for (uint32_t i = 0; i < new_candidate_count; i++)
              candidates[i] = new_candidates[i];
            continue;
          }

        /* No perfect match on the current value. Try for a cast match */
        for (uint32_t i = 0; i < candidate_count; i++)
          {
            const struct TupleType *type = types->list[candidates[i]].args;
            if (type->count < pos)
              continue;

            /* We'll ignore list matches for now */
            if (type->list[pos] == t_list)
              continue;

            /* Try the cast */
            if (valueStaticCast(&t->list[pos], NULL, type->list[pos]))
              new_candidates[new_candidate_count++] = candidates[i];
          }

        /* If we got any imperfect matches, go with them
         */
        if (new_candidate_count > 0)
          {
            candidate_count = new_candidate_count;
            for (uint32_t i = 0; i < new_candidate_count; i++)
              candidates[i] = new_candidates[i];
            continue;
          }

        /* If there is precisely one list match, take it */
        struct MethodType *list_candidate = NULL;

        for (uint32_t i = 0; i < candidate_count; i++)
          {
            const struct TupleType *type = types->list[candidates[i]].args;
            if (type->count < pos)
              continue;
            if (type->list[pos] == t_list)
              {
                if (list_candidate)
                  {
                    /* Ambiguous on a list match. Forget it */
                    return false;
                  }
                list_candidate = &types->list[candidates[i]];
              }
          }

        if (list_candidate)
          return list_candidate;

        /* No match */
        return fallback;
      }

  if (candidate_count == 1)
    return &types->list[candidates[0]];

  return fallback;
}

struct Tuple *
tupleStaticCast(const struct Tuple *t, const struct TupleType *type)
{
  assert(type);
  if (!t)
    {
      /* "No tuple" is a zero-length tuple; can the type accept that?
       */

      /* Zero length is okay */
      if (type->count == 0)
        return tupleBuild();
      /* List is okay */
      if (type->count == 1 && type->list[0] == t_list)
        return tupleBuild();
      /* Anything else is not */
      return NULL;
    }

  /* We'll duplicate it and try to coerce each element as we go */
  struct Tuple *rt = tupleDup(t);

  for (uint32_t i = 0; i < type->count; i++)
    {
      assert(type->list[i] != t_undef);
      if (type->list[i] == t_list)
        {
          /* A list can only come at the end of a type */
          assert(i == type->count - 1);
          /* A list matches everything else */
          return rt;
        }
      /* Check that the value is present */
      if (rt->count < i)
        {
          tupleDestroy(rt);
          return NULL;
        }
      /* and valid */
      switch (rt->list[i].type)
        {
        case t_string:
        case t_uint32:
        case t_int32:
        case t_object:
          break;
        case t_any:
        case t_list:
        case t_undef:
          tupleDestroy(rt);
          return NULL;
        }
      /* and try to cast it */
      if (!valueStaticCast(&rt->list[i], &rt->list[i], type->list[i]))
        {
          tupleDestroy(rt);
          return NULL;
        }
    }
  /* If we got this far, we didn't finish on a list, so the value
   * counts must match - trailing values aren't allowed
   */
  if (type->count == rt->count)
    return rt;
  else
    {
      tupleDestroy(rt);
      return NULL;
    }
}

/* arch-tag: c69288be-87ae-4d4b-b53f-37677ac70648
 */
