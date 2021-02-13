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

#ifndef Y_CPP_LABEL_H
#define Y_CPP_LABEL_H

#include <Y/c++/connection.h>
#include <Y/c++/widget.h>

#include <string>

namespace Y
{
  /** \brief Text label
   * \ingroup remote
   */
  class Label : public Widget
  {
  public:
    Label (Y::Connection *y);
    Label (Y::Connection *y, std::string t);
    virtual ~Label ();

    /** Rendered on the label */
    Object::Property<std::string> text;
    /** Alignment of the rendered text */
    Object::Property<std::string> alignment;

  protected:
    virtual bool onEvent (const std::string &name, const Y::Message::Members& params);
  };
}

#endif

/* arch-tag: a908a672-c134-4fd4-b3fb-e94a20a55e73
 */
