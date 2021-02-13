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

#include <Y/c++/canvas.h>
#include <Y/c++/connection.h>

#include <string>

Y::Canvas::Canvas (Y::Connection *y) : Widget(y, "Canvas"), background(this, "background")
{
  subscribeSignal ("resize");
}

Y::Canvas::~Canvas ()
{
}

bool
Y::Canvas::onEvent (const std::string &name, const Y::Message::Members& params)
{
  if (name == "resize")
    {
      resize ();
      return true;
    }
  return false;
}

void
Y::Canvas::savePainterState ()
{
}

void
Y::Canvas::restorePainterState ()
{
}

void
Y::Canvas::setBlendMode ()
{
}

void
Y::Canvas::setPenColour (uint32_t colour)
{
  invokeMethod ("setPenColour", colour, false);
}

void
Y::Canvas::setFillColour (uint32_t colour)
{
  invokeMethod ("setFillColour", colour, false);
}

void
Y::Canvas::reset (uint32_t &newWidth, uint32_t &newHeight)
{
  Reply *rep = invokeMethod ("reset", true);
  Y::Message::Members res = rep->tuple();
  delete rep;
  if (res.size () >= 2)
    {
      newWidth  = res[0].uint32();
      newHeight = res[1].uint32();
    }
}

void
Y::Canvas::clearRectangle (uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
}

void
Y::Canvas::drawRectangle (uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
}

void
Y::Canvas::drawHLine (uint32_t x, uint32_t y, uint32_t dx)
{
  invokeMethod ("drawHLines", x, y, dx, false);
}

void
Y::Canvas::drawVLine (uint32_t x, uint32_t y, uint32_t dy)
{
  invokeMethod ("drawVLines", x, y, dy, false);
}

void
Y::Canvas::drawLine (uint32_t x, uint32_t y, int32_t dx, int32_t dy)
{
  /* Not a line */
  if (dx == 0 && dy == 0)
    return;
  /* Really a VLine */
  else if (dx == 0)
    {
      if (dy > 0)
        drawVLine (x, y, dy);
      else
        drawVLine (x, y + dy, -dy);
      return;
    }
  /* Really a HLine */
  else if (dy == 0)
    {
      if (dx > 0)
        drawHLine (x, y, dx);
      else
        drawHLine (x + dx, y, -dx);
      return;
    }
  else
    {
      invokeMethod ("drawLines", x, y, dx, dy, false);
      return;
    }
}

void
Y::Canvas::drawLines (Y::Canvas::Lines lines)
{
  uint32_t x, y;
  int32_t dx, dy;
  Y::Message::Members v;

  v.push_back("drawLines");

  for (Y::Canvas::Lines::iterator i = lines.begin(); i != lines.end(); i++)
    {
       v.push_back(i->x);
       v.push_back(i->y);
       v.push_back(i->dx);
       v.push_back(i->dy);
    }
  invokeMethod (v, false);
}

void
Y::Canvas::setBufferSize (uint32_t w, uint32_t h)
{
  invokeMethod ("setBufferSize", w, h, false);
}

void
Y::Canvas::swapBuffers ()
{
  invokeMethod ("swapBuffers", false);
}

void
Y::Canvas::requestSize (uint32_t w, uint32_t h)
{
  invokeMethod ("requestSize", w, h, false);
}

/* arch-tag: 68c6c4a6-b5a4-40b1-9ba1-fdf272b6e55a
 */
