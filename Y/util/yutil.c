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

#include <Y/util/yutil.h>
#include <Y/util/log.h>
#include <stdio.h>
#include <string.h>

void *
ymalloc (size_t n)
{
  int *p = malloc (n+4);
  if (p == NULL)
    {
      Y_FATAL ("Y: out of memory\n");
      abort ();
    }
  *p++ = 0x1ea7beef;
  return (void *)p;  
}

void
yfree (void *p)
{
  int *pi = p;
  if (p == NULL)
    return;
  --pi;
  if (*pi != 0x1ea7beef)
    {
      Y_FATAL ("yfree'd something that wasn't ymalloc'd? %p\n", p);
      abort ();
      free (p);
    }
  else
    {
      *pi = 0xdeadf001;
      free (pi);
    }
}

char *
ystrdup (const char *s)
{
  if (!s)
    return NULL;
  int len = s ? strlen (s) : 0;
  char * result = ymalloc (len + 1);
  strncpy (result, s, len);
  result[len] = '\0';
  return result;
}

/* arch-tag: 8d07f3f6-d59a-49a7-aac4-e0b0733b113a
 */
