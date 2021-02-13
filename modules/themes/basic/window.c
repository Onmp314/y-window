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

#include <Y/widget/window.h>
#include <Y/widget/widget.h>
#include <Y/object/object.h>
#include <Y/screen/screen.h>
#include <Y/text/font.h>
#include <Y/modules/windowmanager.h>
#include <Y/modules/theme.h>

#include "draw.h"
#include "window.h"

static int32_t windowICON[] =
{
  0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6080FF80, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
  0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x8080FF80, 0xFF80FF80, 0xFF80FF80, 0xFF80FF80, 0x8080FF80, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
  0x00000000, 0x2080FF80, 0x8080FF80, 0xFF80FF80, 0xFF80FF80, 0xFF80FF80, 0xFF80FF80, 0xFF80FF80, 0xFF80FF80, 0xFF80FF80, 0x8080FF80, 0x2080FF80, 0x00000000,
  0x00000000, 0x5080FF80, 0xFF80FF80, 0xFF80FF80, 0xFF80FF80, 0xFF80FF80, 0xFF80FF80, 0xFF80FF80, 0xFF80FF80, 0xFF80FF80, 0xFF80FF80, 0x5080FF80, 0x00000000,
  0x00000000, 0x00000000, 0x00000000, 0x8080FF80, 0xFF80FF80, 0xFF80FF80, 0xFF80FF80, 0xFF80FF80, 0xFF80FF80, 0x8080FF80, 0x00000000, 0x00000000, 0x00000000,
  0x00000000, 0xFFFF8080, 0x80FF8080, 0x00000000, 0x00000000, 0xC080FF80, 0xFF80FF80, 0xC080FF80, 0x00000000, 0x00000000, 0x808080FF, 0xFF8080FF, 0x00000000,
  0x00000000, 0xFFFF8080, 0xFFFF8080, 0xFFFF8080, 0x80FF8080, 0x00000000, 0x00000000, 0x00000000, 0x808080FF, 0xFF8080FF, 0xFF8080FF, 0xFF8080FF, 0x00000000,
  0x00000000, 0xFFFF8080, 0xFFFF8080, 0xFFFF8080, 0xFFFF8080, 0xC0FF8080, 0x00000000, 0xC08080FF, 0xFF8080FF, 0xFF8080FF, 0xFF8080FF, 0xFF8080FF, 0x00000000,
  0x00000000, 0xFFFF8080, 0xFFFF8080, 0xFFFF8080, 0xFFFF8080, 0xFFFF8080, 0x00000000, 0xFF8080FF, 0xFF8080FF, 0xFF8080FF, 0xFF8080FF, 0xFF8080FF, 0x00000000,
  0x00000000, 0xFFFF8080, 0xFFFF8080, 0xFFFF8080, 0xFFFF8080, 0xFFFF8080, 0x00000000, 0xFF8080FF, 0xFF8080FF, 0xFF8080FF, 0xFF8080FF, 0xFF8080FF, 0x00000000,
  0x00000000, 0x20FF8080, 0xC0FF8080, 0xFFFF8080, 0xFFFF8080, 0xFFFF8080, 0x00000000, 0xFF8080FF, 0xFF8080FF, 0xFF8080FF, 0xC08080FF, 0x208080FF, 0x00000000,
  0x00000000, 0x00000000, 0x00000000, 0x40FF8080, 0xC0FF8080, 0xFFFF8080, 0x00000000, 0xFF8080FF, 0xC08080FF, 0x408080FF, 0x00000000, 0x00000000, 0x00000000,
  0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x20FF8080, 0x00000000, 0x208080FF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
};

enum WindowRegion
{
WINDOW_REGION_NOTHING,
WINDOW_REGION_MOVE,
WINDOW_REGION_MAXIMISE, WINDOW_REGION_RESTORE, WINDOW_REGION_CLOSE,
WINDOW_REGION_RESIZE_NW, WINDOW_REGION_RESIZE_N, WINDOW_REGION_RESIZE_NE,
WINDOW_REGION_RESIZE_W,                          WINDOW_REGION_RESIZE_E,
WINDOW_REGION_RESIZE_SW, WINDOW_REGION_RESIZE_S, WINDOW_REGION_RESIZE_SE,
WINDOW_REGION_CHILD
};

void
basicWindowInit (struct Window *window)
{
}

