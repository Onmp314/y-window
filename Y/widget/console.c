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

#include <Y/widget/console.h>
#include <Y/widget/widget_p.h>

#include <Y/util/yutil.h>
#include <Y/buffer/painter.h>

#include <Y/object/class_p.h>
#include <Y/object/object_p.h>

#include <Y/text/font.h>

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

/*
 * This whole file, and the console/terminal system in general, needs
 * a serious overhaul.  It was written a bit hastily.  Volunteers?
 */

enum ConsoleCharFlags
{
  CCF_BOLD       = 1<<0,
  CCF_BLINK      = 1<<1,
  CCF_INVERSE    = 1<<2,
  CCF_UNDERLINE  = 1<<3
};

struct ConsoleChar
{
  wchar_t character;
  int8_t foreground;
  int8_t background;
  uint8_t flags;
  uint8_t charset;
};

static struct ConsoleChar consolecharEmpty = { L'\0', -1, -1, 0, 'A' };

struct Console
{
  struct Widget widget;
  uint32_t cols, rows;
  uint32_t cursorCol, cursorRow;
  struct ConsoleChar *contents;
  uint32_t colours[16];
  int foreground, background, bold, blink, inverse, underline;
  int defaultForeground, defaultBackground;
  int charWidth, charHeight, charBase;
  char charset; 
};

static void consoleResize (struct Widget *);
static void consoleReconfigure (struct Widget *);
static void consolePaint (struct Widget *, struct Painter *);
static int consoleKeyboardRaw (struct Widget *, enum YKeyCode, bool, uint32_t);

DEFINE_CLASS(Console);
#include "Console.yc"

/* SUPER
 * Widget
 */

static struct WidgetTable consoleTable =
{
  resize:        consoleResize,
  reconfigure:   consoleReconfigure,
  paint:         consolePaint,
  keyboardRaw:   consoleKeyboardRaw,
};

static inline struct Console *
castBack (struct Widget *widget)
{
  /* assert ( widget -> c == consoleClass ); */
  return (struct Console *)widget;
}

static inline const struct Console *
castBackConst (const struct Widget *widget)
{
  /* assert ( widget -> c == consoleClass ); */
  return (const struct Console *)widget;
}

static void
consoleResizeContents (struct Console *self, uint32_t newCols, uint32_t newRows)
{
  struct ConsoleChar *newData, *dst, *src;
  if (newCols == self -> cols && newRows == self -> rows)
    return;

  newData = ymalloc (sizeof (struct ConsoleChar) * newCols * newRows);

  dst = newData;
  for (uint32_t j = 0; j<newRows; ++j)
    {
      src = self -> contents + self -> cols * j;
      for (uint32_t i = 0; i<newCols; ++i)
        {
          if (j < self -> rows && i < self -> cols)
            memcpy (dst, src, sizeof (struct ConsoleChar));
          else
            memcpy (dst, &consolecharEmpty, sizeof (struct ConsoleChar));
          ++src;
          ++dst;
        }
    }

  yfree (self -> contents);
  self -> contents = newData;
  self -> rows = newRows;
  self -> cols = newCols;
  widgetRepaint (consoleToWidget (self), NULL);
}

static void
consoleRepaintChars (struct Console *self, uint32_t sCol, uint32_t sRow,
                     uint32_t eCol, uint32_t eRow)
{
  struct Rectangle *dirty;
  dirty = rectangleCreate (sCol * self -> charWidth, sRow * self -> charHeight,
                           (eCol - sCol) * self -> charWidth,
                           (eRow - sRow) * self -> charHeight);
  widgetRepaint (consoleToWidget (self), dirty);
}

