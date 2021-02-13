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

#ifndef Y_MESSAGE_TUPLE_H
#define Y_MESSAGE_TUPLE_H

#include <inttypes.h>
#include <sys/types.h>
#include <stdbool.h>
#include <string.h>

enum Type
  {
    /* These can be sent over the wire */
    t_string, t_uint32, t_int32,

    /* These are only used locally */

    /* Indicates no value is present */
    t_undef = 256,
    /* An object; will be passed as a uint32 id over the wire */
    t_object,
    /* Indicates a list. Not valid in messages. Precise meaning varies
     * with context
     */
    t_list,
    /* Indicates a single value of any type. Not valid in messages */
    t_any
  };

struct Object;
struct Value
{
  uint32_t type;
  union
  {
    uint32_t uint32;
    int32_t int32;
    struct
    {
      uint32_t len;
      char *data;
    } string;
    struct Object *obj;
    struct Tuple *tuple;
  };
};

struct Tuple
{
  bool error;
  uint32_t count;
  struct Value *list;
};

struct TupleType
{
  uint32_t count;
  enum Type *list;
};

struct MethodType
{
  struct TupleType *args;
  struct TupleType *result;
  void *data;
};

struct MethodTypes
{
  uint32_t count;
  struct MethodType *list;
};

struct Tuple *tupleCreate(uint32_t count);
struct Tuple *tupleDup(const struct Tuple *t);
void tupleDestroy(struct Tuple *);

void valueToString (const struct Value *m, char **str, size_t *len);
bool valueFromString (const char *str, size_t len, struct Value **m);

struct Value *valueCreate(void);
struct Value *valueDup(const struct Value *v);
void valueDestroy(struct Value *v);

const struct MethodType *tupleMatchType(const struct Tuple *, const struct MethodTypes *);
struct Tuple *tupleStaticCast(const struct Tuple *, const struct TupleType *);
bool valueStaticCast(const struct Value *v, struct Value *out, enum Type t);

struct Tuple *tupleBuild_(const struct Value *);
#define tupleBuild(...) tupleBuild_((const struct Value[]){{.type = t_undef}, ##__VA_ARGS__, {.type = t_undef}})
#define tb_string(S) (struct Value){.type = t_string, {.string = {.len = strlen(S), .data = ystrdup(S)}}}
#define tb_uint32(U) (struct Value){.type = t_uint32, {.uint32 = (U)}}
#define tb_int32(I) (struct Value){.type = t_int32, {.int32 = (I)}}
#define tb_object(O) (struct Value){.type = t_object, {.obj = (O)}}
#define tb_value(V) (struct Value)(V)

#define tupleBuildError(...)                                            \
  ({struct Tuple *rm = tupleBuild(__VA_ARGS__);                         \
    rm->error = true;                                                   \
    rm;                                                                 \
  })

#endif

/* arch-tag: fe1b649b-178c-43b6-b0e8-765902f60066
 */
