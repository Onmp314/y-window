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

#define CHECK_STOP abort()
#include <Y/util/check.h>

#include <Y/util/index.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

const char *checkName;
const char *checkModule;

#define FUNCTIONALITY_NUM_CHECK 20
#define ITERATORS_NUM_CHECK 20

struct index_check_Functionality
{
  int checkCode;
  char *indexValue;
};

static int
index_check_functionality_keyFunction (const void *key_v, const void *obj_v)
{
  const char *key = key_v;
  const struct index_check_Functionality *obj = obj_v;
  return strcmp (key, obj->indexValue);
}

static int functionality_destruction[FUNCTIONALITY_NUM_CHECK];
static int functionality_iteration[FUNCTIONALITY_NUM_CHECK];

static void
index_check_functionality_destructorFunction (void *obj_v)
{
  struct index_check_Functionality *obj = obj_v;
  ++functionality_destruction[obj->checkCode];
  free (obj->indexValue);
  free (obj);
}

static int
index_check_functionality_comparisonFunction (const void *obj1_v, const void *obj2_v)
{
  const struct index_check_Functionality *obj1 = obj1_v;
  const struct index_check_Functionality *obj2 = obj2_v;
  return strcmp (obj1->indexValue, obj2->indexValue);
} 

static int index_check_functionality_iterationUserDataOK;

static void
index_check_functionality_iterationFunction (const void *obj_v, void *userData)
{
  const struct index_check_Functionality *obj = obj_v;
  if (userData != (void *)0x12345678)
    index_check_functionality_iterationUserDataOK = 0;
  functionality_iteration[obj->checkCode]++;
}

static int
index_check_functionality (void)
{
  struct Index *index; 
  struct index_check_Functionality *objs[FUNCTIONALITY_NUM_CHECK];
  int i;

  checkModule = "functionality";

  index = indexCreate (index_check_functionality_keyFunction,
                       index_check_functionality_comparisonFunction);

  CHECK_THAT ( index != NULL );

  for (i=0; i<FUNCTIONALITY_NUM_CHECK; ++i)
    {
      objs[i] = malloc (sizeof (struct index_check_Functionality));
      objs[i]->checkCode = i;
      objs[i]->indexValue = malloc (40);
      snprintf (objs[i]->indexValue, 39, "Value %d", i);
      objs[i]->indexValue[39] ='\0';
      indexAdd (index, objs[i]);
    }

  CHECK_THAT ( indexVerifyConstraints (index) );

  CHECK_THAT ( indexFind (index, "Value 4") == objs[4] );
  CHECK_THAT ( indexFind (index, "Something Random") == NULL );
  CHECK_THAT ( indexRemove (index, "Value 16") == objs[16] );
  CHECK_THAT ( indexRemove (index, "Value 16") == NULL );
  CHECK_THAT ( indexFind (index, "Value 14") == objs[14] );
  CHECK_THAT ( indexFind (index, "Value 15") == objs[15] );
  CHECK_THAT ( indexFind (index, "Value 16") == NULL );
  CHECK_THAT ( indexFind (index, "Value 17") == objs[17] );
  CHECK_THAT ( indexFind (index, "Value 18") == objs[18] );
  CHECK_THAT ( indexRemove (index, "Value 0") == objs[0] );
  CHECK_THAT ( indexFind (index, "Value 0") == NULL );
  CHECK_THAT ( indexFind (index, "Value 1") == objs[1] );
  CHECK_THAT ( indexRemove (index, "Value 1") == objs[1] );
  CHECK_THAT ( indexRemove (index, "Value 2") == objs[2] );
  CHECK_THAT ( indexRemove (index, "Value 3") == objs[3] );
  CHECK_THAT ( indexRemove (index, "Value 4") == objs[4] );

  CHECK_THAT ( indexFind (index, "Value 5") == objs[5] );
  CHECK_THAT ( indexFind (index, "Value 19") == objs[19] );
  
  CHECK_THAT ( indexVerifyConstraints (index) );
  indexAdd (index, objs[3]);
  indexAdd (index, objs[2]);
  indexAdd (index, objs[16]);
  indexAdd (index, objs[0]);
  indexAdd (index, objs[4]);
  indexAdd (index, objs[1]);

  CHECK_THAT ( indexVerifyConstraints (index) );
  CHECK_THAT ( indexFind (index, "Value 0") == objs[0] );
  CHECK_THAT ( indexFind (index, "Value 1") == objs[1] );
  CHECK_THAT ( indexFind (index, "Value 2") == objs[2] );
  CHECK_THAT ( indexFind (index, "Value 14") == objs[14] );
  CHECK_THAT ( indexFind (index, "Value 15") == objs[15] );
  CHECK_THAT ( indexFind (index, "Value 16") == objs[16] );
  CHECK_THAT ( indexFind (index, "Value 17") == objs[17] );
  CHECK_THAT ( indexFind (index, "Value 18") == objs[18] );

  for (i = 0; i < FUNCTIONALITY_NUM_CHECK; ++i)
    functionality_iteration[i] = 0;

  index_check_functionality_iterationUserDataOK = 1;
  indexIterate (index, (void *)0x12345678, index_check_functionality_iterationFunction);

  CHECK_THAT (index_check_functionality_iterationUserDataOK);

  for (i = 0; i < FUNCTIONALITY_NUM_CHECK; ++i)
    CHECK_THAT (functionality_iteration[i] == 1);

  indexAdd (index, objs[0]);  /* added twice */

  CHECK_THAT ( indexFind (index, "Value 0") == objs[0] );
  CHECK_THAT ( indexRemove (index, "Value 0") == objs[0] );
  CHECK_THAT ( indexFind (index, "Value 0") == NULL );

  CHECK_THAT ( indexVerifyConstraints (index) );

  for (i = 0; i < FUNCTIONALITY_NUM_CHECK; ++i)
    functionality_destruction[i] = 0;

  indexDestroy (index, index_check_functionality_destructorFunction);

  CHECK_THAT (functionality_destruction[0] == 0);
  CHECK_THAT (functionality_destruction[1] == 1);
  CHECK_THAT (functionality_destruction[2] == 1);
  CHECK_THAT (functionality_destruction[14] == 1);
  CHECK_THAT (functionality_destruction[15] == 1);
  CHECK_THAT (functionality_destruction[16] == 1);
  CHECK_THAT (functionality_destruction[17] == 1);
  CHECK_THAT (functionality_destruction[18] == 1);
  CHECK_THAT (functionality_destruction[19] == 1);

  index_check_functionality_destructorFunction (objs[0]);

  return 0;
}

