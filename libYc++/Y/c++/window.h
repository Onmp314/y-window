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

#ifndef Y_CPP_WINDOW_H
#define Y_CPP_WINDOW_H

#include <Y/c++/connection.h>
#include <Y/c++/widget.h>

#include <string>

namespace Y
{
  /** \brief %Window
   * \ingroup remote
   */
  class Window : public Widget
  {
  public:
    Window (Y::Connection *y);
    Window (Y::Connection *y, std::string t);
    virtual ~Window ();

    void show ();
    void setChild (Widget *);
    void setFocussed (Widget *);

    /** Window title */
    Object::Property<std::string> title;
    /** Background colour */
    Object::Property<uint32_t> background;
    /** Signalled when the user attempts to close the window. The
     * window is not closed automatically; the client must do that if
     * appropriate, by handling this signal.
     */
    SigC::Signal0<void> requestClose;

  protected:
    virtual bool onEvent (const std::string &, const Y::Message::Members&);
  };
}

#endif

/* arch-tag: 86645ee9-343e-4880-9c1e-0d10fd93b192
 */
