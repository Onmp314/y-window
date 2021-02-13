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

#ifndef Y_CPP_CANVAS_H
#define Y_CPP_CANVAS_H

#include <Y/c++/connection.h>
#include <Y/c++/widget.h>
#include <stdint.h>
#include <sigc++/sigc++.h>
#include <vector>

namespace Y
{
  /** \brief 2D bitmap canvas
   * \ingroup remote
   *
   * A canvas is a 2D bitmap object that supports server-side drawing primitives
   */
  class Canvas : public Widget
  {
  public:
    /** \brief Line primitive
     *
     * A trivial representation of a line using a coordinate and a
     * vector. This is used to simplify the interface to
     * Y::Canvas::drawLines.
     */
    class Line
    {
    public:
      Line(uint32_t x_, uint32_t y_, int32_t dx_, int32_t dy_) : x(x_), y(y_), dx(dx_), dy(dy_)
      {
      }
      uint32_t x, y;
      int32_t dx, dy;
    };
    typedef std::vector<Line> Lines;

    Canvas (Y::Connection *y);
    virtual ~Canvas ();

    void savePainterState ();
    void restorePainterState ();
    void setBlendMode ();
    void setPenColour (uint32_t);
    void setFillColour (uint32_t);
    void reset (uint32_t &newWidth, uint32_t &newHeight);
    void clearRectangle (uint32_t x, uint32_t y, uint32_t w, uint32_t h);
    void drawRectangle (uint32_t x, uint32_t y, uint32_t w, uint32_t h);
    void drawHLine (uint32_t x, uint32_t y, uint32_t dx);
    void drawVLine (uint32_t x, uint32_t y, uint32_t dy);
    void drawLine (uint32_t x, uint32_t y, int32_t dx, int32_t dy);
    void drawLine (Line l) {drawLine(l.x, l.y, l.dx, l.dy);}
    void drawLines (Lines lines);
    void setBufferSize (uint32_t w, uint32_t h);
    void swapBuffers ();
    void requestSize (uint32_t w, uint32_t h);

    /** Background colour
     */
    Object::Property<uint32_t> background;
    /** Signalled when the size of the canvas changes
     */
    SigC::Signal0<void> resize;

  protected:
    virtual bool onEvent (const std::string &, const Y::Message::Members&);
  };
}

#endif

/* arch-tag: 6a8c0c24-4328-43de-94b6-fc2053533128
 */
