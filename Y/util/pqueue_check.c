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

#include <Y/util/pqueue.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

const char *checkName;
const char *checkModule;

#define FUNCTIONALITY_NUM_CHECK 100

struct pqueue_check_Functionality
{
  long int priority;
  int checkCode;
};

static int
pqueue_check_functionality_comparisonFunction (const void *obj1_v, const void *obj2_v)
{
  const struct pqueue_check_Functionality *obj1 = obj1_v;
  const struct pqueue_check_Functionality *obj2 = obj2_v;
  return obj1 -> priority - obj2 -> priority;
}

static char pqueue_check_functionality_usage[FUNCTIONALITY_NUM_CHECK];

static void
pqueue_check_functionality_destructorFunction (void *obj_v)
{
  struct pqueue_check_Functionality *obj = obj_v;
  pqueue_check_functionality_usage[obj -> checkCode]++;
  free (obj);
}

static int
pqueue_check_functionality (void)
{
  struct PQueue *pqueue;
  struct pqueue_check_Functionality *objs[FUNCTIONALITY_NUM_CHECK];
  struct pqueue_check_Functionality *obj;
  int i;

  checkModule = "functionality";

  srandom (time (NULL));

  pqueue = pqueueCreate (pqueue_check_functionality_comparisonFunction);

  CHECK_THAT ( pqueue != NULL );

  for (i=0; i<FUNCTIONALITY_NUM_CHECK; ++i)
    {
      objs[i] = malloc (sizeof (struct pqueue_check_Functionality));
      objs[i] -> priority = random ();
      objs[i] -> checkCode = i;
      pqueue_check_functionality_usage[i] = 0;
      pqueueInsert (pqueue, objs[i]);
    }

  i = 0;
  while ((obj = pqueueGetNext (pqueue)) != NULL)
    {
      CHECK_THAT ( i <= obj -> priority );
      pqueue_check_functionality_usage[obj -> checkCode]++;
      i = obj -> priority;
    } 

  for (i=0; i<FUNCTIONALITY_NUM_CHECK; ++i)
    {
      CHECK_THAT ( pqueue_check_functionality_usage[i] == 1 );
    }
  
  for (i=FUNCTIONALITY_NUM_CHECK-1; i>=0; --i)
    {
      pqueueInsert (pqueue, objs[i]);
    }

  pqueueDestroy (pqueue, pqueue_check_functionality_destructorFunction);

  for (i=0; i<FUNCTIONALITY_NUM_CHECK; ++i)
    {
      CHECK_THAT ( pqueue_check_functionality_usage[i] == 2 );
    }

  return 0;
    
}


int
main (int argc, char **argv)
{
  int failed = 0;
  checkName = "PQueue";
  failed = pqueue_check_functionality () ? 1 : failed;
  return failed;
}

/* arch-tag: 03de760f-7201-47d9-ab18-298fc1ac2dba
 */
