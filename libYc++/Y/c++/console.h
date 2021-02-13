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

#ifndef Y_CPP_CONSOLE_H
#define Y_CPP_CONSOLE_H

#include <Y/c++/connection.h>
#include <Y/c++/widget.h>
#include <stdint.h>
#include <sigc++/sigc++.h>

#include <Y/const.h>

namespace Y
{
  /** \brief Text console
   * \ingroup remote
   *
   * A console is a 2D text region with support for scrolling and keyboard input
   */
  class Console : public Widget
  {
    public:
      Console (Y::Connection *y);
      virtual ~Console ();

      void drawText (uint32_t col, uint32_t row, const std::string &mbstring, uint32_t width);
      void clearRect (uint32_t sCol, uint32_t sRow, uint32_t eCol, uint32_t eRow);
      void setRendition (uint32_t bold, uint32_t blink, uint32_t inverse, uint32_t underline,
                         uint32_t foreground, uint32_t background, char charset);
      void swapVideo ();
      void ring ();
      void updateCursorPos (uint32_t col, uint32_t row);
      void scrollView (uint32_t destRow, uint32_t srcRow, uint32_t numLine);

      /** Signalled when a keystroke is received. The first argument
       * is the keycode, and the second argument is the current
       * keyboard modifier state.
       */
      SigC::Signal2<void, enum YKeyCode, uint32_t> keyPress;
      /** Signalled when the console is resized. The first argument is
       * the new count of columns, and the second argument is the new
       * count of rows.
       */
      SigC::Signal2<void, uint32_t, uint32_t> resize;

    protected:
      virtual bool onEvent (const std::string &, const Y::Message::Members&);
  };
}

#endif

/* arch-tag: 9fe8ff4e-fe53-487d-859b-a49549236937
 */
