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

#include <Y/c++/menu.h>
#include <Y/c++/connection.h>

#include <string>

Y::Menu::Menu (Y::Connection *y) : Widget(y, "Menu")
{
  subscribeSignal ("clicked"); 
}

void
Y::Menu::addItem (int id, const std::string &text, Menu *submenu)
{
  invokeMethod("addItem", id, text, (submenu == NULL ? 0 : submenu -> id()));
  if (submenu != NULL)
    submenu->parent = this;
}

Y::Menu::~Menu ()
{
}


/* arch-tag: 84cb372b-761b-42c5-b138-1f09519d6061
 */