void
consoleDrawText (struct Console *self, uint32_t col, uint32_t row, wchar_t *string,
                 uint32_t width)
{
  uint32_t i = col, j = row;
  wchar_t *cs = string;
  struct ConsoleChar *cur = self -> contents + self -> cols * j + i;
  uint32_t start = i;

  if (i >= self -> cols || j >= self -> rows)
    return;

  while (*cs != L'\0')
    {
      cur -> character = *cs;
      cur -> foreground = self -> foreground;
      cur -> background = self -> background;
      cur -> flags = ((self -> bold)      ? CCF_BOLD      : 0)
                   | ((self -> blink)     ? CCF_BLINK     : 0)
                   | ((self -> inverse)   ? CCF_INVERSE   : 0)
                   | ((self -> underline) ? CCF_UNDERLINE : 0);
      cur -> charset = self -> charset;
      ++i;
      ++cur;
      if (i >= self -> cols)
        {
          consoleRepaintChars (self, start, j, self -> cols, j + 1);
          i = 0;
          start = 0;
          ++j;
          cur = self -> contents + self -> rows * j + i;
          if (j >= self -> rows)
            return; /* scroll? */
        }
      ++cs;
    }
  consoleRepaintChars (self, start, j, i, j + 1);
}

/* METHOD
 * clearRect :: (uint32, uint32, uint32, uint32) -> ()
 */
void
consoleClearRect (struct Console *self, uint32_t sCol, uint32_t sRow, uint32_t eCol, uint32_t eRow)
{
  if (sCol >= self -> cols)
    return;
  if (sRow >= self -> rows)
    return;
  if (eCol > self -> cols)
    eCol = self -> cols;
  if (eRow > self -> rows)
    eRow = self -> rows;
  for (uint32_t j = sRow; j < eRow; ++j)
    {
      struct ConsoleChar *cur = self -> contents + self -> cols * j + sCol;
      for (uint32_t i = sCol; i < eCol; ++i)
        {
          cur -> character = L'\0';
          cur -> foreground = -1;
          cur -> background = -1;
          cur -> flags = 0;
          cur -> charset = self -> charset; 
          ++cur;
        }
    }
  consoleRepaintChars (self, sCol, sRow, eCol, eRow);
}

static void
consolePaint (struct Widget *self_w, struct Painter *painter)
{
  struct Console *self = castBack (self_w);
  struct ConsoleChar *cur;
  struct Font *font = fontCreate ("Bitstream Vera Sans Mono", "Roman", 11);
  struct Rectangle *painterClip = painterGetClipRectangle (painter);

  uint32_t left = painterClip -> x / self -> charWidth;
  uint32_t right = (painterClip -> x + painterClip -> w + self -> charWidth - 1)
                 / self -> charWidth;
  uint32_t top = painterClip -> y / self -> charHeight; 
  uint32_t bottom = (painterClip -> y + painterClip -> h + self -> charHeight - 1)
                  / self -> charHeight;

  rectangleDestroy (painterClip);

  if (right > self -> cols)
    right = self -> cols;

  if (bottom > self -> rows)
    bottom = self -> rows;

  for (uint32_t j = top; j < bottom; ++j)
    {
      cur = self -> contents + self -> cols * j + left;
      for (uint32_t i = left; i < right; ++i)
        {
          int8_t background = cur -> background;
          int8_t foreground = cur -> foreground;
          if (background < 0)
            background = self -> defaultBackground;
          if (foreground < 0)
             foreground = self -> defaultForeground;  
          if ((cur -> flags & CCF_INVERSE) != 0)
            {
              int8_t tmp = background;
              background = foreground;
              foreground = tmp;
            } 
          if (cur -> flags & CCF_BOLD)
            foreground += 8;
          if (cur -> flags & CCF_BLINK)
            background += 8;
          painterSetPenColour  (painter, self -> colours[background % 16]);
          painterSetFillColour (painter, self -> colours[background % 16]);
          painterDrawRectangle (painter, self -> charWidth * i,
                                self -> charHeight * j,
                                self -> charWidth, self -> charHeight);
          if (cur -> character != L'\0')
            {
              wchar_t str[2] = { cur -> character, L'\0' };
              painterSetPenColour (painter, self -> colours[foreground % 16]);
              fontRenderWCString (font, painter, str,
                                  self -> charWidth * i,
                                  self -> charHeight * j + self -> charBase);
            }
          ++cur;
        }
    } 

  if (self -> cols * self -> charWidth < (uint32_t)self -> widget.w)
    {
      painterSetPenColour (painter, self -> colours[self -> defaultBackground]);
      painterSetFillColour (painter, self -> colours[self -> defaultBackground]);
      painterDrawRectangle (painter, self -> cols * self -> charWidth, 0,
                            self -> widget.w - self -> cols * self -> charWidth,
                            self -> widget.h);
    }

  if (self -> rows * self -> charHeight < (uint32_t)self -> widget.h)
    {
      painterSetPenColour (painter, self -> colours[self -> defaultBackground]);
      painterSetFillColour (painter, self -> colours[self -> defaultBackground]);
      painterDrawRectangle (painter, 0, self -> rows * self -> charHeight,
                            self -> widget.w,
                            self -> widget.h - self -> rows * self -> charHeight);
    }

  fontDestroy (font);
}