static int
index_check_iterators (void)
{
  struct Index *index; 
  struct index_check_Functionality *objs[FUNCTIONALITY_NUM_CHECK];
  struct IndexIterator *iter;
  int i;

  checkModule = "iterators";

  index = indexCreate (index_check_functionality_keyFunction,
                       index_check_functionality_comparisonFunction);

  CHECK_THAT ( index != NULL );
  
  for (i=0; i<ITERATORS_NUM_CHECK; ++i)
    {
      objs[i] = malloc (sizeof (struct index_check_Functionality));
      objs[i]->checkCode = i;
      objs[i]->indexValue = malloc (40);
      snprintf (objs[i]->indexValue, 39, "Value %09d", i);
      objs[i]->indexValue[39] ='\0';
      indexAdd (index, objs[i]);
    }

  /* check forwards */

  iter = indexGetStartIterator (index);
  for (i=0; i<ITERATORS_NUM_CHECK; ++i)
    {
      struct index_check_Functionality *obj = indexiteratorGet (iter);
      CHECK_THAT ( obj == objs[i] );
      indexiteratorNext (iter);
    }
  CHECK_THAT ( indexiteratorGet (iter) == NULL );
  indexiteratorDestroy (iter);

  /* check backwards */

  iter = indexGetEndIterator (index);
  for (i=ITERATORS_NUM_CHECK-1; i>=0; --i)
    {
      struct index_check_Functionality *obj = indexiteratorGet (iter);
      CHECK_THAT ( obj == objs[i] );
      indexiteratorPrevious (iter);
    }
  CHECK_THAT ( indexiteratorGet (iter) == NULL );

  /* go forwards again */
  indexiteratorNext (iter);
  for (i=0; i<ITERATORS_NUM_CHECK; ++i)
    {
      struct index_check_Functionality *obj;
      CHECK_THAT ( indexiteratorHasValue (iter) );
      obj = indexiteratorGet (iter);
      CHECK_THAT ( obj == objs[i] );
      indexiteratorNext (iter);
    }
  CHECK_THAT ( indexiteratorHasValue (iter) == 0 );
  CHECK_THAT ( indexiteratorGet (iter) == NULL );
  indexiteratorDestroy (iter);

  return 0;
}

int
main (int argc, char **argv)
{
  int failed = 0;
  checkName = "Index";
  failed = index_check_functionality () ? 1 : failed;
  failed = index_check_iterators ()     ? 1 : failed;
  return failed;
}

/* arch-tag: 419f8d07-add8-40a4-89fd-39fa75406d37
 */