static void
paintTitleBar (struct Window *window, struct Painter *painter,
               int x, int y, int w)
{
  /* title bar */
  const struct Value *titleValue = objectGetProperty (windowToObject (window), "title");
  const char *title = titleValue ? titleValue->string.data : "";
  struct Font *font = fontCreate ("Bitstream Vera Sans", "Bold", 12);
  int ascender, i, foreground;
  int titlex = x + 22;
  int titley = y + 3;
  uint32_t titleColours[2][2] = {{0xFF505050, 0xFFAAAAAA},
                                 {0xFF5050C0, 0xFFAAAADD}};

  fontGetMetrics (font, &ascender, NULL, NULL);
  titley += ascender;
  painterSetFillColour (painter, 0xFF505050);
  if (wmSelectedWindow () == window)
    foreground = 1;
  else
    foreground = 0;

  for(i = x; i < w + x; i++)
    {
      uint32_t blendColour = 0xFF000000;
      colourBlendSourceOver (&blendColour, titleColours[foreground][0],
        titleColours[foreground][1], (i - x) * 0xFF / w);
      painterSetPenColour (painter, blendColour);
      painterDrawVLine (painter, i, x, 19);
    }

  painterSetBlendMode (painter, COLOUR_BLEND_SOURCE_OVER);
  painterDrawRGBAData (painter, windowICON, x + 4, x + 3, 13, 13, 13);
  painterSaveState (painter);
  {
    struct Rectangle clipRect = { x + 18, y, w - 47, 19 };
    painterClipTo (painter, &clipRect);
  }
  painterSetPenColour  (painter, 0x60000000);
  fontRenderString (font, painter, title, titlex, titley);
  painterSetPenColour  (painter, 0x30000000);
  fontRenderString (font, painter, title, titlex-1, titley);
  fontRenderString (font, painter, title, titlex+1, titley);
  fontRenderString (font, painter, title, titlex, titley-1);
  fontRenderString (font, painter, title, titlex, titley+1);
  painterSetPenColour  (painter, 0xFFFFFFFF);
  fontRenderString (font, painter, title, titlex-1, titley-1);
  painterRestoreState (painter);
  fontDestroy (font);

  painterSetPenColour (painter, 0xFFFFFFFF);
  painterDrawLine (painter, x + w - 12, y + 5,  7,  7);
  painterDrawLine (painter, x + w -  6, y + 5, -7,  7);
  if (windowGetSizeState (window) == WINDOW_SIZE_NORMAL)
    {
      painterDrawLine (painter, x + w - 22, y + 5, -4,  7);
      painterDrawLine (painter, x + w - 22, y + 5,  4,  7);
    }
  else
    {
      painterDrawLine (painter, x + w - 22, y + 12, -4, -7);
      painterDrawLine (painter, x + w - 22, y + 12,  4, -7);
    }

}

void
basicWindowPaint (struct Window *window, struct Painter *painter)
{
  struct Rectangle *rect = widgetGetRectangle (windowToWidget (window));
  struct Widget *child = windowGetChild (window);
  bool selected = (wmSelectedWindow () == window);
  uint32_t bgcolour = 0x00C0C0C0 | (selected ? 0xFF000000 : 0xC0000000 );
  const struct Value *bgcolourProperty = objectGetProperty (windowToObject (window), "background");
  uint32_t setbgcolour = bgcolourProperty ? bgcolourProperty->uint32 : bgcolour;

  if (windowGetSizeState (window) == WINDOW_SIZE_NORMAL)
    {
      /* resizing border */
      painterSetBlendMode   (painter, COLOUR_BLEND_SOURCE);
      painterSetFillColour  (painter, bgcolour);
      painterClearRectangle (painter, 0, 0, rect -> w, 5);
      painterClearRectangle (painter, rect -> w - 5, 0, 5, rect -> h); 
      painterClearRectangle (painter, 0, rect -> h - 5, rect -> w, 5);
      painterClearRectangle (painter, 0, 0, 5, rect -> h);
      painterSetPenColour   (painter, 0xFF000000);
      painterDrawHLine      (painter, 0, 0, rect -> w);
      painterDrawHLine      (painter, 0, rect -> h - 1, rect -> w);
      painterDrawVLine      (painter, 0, 0, rect -> h);
      painterDrawVLine      (painter, rect -> w - 1, 0, rect -> h);

      /* content area */
      painterSetFillColour  (painter, setbgcolour);
      painterClearRectangle (painter, 5, 23, rect -> w - 10, rect -> h - 28);

      paintTitleBar (window, painter, 5, 5, rect -> w - 10);
    }
  else
    {
      painterSetFillColour  (painter, setbgcolour);
      painterClearRectangle (painter, 0, 20, rect -> w, rect -> h - 20);
      paintTitleBar (window, painter, 0, 0, rect -> w);
    }

  rectangleDestroy (rect);

  /* paint child */
  if (child != NULL)
    {
      struct Rectangle *childRect = widgetGetRectangle (child);
      painterSaveState (painter);
      painterEnter (painter, childRect);
      if (!painterFullyClipped (painter))
        {
          widgetPaint (child, painter);
        }
      painterRestoreState (painter);
      rectangleDestroy (childRect);
    }
}

