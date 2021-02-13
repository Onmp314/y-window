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

#ifndef Y_CPP_BUTTON_H
#define Y_CPP_BUTTON_H

#include <Y/c++/connection.h>
#include <Y/c++/widget.h>

namespace Y
{
  /** \brief Push button
   * \ingroup remote
   */
  class Button : public Widget
  {
  public:
    Button (Y::Connection *y);
    Button (Y::Connection *y, std::string t);
    virtual ~Button ();

    /** This string is rendered onto the button
     */
    Object::Property<std::string> text;
    /** Signalled when the button is fully clicked (depressed and
     * released)
     */
    SigC::Signal0<void> clicked;

  protected:
    virtual bool onEvent (const std::string &, const Y::Message::Members&);
  };
}

#endif

/* arch-tag: 48ac1bf4-65cd-4abe-9913-062868ea70cd
 */
