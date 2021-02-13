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

#ifndef Y_CPP_CHECKBOX_H
#define Y_CPP_CHECKBOX_H

#include <Y/c++/connection.h>
#include <Y/c++/widget.h>

namespace Y
{
  /** \brief Check box
   * \ingroup remote
   */
  class CheckBox : public Widget
  {
  public:
    CheckBox (Y::Connection *y);
    CheckBox (Y::Connection *y, std::string t);
    virtual ~CheckBox ();

    /** This string is rendered next to the check box
     */
    Object::Property<std::string> text;
    /** Zero for unchecked, one for checked
     */
    Object::Property<uint32_t> checked;
    /** Signalled when the check box is clicked, and its state has
     * changed.
     *
     * The boolean argument is the new state of the check box.
     */
    SigC::Signal1<void,bool> clicked;

  protected:
    virtual bool onEvent (const std::string &, const Y::Message::Members&);
  };
}

#endif

/* arch-tag: 414513ee-39a0-4ed5-b85e-98a44f6a7980
 */
