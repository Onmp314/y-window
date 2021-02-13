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

#ifndef Y_CPP_MENU_H
#define Y_CPP_MENU_H

#include <Y/c++/connection.h>
#include <Y/c++/widget.h>

namespace Y
{
  /** \brief Menu
   * \ingroup remote
   */
  class Menu : public Widget
  {
  public:
    Menu (Y::Connection *y);
    virtual ~Menu ();

    void addItem (int id, const std::string &text, Menu *submenu = NULL);

    SigC::Signal1<void,int> clicked;
  };
}

#endif

/* arch-tag: fb6a3469-6802-4929-972d-15ec3a1e851b
 */