int
consoleKeyboardRaw (struct Widget *self_w, enum YKeyCode key,
                    bool pressed, uint32_t modifierState)
{
  struct Console *self = castBack (self_w);
  if (pressed)
    objectEmitSignal (consoleToObject (self), "keyPress", tb_uint32(key), tb_uint32(modifierState));
  return 1;
}

static uint32_t defaultColours[] =
{
  0xBB000000, 0xFFCC0000, 0xFF00CC00, 0xFFCCCC00,
  0xFF4444CC, 0xFFCC00CC, 0xFF00CCCC, 0xFFCCCCCC,
  0xFF666666, 0xFFFF6666, 0xFF66FF66, 0xFFFFFF66,
  0xFF6666FF, 0xFFFF66FF, 0xFF66FFFF, 0xFFFFFFFF
};

static struct Console *
consoleCreate (void)
{
  struct Console *self = ymalloc (sizeof (struct Console));
  objectInitialise (&(self -> widget.o), CLASS(Console));
  widgetInitialise (&(self -> widget), &consoleTable);
  self -> cols = 0;
  self -> rows = 0;
  self -> cursorCol = 0;
  self -> cursorRow = 0;
  self -> contents = NULL;
  memcpy (self -> colours, defaultColours, sizeof (defaultColours));
  self -> defaultForeground = 7;
  self -> defaultBackground = 0;
  self -> foreground = -1;
  self -> background = -1;
  self -> bold = 0;
  self -> blink = 0;
  self -> inverse = 0;
  self -> underline = 0;
  self -> charset = 'A';
  /* FIXME: this really should be derived from the font */
  self -> charWidth = 7;
  self -> charHeight = 12;
  self -> charBase = 9;
  consoleResizeContents (self, 80, 24);
  return self;
}

/* METHOD
 * DESTROY :: () -> ()
 */
static void
consoleDestroy (struct Console *self)
{
  widgetFinalise (consoleToWidget (self));
  objectFinalise (consoleToObject (self));
  yfree (self);
}

struct Widget *
consoleToWidget (struct Console *self)
{
  return &(self -> widget);
}

struct Object *
consoleToObject (struct Console *self)
{
  return &(self -> widget.o);
}

/* METHOD
 * Console :: () -> (object)
 */
struct Object *
consoleInstantiate (void)
{
  return consoleToObject (consoleCreate ());
}

static void
consoleResize (struct Widget *self_w)
{
  struct Console *self = castBack (self_w);
  uint32_t newCols = self -> widget.w / self -> charWidth;
  uint32_t newRows = self -> widget.h / self -> charHeight;
  if (newCols < 1)
    newCols = 1;
  if (newRows < 1)
    newRows = 1;
  if (newCols != self->cols || newRows != self->rows)
    {
      consoleResizeContents (self, newCols, newRows);
      objectEmitSignal (consoleToObject (self), "resize", tb_uint32(self->cols), tb_uint32(self->rows));
    }
}