int
basicWindowGetRegion (struct Window *window, int32_t x, int32_t y)
{
  struct Rectangle *rect = widgetGetRectangle (windowToWidget (window));
  int region = WINDOW_REGION_NOTHING;
  int32_t wx = 0, wy = 0, ww = rect -> w, wh = rect -> h;
  switch (windowGetSizeState (window))
    {
      case WINDOW_SIZE_NORMAL:
        wx = 5;
        wy = 5;
        ww = rect -> w - 10;
        wh = rect -> h - 10;
        break;
      case WINDOW_SIZE_MAXIMISE:
        wx = 0;
        wy = 0;
        ww = rect -> w;
        wh = rect -> h;
    }

  if (y > wy && y <= wy + 19)
    {
      if (x > wx + wx + 19 && x < wx + ww - 27)
        region = WINDOW_REGION_MOVE;
      if (x >= wx + ww - 27 && x < wx + ww - 14)
        {
          if (windowGetSizeState (window) == WINDOW_SIZE_NORMAL)
            region = WINDOW_REGION_MAXIMISE;
          else
            region = WINDOW_REGION_RESTORE;
        }
      if (x >= wx + ww - 14 && x <= wx + ww)
        region = WINDOW_REGION_CLOSE;
    }
  if (x > wx && x < wx + ww && y > wy + 19 && y < wy + wh)
    region = WINDOW_REGION_CHILD;

  if (windowGetSizeState (window) == WINDOW_SIZE_NORMAL)
    {
      if (y <= 5)
        {
          if (x <= 24)
            region = WINDOW_REGION_RESIZE_NW;
          if (x > 24 && x <= rect -> w - 24)
            region = WINDOW_REGION_RESIZE_N;
          if (x > rect -> w - 24 && x <= rect -> w)
            region = WINDOW_REGION_RESIZE_NE;
        }
      if (x <= 5)
        {
          if (y <= 24)
            region = WINDOW_REGION_RESIZE_NW;
          if (y > 24 && y <= rect -> h - 24)
            region = WINDOW_REGION_RESIZE_W;
          if (y > rect -> h - 24 && y <= rect -> h)
            region = WINDOW_REGION_RESIZE_SW;
        }
      if (y >= rect -> h - 5 && y <= rect -> h)
        {
          if (x <= 24)
            region = WINDOW_REGION_RESIZE_SW;
          if (x > 24 && x <= rect -> w - 24)
            region = WINDOW_REGION_RESIZE_S;
          if (x > rect -> w - 24 && x <= rect -> w)
            region = WINDOW_REGION_RESIZE_SE;
        }
      if (x >= rect -> w - 5 && x <= rect -> w)
        {
          if (y <= 24)
            region = WINDOW_REGION_RESIZE_NE;
          if (y > 24 && y <= rect -> h - 24)
            region = WINDOW_REGION_RESIZE_E;
          if (y > rect -> h - 24 && y <= rect -> h)
            region = WINDOW_REGION_RESIZE_SE;
        }
    }  
  rectangleDestroy (rect);
  return region;
}

int
basicWindowPointerMotion (struct Window *w, int32_t x, int32_t y, int32_t dx, int32_t dy)
{
  int r = basicWindowGetRegion (w, x, y);
  switch (r)
    {
      case WINDOW_REGION_CHILD:
        return widgetPointerMotion (windowGetChild (w), x - 5, y - 24, dx, dy);
    }
  return 0;
}

