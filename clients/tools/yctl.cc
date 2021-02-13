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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <Y/c++.h>

#include <string>
#include <vector>

int
main (int argc, char **argv)
{
  if (argc <= 1)
    return EXIT_SUCCESS;

  Y::Connection y;

  Y::Class *c = y.findClass(argv[1]);

  Y::Message::Members v;

  for (int i = 2; i < argc; ++i)
    v.push_back(argv[i]);

  Y::Reply *rep = c->invokeMethod(v, true);

  Y::Message::Members params = rep->tuple();
  for (Y::Message::Members::iterator i = params.begin(); i < params.end(); i++)
    std::cout << *i << std::endl;

  delete rep;

  return EXIT_SUCCESS;
}

/* arch-tag: 854a2dbd-cc0e-4a49-9ae9-9031e4ca590c
 */
