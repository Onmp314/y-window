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

#include <Y/c++.h>
#include "sample.h"

#include <iostream>

using namespace SigC;
using namespace std;

Sample::Sample (Y::Connection *y_) : y(y_)
{
  window = new Y::Window (y, "Sample Application");

  grid = new Y::GridLayout (y);
  window -> setChild (grid);

  window -> requestClose.connect (bind (slot (&exit), EXIT_SUCCESS));

  samplelabel = new Y::Label (y, "The Y Window System");
  samplelabel->alignment.set("center");
  grid -> addWidget (samplelabel, 0, 0, 7, 1);

  samplebutton = new Y::Button (y, "Sample Button");
  grid -> addWidget (samplebutton, 0, 1, 3);

  samplecheckbox = new Y::CheckBox (y, "Sample Check Box");
  grid -> addWidget (samplecheckbox, 4, 1, 3);

  window -> show ();
}

Sample::~Sample ()
{
}


int
main (int argc, char **argv)
{
  Y::Connection y;
  Sample s(&y);
  y.run();
}

/* arch-tag: e5a4fbe8-beb9-4c65-8e48-28489a967a50
 */