int
basicWindowPointerButton (struct Window *w, int32_t x, int32_t y, uint32_t b, bool pressed)
{
  int r = basicWindowGetRegion (w, x, y);
  if (b == 0 && pressed == 1)
    {
      switch (r)
        { 
          case WINDOW_REGION_MOVE:
            windowStartReshape (w, x, y, WINDOW_RESHAPE_MOVE); break;
          case WINDOW_REGION_MAXIMISE:
            wmMaximiseWindow (w);   break;
          case WINDOW_REGION_RESTORE:
            wmRestoreWindow (w);   break;
          case WINDOW_REGION_CLOSE:
            windowRequestClose (w);   break;
          case WINDOW_REGION_RESIZE_NW:
            windowStartReshape (w, x, y, WINDOW_RESHAPE_RESIZE_NW); break;
          case WINDOW_REGION_RESIZE_N:
            windowStartReshape (w, x, y, WINDOW_RESHAPE_RESIZE_N); break;
          case WINDOW_REGION_RESIZE_NE:
            windowStartReshape (w, x, y, WINDOW_RESHAPE_RESIZE_NE); break;
          case WINDOW_REGION_RESIZE_W:
            windowStartReshape (w, x, y, WINDOW_RESHAPE_RESIZE_W); break;
          case WINDOW_REGION_RESIZE_E:
            windowStartReshape (w, x, y, WINDOW_RESHAPE_RESIZE_E); break;
          case WINDOW_REGION_RESIZE_SW:
            windowStartReshape (w, x, y, WINDOW_RESHAPE_RESIZE_SW); break;
          case WINDOW_REGION_RESIZE_S:
            windowStartReshape (w, x, y, WINDOW_RESHAPE_RESIZE_S); break;
          case WINDOW_REGION_RESIZE_SE:
            windowStartReshape (w, x, y, WINDOW_RESHAPE_RESIZE_SE); break;
        }
    }
  if (b == 0 && pressed == 0)
    {
      windowStopReshape (w);
      return 1;
    }
  if (r == WINDOW_REGION_CHILD)
    return widgetPointerButton (windowGetChild (w), x - 5, y - 24, b, pressed);
  return 1;
}

static struct Rectangle *
basicWindowChildRectangle (struct Window *w)
{
  struct Rectangle *wr = widgetGetRectangle (windowToWidget (w));
  struct Rectangle *cr = rectangleCreate (5, 24, wr -> w - 10, wr -> h - 29);
  rectangleDestroy (wr);
  return cr;
}

static void
basicWindowPointerMove (struct Window *window, int32_t x, int32_t y, int32_t dx, int32_t dy)
{
}

void
basicWindowReconfigure (struct Window *window,
                        int32_t *minWidth_p, int32_t *minHeight_p,
                        int32_t *reqWidth_p, int32_t *reqHeight_p,
                        int32_t *maxWidth_p, int32_t *maxHeight_p)
{
  struct Widget *child = windowGetChild (window);
  *minWidth_p  = 64;
  *minHeight_p = 48;
  *maxWidth_p  = -1;
  *maxHeight_p = -1;
 
  if (child != NULL)
    { 
      int32_t childMinWidth, childMinHeight;
      int32_t childReqWidth, childReqHeight;
      int32_t childMaxWidth, childMaxHeight;
      widgetGetConstraints (child, &childMinWidth, &childMinHeight,
                            &childReqWidth, &childReqHeight,
                            &childMaxWidth, &childMaxHeight);
      if (childMinWidth > 54)
        *minWidth_p = childMinWidth + 10;   
      if (childMinHeight > 19)
        *minHeight_p = childMinHeight + 29;   
      if (childMaxWidth > 0)
        *maxWidth_p = childMaxWidth + 10;   
      if (childMaxHeight > 0)
        *maxHeight_p = childMaxHeight + 29;

      if (*reqWidth_p < 0 && childReqWidth > 54)
        *reqWidth_p = childReqWidth + 10;   
      if (*reqHeight_p < 0 && childReqHeight > 19)
        *reqHeight_p = childReqHeight + 29;   
    }

  if (*maxWidth_p > 0)
    {
      if (*maxWidth_p < *minWidth_p)
        *maxWidth_p = *minWidth_p;
      if (*reqWidth_p > *maxWidth_p)
        *reqWidth_p = *maxWidth_p;
    }
  if (*maxHeight_p > 0)
    {
      if (*maxHeight_p < *minHeight_p)
        *maxHeight_p = *minHeight_p;
      if (*reqHeight_p > *maxHeight_p)
        *reqHeight_p = *maxHeight_p;
    }
  if (*reqWidth_p < *minWidth_p)
    *reqWidth_p = *minWidth_p;
  if (*reqHeight_p < *minHeight_p)
    *reqHeight_p = *minHeight_p;
}

void
basicWindowResize (struct Window *window)
{
  struct Widget *child = windowGetChild (window);
  if (child != NULL)
    {
      int w, h;
      widgetGetSize (windowToWidget (window), &w, &h);
      switch (windowGetSizeState (window))
        {
          case WINDOW_SIZE_NORMAL:
            widgetMove (child, 5, 24);
            widgetResize (child, w - 10, h - 29);
            break;
          case WINDOW_SIZE_MAXIMISE:
            widgetMove (child, 0, 19);
            widgetResize (child, w, h - 19);
        }  
    }
}

/* arch-tag: ba71cba5-7e3f-408e-97ff-4c11944496fb
 */
