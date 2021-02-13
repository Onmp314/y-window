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

#include <Y/c++/window.h>
#include <Y/c++/connection.h>

#include <string>

Y::Window::Window (Y::Connection *y) : Widget(y, "Window"),
                                       title(this, "title"), background(this, "background")
{
  subscribeSignal ("requestClose");
}

Y::Window::Window (Y::Connection *y, std::string t) : Widget(y, "Window"),
                                                      title(this, "title"), background(this, "background")
{
  subscribeSignal ("requestClose");
  title.set(t);
}

Y::Window::~Window ()
{
}

void
Y::Window::show ()
{
  invokeMethod ("show", false); 
}

void
Y::Window::setChild (Widget *w)
{
  if (w != NULL)
    {
      invokeMethod ("setChild", w->id(), false);
      w->parent = this;
    }
}

void
Y::Window::setFocussed (Widget *w)
{
  if (w != NULL)
    {
      invokeMethod ("setFocussed", w->id(), false);
    }
}

bool
Y::Window::onEvent (const std::string &name, const Y::Message::Members& params)
{
  if (name == "requestClose")
    {
      requestClose ();
      return true;
    }
  return false;
}

/* arch-tag: 4952e419-7783-4f6c-86cc-cf90f1a077b5
 */
