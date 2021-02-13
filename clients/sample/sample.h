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

#ifndef YSAMPLE_SAMPLE_H
#define YSAMPLE_SAMPLE_H

#include <Y/c++.h>
#include <sigc++/sigc++.h>

using namespace SigC;
using namespace std;

class Sample : public SigC::Object
{
 private:
  Y::Window *window;
  Y::GridLayout *grid;
  Y::Label *samplelabel;
  Y::Button *samplebutton;
  Y::CheckBox *samplecheckbox;

  Y::Connection *y;

 public:
  Sample (Y::Connection *y);
  virtual ~Sample ();

};

#endif

/* arch-tag: ca7ea1d1-4b75-4314-8bde-fff02f405952
 */
