/************************************************************************
 *   Copyright (C) Mark Thomas <markbt@efaref.net>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include "clock.h"

#include <iostream>

#include <time.h>
#include <sys/time.h>

#define COS30    (0.8660254)
#define SIN30    (0.5)

using std::cout;
using std::endl;
using SigC::slot;
using SigC::bind;

Clock::Clock (Y::Connection *y_) : y(y_), ticker(y, 1000)
{
  width = 64;
  height = 64;

  window = new Y::Window (y, "Clock");

  canvas = new Y::Canvas (y);
  canvas -> requestSize (128, 128);

  window -> setChild (canvas);

  window -> requestClose.connect (bind (slot (&exit), EXIT_SUCCESS));

  drawClock ();

  canvas -> resize.connect (slot (*this, &Clock::drawClock));

  // canvas -> setProperty ("background", 0xFFC0C0C0);

  ticker.tick.connect (slot (*this, &Clock::tick));

  window -> show ();
}

Clock::~Clock ()
{
}

void
Clock::tick ()
{
  drawClock ();
}

void
Clock::drawClock ()
{
  canvas -> reset (width, height);

  int r = (width < height) ? width/2 : height/2;
  int cx = width / 2;
  int cy = height / 2;
  int t = r / 8;
  if (t < 2)
    t = 2;
  int s = r - t;
  int ssin30 = (int)(s * SIN30);
  int scos30 = (int)(s * COS30);
  int tsin30 = (int)(t * SIN30);
  int tcos30 = (int)(t * COS30); 

  /* clock face */
  canvas -> setPenColour (0xFF000000);
  Y::Canvas::Lines face;
  /* 12 posn */
  face.push_back(Y::Canvas::Line (cx + 1, cy - s, 0, -t));
  face.push_back (Y::Canvas::Line (cx + 1, cy - r, t/2, 0));
  face.push_back (Y::Canvas::Line (cx + t/2 + 1, cy - r, -t/2, t));
  face.push_back (Y::Canvas::Line (cx - 1, cy - s, 0, -t));
  face.push_back (Y::Canvas::Line (cx - 1, cy - r, -t/2, 0));
  face.push_back (Y::Canvas::Line (cx - t/2 - 1, cy - r, t/2, t));
  /* 1 posn */
  face.push_back (Y::Canvas::Line (cx + ssin30, cy - scos30, tsin30, -tcos30));
  /* 2 posn */
  face.push_back (Y::Canvas::Line (cx + scos30, cy - ssin30, tcos30, -tsin30));
  /* 3 posn */
  face.push_back (Y::Canvas::Line (cx + s, cy, t, 0));
  /* 4 posn */
  face.push_back (Y::Canvas::Line (cx + scos30, cy + ssin30, tcos30, tsin30));
  /* 5 posn */
  face.push_back (Y::Canvas::Line (cx + ssin30, cy + scos30, tsin30, tcos30));
  /* 6 posn */
  face.push_back (Y::Canvas::Line (cx, cy + s, 0, t));
  /* 7 posn */
  face.push_back (Y::Canvas::Line (cx - ssin30, cy + scos30, -tsin30, tcos30));
  /* 8 posn */
  face.push_back (Y::Canvas::Line (cx - scos30, cy + ssin30, -tcos30, tsin30));
  /* 9 posn */
  face.push_back (Y::Canvas::Line (cx - s, cy, -t, 0));
  /* 10 posn */
  face.push_back (Y::Canvas::Line (cx - scos30, cy - ssin30, -tcos30, -tsin30));
  /* 11 posn */
  face.push_back (Y::Canvas::Line (cx - ssin30, cy - scos30, -tsin30, -tcos30));
  canvas->drawLines(face);

  time_t now = std::time (NULL);
  struct std::tm *currentTime = std::localtime (&now);
  
  double scAng = currentTime -> tm_sec * M_PI / 30.0;
  double mnAng = currentTime -> tm_min * M_PI / 30.0 + scAng / 60.0;
  double hrAng = currentTime -> tm_hour * M_PI / 6.0 + mnAng / 12.0;

  Y::Canvas::Lines hands;
  canvas -> setPenColour (0xFF0000FF);
  hands.push_back (Y::Canvas::Line (cx, cy, (int) (r * 0.5 * sin (hrAng)),
                                    (int) (-r * 0.5 * cos (hrAng))));
  hands.push_back (Y::Canvas::Line (cx, cy, (int) (r * 0.85 * sin (mnAng)),
                                    (int) (-r * 0.85 * cos (mnAng))));
  canvas -> drawLines (hands);
  canvas -> setPenColour (0xCCFF0000);
  canvas -> drawLine (cx, cy, (int) (r * 0.95 * sin (scAng)),
                              (int) (-r * 0.95 * cos (scAng)));

  canvas -> swapBuffers ();
}

int
main (int argc, char **argv)
{
  Y::Connection y;
  Clock *c = new Clock (&y);
  y.run();
}

/* arch-tag: 68a9c23d-bb8b-40d4-8547-b920642a1b4e
 */
