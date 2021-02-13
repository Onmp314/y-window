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

#include <Y/util/rectangle.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

const char *checkName;
const char *checkModule;

static int
rectangle_check_functionality (void)
{
  struct Rectangle *r1, *r2, *r3;

  checkModule = "functionality";

  r1 = rectangleCreate (400, 0, 800, 200);

  CHECK_THAT ( r1 != NULL );
  CHECK_THAT ( r1->x == 400 );
  CHECK_THAT ( r1->y == 0 );
  CHECK_THAT ( r1->w == 800 );
  CHECK_THAT ( r1->h == 200 );

  r2 = rectangleDuplicate (r1);

  CHECK_THAT ( r2 != NULL );
  CHECK_THAT ( r2->x == 400 );
  CHECK_THAT ( r2->y == 0 );
  CHECK_THAT ( r2->w == 800 );
  CHECK_THAT ( r2->h == 200 );

  r1->x = 0;

  CHECK_THAT ( r1->x == 0 );
  CHECK_THAT ( r2->x == 400 );

  r3 = rectangleCreate (0, 0, 0, 0);

  CHECK_THAT ( r3 != NULL );

  /* Union two disparate rectangles */
  r1->x = 0;
  r1->y = 0;
  r1->w = 10;
  r1->h = 10;
  r2->x = 20;
  r2->y = 20;
  r2->w = 10;
  r2->h = 10;

  CHECK_THAT ( rectangleUnion (r3, r1, r2) == 0 );
  CHECK_THAT ( r3->x == 0 );
  CHECK_THAT ( r3->y == 0 );
  CHECK_THAT ( r3->w == 30 );
  CHECK_THAT ( r3->h == 30 );

  CHECK_THAT ( rectangleUnion (r3, r2, r1) == 0 );
  CHECK_THAT ( r3->x == 0 );
  CHECK_THAT ( r3->y == 0 );
  CHECK_THAT ( r3->w == 30 );
  CHECK_THAT ( r3->h == 30 );

  /* Union two intersecting rectangles */  
  r1->x = 0;
  r1->y = 0;
  r1->w = 30;
  r1->h = 30;
  r2->x = 10;
  r2->y = 10;
  r2->w = 30;
  r2->h = 30;

  CHECK_THAT ( rectangleUnion (r3, r1, r2) == 1 );
  CHECK_THAT ( r3->x == 0 );
  CHECK_THAT ( r3->y == 0 );
  CHECK_THAT ( r3->w == 40 );
  CHECK_THAT ( r3->h == 40 );

  CHECK_THAT ( rectangleUnion (r3, r2, r1) == 1 );
  CHECK_THAT ( r3->x == 0 );
  CHECK_THAT ( r3->y == 0 );
  CHECK_THAT ( r3->w == 40 );
  CHECK_THAT ( r3->h == 40 );

  /* Intersect two disparate rectangles */
  r1->x = 0;
  r1->y = 0;
  r1->w = 10;
  r1->h = 10;
  r2->x = 20;
  r2->y = 20;
  r2->w = 10;
  r2->h = 10;

  CHECK_THAT ( rectangleIntersect (r3, r1, r2) == 0 );
  CHECK_THAT ( rectangleIntersect (r3, r2, r1) == 0 );

  /* Union two intersecting rectangles */  
  r1->x = 0;
  r1->y = 0;
  r1->w = 30;
  r1->h = 30;
  r2->x = 20;
  r2->y = 20;
  r2->w = 30;
  r2->h = 30;

  CHECK_THAT ( rectangleIntersect (r3, r1, r2) == 1 );
  CHECK_THAT ( r3->x == 20 );
  CHECK_THAT ( r3->y == 20 );
  CHECK_THAT ( r3->w == 10 );
  CHECK_THAT ( r3->h == 10 );

  CHECK_THAT ( rectangleIntersect (r3, r2, r1) == 1 );
  CHECK_THAT ( r3->x == 20 );
  CHECK_THAT ( r3->y == 20 );
  CHECK_THAT ( r3->w == 10 );
  CHECK_THAT ( r3->h == 10 );

  rectangleDestroy (r1);
  rectangleDestroy (r2);
  rectangleDestroy (r3);

  return 0;
}

int
main (int argc, char **argv)
{
  int failed = 0;
  checkName = "Rectangle";
  failed = rectangle_check_functionality () ? 1 : failed;
  return failed;
}

/* arch-tag: 04c58d6c-64f5-4748-a48a-147483660e2f
 */
