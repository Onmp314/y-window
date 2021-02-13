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

#include <Y/c++/button.h>
#include <Y/c++/connection.h>

#include <string>

Y::Button::Button (Y::Connection *y) : Widget(y, "Button"), text(this, "text")
{
  subscribeSignal ("clicked");
}

/** \brief Creates a button and sets the text property
 */
Y::Button::Button (Y::Connection *y, std::string t) : Widget(y, "Button"), text(this, "text")
{
  subscribeSignal ("clicked");
  text.set(t);
}

Y::Button::~Button ()
{
}

bool
Y::Button::onEvent (const std::string &name, const Y::Message::Members& params)
{
  if (name == "clicked")
    {
      clicked ();
      return true;
    }
  return false;
}

/* arch-tag: 3f2612a8-50af-4114-a305-a2476fec119b
 */
