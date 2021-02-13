/************************************************************************
 *   Copyright (C) Mark Thomas <markbt@efaref.net>
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

#define CHECK_STOP abort()
#include <Y/util/check.h>

#include <Y/util/rbtree.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

const char *checkName;
const char *checkModule;

#define FUNCTIONALITY_NUM_CHECK 20
#define ITERATORS_NUM_CHECK 20

struct rbtree_check_Functionality
{
  int checkCode;
  char *rbtreeValue;
};

static int
rbtree_check_functionality_keyFunction (const void *key_v, const void *obj_v)
{
  const char *key = key_v;
  const struct rbtree_check_Functionality *obj = obj_v;
  return strcmp (key, obj->rbtreeValue);
}

static int functionality_destruction[FUNCTIONALITY_NUM_CHECK];
static int functionality_iteration[FUNCTIONALITY_NUM_CHECK];

static void
rbtree_check_functionality_destructorFunction (void *obj_v)
{
  struct rbtree_check_Functionality *obj = obj_v;
  ++functionality_destruction[obj->checkCode];
  free (obj->rbtreeValue);
  free (obj);
}

static int
rbtree_check_functionality_comparisonFunction (const void *obj1_v, const void *obj2_v)
{
  const struct rbtree_check_Functionality *obj1 = obj1_v;
  const struct rbtree_check_Functionality *obj2 = obj2_v;
  return strcmp (obj1->rbtreeValue, obj2->rbtreeValue);
} 

static int rbtree_check_functionality_iterationUserDataOK;

static void
rbtree_check_functionality_iterationFunction (void *obj_v, void *userData)
{
  const struct rbtree_check_Functionality *obj = obj_v;
  if (userData != (void *)0x12345678)
    rbtree_check_functionality_iterationUserDataOK = 0;
  functionality_iteration[obj->checkCode]++;
}

static void
rbtree_check_functionality_printf (void *obj_v, void *userData)
{
  const struct rbtree_check_Functionality *obj = obj_v;
  printf("%s\n", obj->rbtreeValue);
}

static int
rbtree_check_functionality (void)
{
  struct rbtree *rbtree; 
  struct rbtree_check_Functionality *objs[FUNCTIONALITY_NUM_CHECK];
  int i;

  checkModule = "functionality";

  rbtree = new_rbtree (rbtree_check_functionality_keyFunction,
                       rbtree_check_functionality_comparisonFunction);

  CHECK_THAT ( rbtree != NULL );

  for (i=0; i<FUNCTIONALITY_NUM_CHECK; ++i)
    {
      objs[i] = malloc (sizeof (struct rbtree_check_Functionality));
      objs[i]->checkCode = i;
      objs[i]->rbtreeValue = malloc (40);
      snprintf (objs[i]->rbtreeValue, 39, "Value %d", i);
      objs[i]->rbtreeValue[39] ='\0';
      rbtree_insert (rbtree, objs[i]);
    }

  rbtree_check_sanity (rbtree);

  CHECK_THAT ( rbtree_node_data (rbtree_find (rbtree, "Value 4")) == objs[4] );
  CHECK_THAT ( rbtree_find (rbtree, "Something Random") == NULL );
  rbtree_node_delete (rbtree_find (rbtree, "Value 16"));
  CHECK_THAT ( rbtree_find (rbtree, "Value 16") == NULL );
  CHECK_THAT ( rbtree_node_data (rbtree_find (rbtree, "Value 14")) == objs[14] );
  CHECK_THAT ( rbtree_node_data (rbtree_find (rbtree, "Value 15")) == objs[15] );
  CHECK_THAT ( rbtree_find (rbtree, "Value 16") == NULL );
  CHECK_THAT ( rbtree_node_data (rbtree_find (rbtree, "Value 17")) == objs[17] );
  CHECK_THAT ( rbtree_node_data (rbtree_find (rbtree, "Value 18")) == objs[18] );
  rbtree_node_delete (rbtree_find (rbtree, "Value 0"));
  CHECK_THAT ( rbtree_find (rbtree, "Value 0") == NULL );
  CHECK_THAT ( rbtree_node_data (rbtree_find (rbtree, "Value 1")) == objs[1] );
  rbtree_node_delete (rbtree_find (rbtree, "Value 1"));
  CHECK_THAT ( rbtree_node_data (rbtree_find (rbtree, "Value 2")) == objs[2] );
  rbtree_node_delete (rbtree_find (rbtree, "Value 2"));
  CHECK_THAT ( rbtree_node_data (rbtree_find (rbtree, "Value 3")) == objs[3] );
  rbtree_node_delete (rbtree_find (rbtree, "Value 3"));
  CHECK_THAT ( rbtree_node_data (rbtree_find (rbtree, "Value 4")) == objs[4] );
  rbtree_node_delete (rbtree_find (rbtree, "Value 4"));

  CHECK_THAT ( rbtree_node_data (rbtree_find (rbtree, "Value 5")) == objs[5] );
  CHECK_THAT ( rbtree_node_data (rbtree_find (rbtree, "Value 6")) == objs[6] );
  CHECK_THAT ( rbtree_node_data (rbtree_find (rbtree, "Value 7")) == objs[7] );
  CHECK_THAT ( rbtree_node_data (rbtree_find (rbtree, "Value 8")) == objs[8] );
  CHECK_THAT ( rbtree_node_data (rbtree_find (rbtree, "Value 9")) == objs[9] );
  CHECK_THAT ( rbtree_node_data (rbtree_find (rbtree, "Value 10")) == objs[10] );
  CHECK_THAT ( rbtree_node_data (rbtree_find (rbtree, "Value 11")) == objs[11] );
  CHECK_THAT ( rbtree_node_data (rbtree_find (rbtree, "Value 12")) == objs[12] );
  CHECK_THAT ( rbtree_node_data (rbtree_find (rbtree, "Value 13")) == objs[13] );
  CHECK_THAT ( rbtree_node_data (rbtree_find (rbtree, "Value 14")) == objs[14] );
  CHECK_THAT ( rbtree_node_data (rbtree_find (rbtree, "Value 15")) == objs[15] );
  CHECK_THAT ( rbtree_node_data (rbtree_find (rbtree, "Value 17")) == objs[17] );
  CHECK_THAT ( rbtree_node_data (rbtree_find (rbtree, "Value 18")) == objs[18] );
  CHECK_THAT ( rbtree_node_data (rbtree_find (rbtree, "Value 19")) == objs[19] );
  
  rbtree_check_sanity (rbtree);
  rbtree_insert (rbtree, objs[3]);
  rbtree_insert (rbtree, objs[2]);
  rbtree_insert (rbtree, objs[16]);
  rbtree_insert (rbtree, objs[0]);
  rbtree_insert (rbtree, objs[4]);
  rbtree_insert (rbtree, objs[1]);

  rbtree_check_sanity (rbtree);

  CHECK_THAT ( rbtree_node_data (rbtree_find (rbtree, "Value 0")) == objs[0] );
  CHECK_THAT ( rbtree_node_data (rbtree_find (rbtree, "Value 1")) == objs[1] );
  CHECK_THAT ( rbtree_node_data (rbtree_find (rbtree, "Value 2")) == objs[2] );
  CHECK_THAT ( rbtree_node_data (rbtree_find (rbtree, "Value 14")) == objs[14] );
  CHECK_THAT ( rbtree_node_data (rbtree_find (rbtree, "Value 15")) == objs[15] );
  CHECK_THAT ( rbtree_node_data (rbtree_find (rbtree, "Value 16")) == objs[16] );
  CHECK_THAT ( rbtree_node_data (rbtree_find (rbtree, "Value 17")) == objs[17] );
  CHECK_THAT ( rbtree_node_data (rbtree_find (rbtree, "Value 18")) == objs[18] );

  for (i = 0; i < FUNCTIONALITY_NUM_CHECK; ++i)
    functionality_iteration[i] = 0;

  rbtree_check_functionality_iterationUserDataOK = 1;
  rbtree_walk (rbtree, (void *)0x12345678, rbtree_check_functionality_iterationFunction);

  CHECK_THAT (rbtree_check_functionality_iterationUserDataOK);

  for (i = 0; i < FUNCTIONALITY_NUM_CHECK; ++i)
    CHECK_THAT (functionality_iteration[i] == 1);

  rbtree_insert (rbtree, objs[0]);  /* added twice */

  CHECK_THAT ( rbtree_node_data (rbtree_find (rbtree, "Value 0")) == objs[0] );
  rbtree_node_delete (rbtree_find (rbtree, "Value 0"));
  CHECK_THAT ( rbtree_node_data (rbtree_find (rbtree, "Value 0")) == objs[0] );
  rbtree_node_delete (rbtree_find (rbtree, "Value 0"));
  CHECK_THAT ( rbtree_find (rbtree, "Value 0") == NULL );

  rbtree_check_sanity (rbtree);

  for (i = 0; i < FUNCTIONALITY_NUM_CHECK; ++i)
    functionality_destruction[i] = 0;

  rbtree_destroy (rbtree, rbtree_check_functionality_destructorFunction);

  CHECK_THAT (functionality_destruction[0] == 0);
  CHECK_THAT (functionality_destruction[1] == 1);
  CHECK_THAT (functionality_destruction[2] == 1);
  CHECK_THAT (functionality_destruction[14] == 1);
  CHECK_THAT (functionality_destruction[15] == 1);
  CHECK_THAT (functionality_destruction[16] == 1);
  CHECK_THAT (functionality_destruction[17] == 1);
  CHECK_THAT (functionality_destruction[18] == 1);
  CHECK_THAT (functionality_destruction[19] == 1);

  rbtree_check_functionality_destructorFunction (objs[0]);

  return 0;
}