static void
consoleReconfigure (struct Widget *self_w)
{
  struct Console *self = castBack (self_w);

  self -> widget.reqWidth  = self -> charWidth * 80;
  self -> widget.reqHeight = self -> charHeight * 24;
  self -> widget.minWidth  = self -> charWidth;
  self -> widget.minHeight = self -> charHeight;

  widgetReconfigure (self -> widget.container);
}

/* METHOD
 * drawText :: (uint32, uint32, string, uint32) -> ()
 */
static void
consoleCDrawText (struct Console *self, uint32_t col, uint32_t row, uint32_t length, const char *string, uint32_t width)
{
  wchar_t wstring[2 * (length + 1)];
  mbstowcs (wstring, string, 2 * (length + 1));

  consoleDrawText (self, col, row, wstring, width);
}

void
consoleSetRendition (struct Console *self, int bold, int blink, int inverse, int underline, int foreground, int background, const char *charset)
{
  self -> bold = bold;
  self -> blink = blink;
  self -> inverse = inverse;
  self -> underline = underline;
  if (foreground >= 1 && foreground <= 8)
    self -> foreground = foreground - 1;
  else
    self -> foreground = -1;
  if (background >= 1 && background <= 8)
    self -> background = background - 1;
  else
  self -> background = -1;
  self -> charset = charset[0];
}

/* METHOD
 * setRendition :: (uint32, uint32, uint32, uint32, uint32, uint32, string) -> ()
 */
static void
consoleCSetRendition (struct Console *self,
                      uint32_t bold, uint32_t blink, uint32_t inverse, uint32_t underline,
                      uint32_t foreground, uint32_t background,
                      uint32_t charset_len, const char *charset)
{
  consoleSetRendition (self, bold, blink, inverse, underline, foreground, background, charset);
}

/* METHOD
 * swapVideo :: () -> ()
 */
void
consoleSwapVideo (struct Console *self)
{
  Y_TRACE ("Swap Video ?");
}

/* METHOD
 * ring :: () -> ()
 */
void
consoleRing (struct Console *self)
{
  Y_TRACE ("***BEEP***");
}

/* METHOD
 * updateCursorPos :: (uint32, uint32) -> ()
 */
void
consoleUpdateCursorPos (struct Console *self, uint32_t col, uint32_t row)
{
  if (col != self -> cursorCol || row != self -> cursorRow)
    {
      consoleRepaintChars (self, self -> cursorCol, self -> cursorRow,
                           self -> cursorCol + 1, self -> cursorRow +1);
      self -> cursorCol = col;
      self -> cursorRow = row;
      consoleRepaintChars (self, self -> cursorCol, self -> cursorRow,
                           self -> cursorCol + 1, self -> cursorRow +1);
    }
}

/* METHOD
 * scrollView :: (uint32, uint32, uint32) -> ()
 */
void
consoleScrollView (struct Console *self, uint32_t destRow, uint32_t srcRow, uint32_t numLines)
{
  if (destRow >= self -> rows)
    return;
  if (srcRow >= self -> rows)
    return;
  if (destRow + numLines > self -> rows)
    numLines = self -> rows - destRow;
  if (srcRow + numLines > self -> rows)
    numLines = self -> rows - srcRow;
  memmove (self -> contents + destRow * self -> cols,
           self -> contents + srcRow * self -> cols,
           numLines * self -> cols * sizeof (struct ConsoleChar)); 
  if (destRow < srcRow)
    consoleClearRect (self, 0, MAX (srcRow, destRow + numLines),
                      self -> cols, srcRow + numLines);
  else if (srcRow < destRow)
    consoleClearRect (self, 0, srcRow, self -> cols,
                      MIN (numLines, destRow - srcRow));
  consoleRepaintChars (self, 0, destRow, self -> cols, destRow + numLines);
}

/* arch-tag: 0b2da405-d961-435e-afae-1cf8f01f8310
 */
