/************************************************************************
 *   Copyright (C) Mark Thomas <markbt@efaref.net>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <Y/c++/checkbox.h>
#include <Y/c++/connection.h>

#include <string>

Y::CheckBox::CheckBox (Y::Connection *y) : Widget(y, "CheckBox"),
                                           text(this, "text"), checked(this, "checked")
{
  subscribeSignal ("clicked");
}

Y::CheckBox::CheckBox (Y::Connection *y, std::string t) : Widget(y, "CheckBox"),
                                                          text(this, "text"), checked(this, "checked")
{
  subscribeSignal ("clicked");
  text.set(t);
}

Y::CheckBox::~CheckBox ()
{
}

bool
Y::CheckBox::onEvent (const std::string &name, const Y::Message::Members& params)
{
  if (name == "clicked" && params.size () >= 1)
    {
      clicked (params[1].uint32());
      return true;
    }
  return false;
} 


/* arch-tag: 93e2077b-068d-4cd9-a63f-1eb1d012b8cc
 */
