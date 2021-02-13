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

#include <Y/c++/console.h>
#include <Y/c++/connection.h>

#include <string>

Y::Console::Console (Y::Connection *y) : Widget(y, "Console")
{
  subscribeSignal ("keyPress");
  subscribeSignal ("resize");
}

Y::Console::~Console ()
{
}

void
Y::Console::drawText (uint32_t col, uint32_t row, const std::string &mbstring,
                      uint32_t width)
{
  invokeMethod ("drawText", col, row, mbstring, width, false);
}

void
Y::Console::clearRect (uint32_t sCol, uint32_t sRow, uint32_t eCol, uint32_t eRow)
{
  invokeMethod ("clearRect", sCol, sRow, eCol, eRow, false);
}

void
Y::Console::setRendition (uint32_t bold, uint32_t blink, uint32_t inverse, uint32_t underline,
                          uint32_t foreground, uint32_t background, char charset)
{
  char charsetstring[] = { charset, '\0' };
  Y::Message::Members v;
  v.push_back("setRendition");
  v.push_back(bold);
  v.push_back(blink);
  v.push_back(inverse);
  v.push_back(underline);
  v.push_back(foreground);
  v.push_back(background);
  v.push_back(charsetstring);
  invokeMethod (v, false);
}

void
Y::Console::swapVideo ()
{
  invokeMethod ("swapVideo", false);
}

void
Y::Console::ring ()
{
  invokeMethod ("ring", false);
}

void
Y::Console::updateCursorPos (uint32_t col, uint32_t row)
{
  invokeMethod ("updateCursorPos", col, row, false);
}

void
Y::Console::scrollView (uint32_t destRow, uint32_t srcRow, uint32_t numLines)
{
  invokeMethod ("scrollView", destRow, srcRow, numLines, false);
}

bool
Y::Console::onEvent (const std::string &name, const Y::Message::Members& params)
{
  if (name == "keyPress" && params.size () == 2)
    {
      enum YKeyCode keyId =(enum YKeyCode) params[0].uint32();
      uint32_t modState = params[1].uint32();
      keyPress (keyId, modState);
      return true;
    }
  else if (name == "resize" && params.size () == 2)
    {
      int cols = params[0].uint32();
      int rows = params[1].uint32();
      resize (cols, rows);
      return true;
    }
  return false;
}

/* arch-tag: 3c4bea9a-db16-4a53-b40c-317e2ab8a93a
 */