static int
rbtree_check_iterators (void)
{
  struct rbtree *rbtree; 
  struct rbtree_check_Functionality *objs[FUNCTIONALITY_NUM_CHECK];
  int i;

  checkModule = "iterators";

  rbtree = new_rbtree (rbtree_check_functionality_keyFunction,
                       rbtree_check_functionality_comparisonFunction);

  CHECK_THAT ( rbtree != NULL );
  
  for (i=0; i<ITERATORS_NUM_CHECK; ++i)
    {
      objs[i] = malloc (sizeof (struct rbtree_check_Functionality));
      objs[i]->checkCode = i;
      objs[i]->rbtreeValue = malloc (40);
      snprintf (objs[i]->rbtreeValue, 39, "Value %09d", i);
      objs[i]->rbtreeValue[39] ='\0';
      rbtree_insert (rbtree, objs[i]);
    }

  /* check forwards */

  struct rbtree_node *node = rbtree_head(rbtree);
  for (i=0; i<ITERATORS_NUM_CHECK; ++i)
    {
      struct rbtree_check_Functionality *obj = rbtree_node_data (node);
      CHECK_THAT ( obj == objs[i] );
      node = rbtree_node_next(node);
    }
  CHECK_THAT ( node == NULL );

  /* check backwards */

  node = rbtree_tail(rbtree);
  for (i=ITERATORS_NUM_CHECK-1; i>=0; --i)
    {
      struct rbtree_check_Functionality *obj = rbtree_node_data (node);
      CHECK_THAT ( obj == objs[i] );
      node = rbtree_node_prev(node);
    }
  CHECK_THAT ( node == NULL );

  return 0;
}

int
main (int argc, char **argv)
{
  int failed = 0;
  checkName = "rbtree";
  failed = rbtree_check_functionality () ? 1 : failed;
  failed = rbtree_check_iterators ()     ? 1 : failed;
  return failed;
}

/* arch-tag: 359717a3-fcac-431d-bdac-89d79489e1c0
 */
