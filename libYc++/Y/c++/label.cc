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

#include <Y/c++/label.h>
#include <Y/c++/connection.h>

#include <string>

Y::Label::Label (Y::Connection *y) : Widget(y, "Label"),
                                     text(this, "text"), alignment(this, "alignment")
{
}

Y::Label::Label (Y::Connection *y, std::string t) : Widget(y, "Label"),
                                                    text(this, "text"), alignment(this, "alignment")
{
  text.set(t);
}

Y::Label::~Label ()
{
}

bool
Y::Label::onEvent (const std::string &name, const Y::Message::Members& params)
{
  return false;
}

/* arch-tag: 05fde3ef-746f-49d3-8473-f33a66ad17e5
 */
