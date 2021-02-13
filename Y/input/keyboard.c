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

#include <Y/y.h>
#include <Y/const.h>
#include <Y/main/control.h>
#include <Y/input/keyboard.h>
#include <Y/input/keymap.h>
#include <Y/screen/screen.h>
#include <Y/object/object.h>
#include <Y/widget/widget.h>
#include <Y/util/yutil.h>
#include <Y/modules/windowmanager.h>

#include <stdio.h>
#include <ctype.h>

enum WMKeys
{
  WMKEY_LEFT  = 0x01,
  WMKEY_RIGHT = 0x02,
  WMKEYS      = 0x03
};

enum LogicalModifierCode
{
  LOGICAL_NONE  = 0x0000,
  LOGICAL_LSHIFT= 0x0001,
  LOGICAL_RSHIFT= 0x0002,
  LOGICAL_LCTRL = 0x0040,
  LOGICAL_RCTRL = 0x0080,
  LOGICAL_LALT  = 0x0100,
  LOGICAL_RALT  = 0x0200,
  LOGICAL_LMETA = 0x0400,
  LOGICAL_RMETA = 0x0800,
  LOGICAL_NUM   = 0x1000,
  LOGICAL_CAPS  = 0x2000,
  LOGICAL_MODE  = 0x4000
};

static enum WMKeys wmKeys;
static int keyboardModifierState = 0;
static struct Widget *keyboardFocus = NULL;
static enum YKeyCode currentKey = 0;
static int keyRepeatTimer = 0;

static int
modifiersConvertLogical (int logical)
{
  int mod = 0;
  if ((logical & LOGICAL_LSHIFT) || (logical & LOGICAL_RSHIFT))
    mod |= YMOD_SHIFT;
  if ((logical & LOGICAL_CAPS))
    mod ^= YMOD_SHIFT;
  if ((logical & LOGICAL_LCTRL) || (logical & LOGICAL_RCTRL))
    mod |= YMOD_CTRL;
  if ((logical & LOGICAL_LALT) || (logical & LOGICAL_RALT))
    mod |= YMOD_ALT;
  if ((logical & LOGICAL_LMETA) || (logical & LOGICAL_RMETA))
    mod |= YMOD_META;
  if ((logical & LOGICAL_MODE))
    mod |= YMOD_MODE;
  return mod;
}

void
keyboardSetFocus (struct Widget *focus)
{
  /* struct Widget *oldFocus = keyboardFocus; */
  keyboardFocus = focus;
  if (keyboardFocus != NULL)
    {
      Y_SILENT ("Focusing widget %d",
                objectGetID (widgetToObject (keyboardFocus)));
    }
 /*
  if (oldFocus != keyboardFocus)
    {
      if (oldFocus != NULL)
        widgetFocusLost (oldFocus);
      if (keyboardFocus != NULL)
        widgetFocusGained (keyboardFocus);
    }
 */
}

void
keyboardRemoveFocus (struct Widget *focus)
{
  if (focus == keyboardFocus)
    {
      if (focus != NULL)
        {
          Y_SILENT ("Blurring widget %d",
                    objectGetID (widgetToObject (keyboardFocus)));
        }
     /*
      if (keyboardFocus != NULL)
        widgetFocusLost (keyboardFocus);
      */
      keyboardFocus = NULL;
    }
}

static void
keyboardKeyRepeat (void *data_v)
{
  int modifiers = modifiersConvertLogical (keyboardModifierState);
  enum YKeyCode mappedKey = keymapMapKey (currentKey, modifiers);
  if (wmKeys & WMKEYS)
    wmKeyboard (mappedKey, 1, modifiers);
  else if (keyboardFocus != NULL)
    widgetKeyboardRaw (keyboardFocus, mappedKey, 1, modifiers);
  keyRepeatTimer = controlTimerDelay (0, 50, NULL, keyboardKeyRepeat);
}

static void
keyboardModifierDown (enum YKeyCode key)
{
  switch (key)
    {
      case YK_LSHIFT:   keyboardModifierState |= LOGICAL_LSHIFT;  break; 
      case YK_RSHIFT:   keyboardModifierState |= LOGICAL_RSHIFT;  break; 
      case YK_LCTRL:    keyboardModifierState |= LOGICAL_LCTRL;   break; 
      case YK_RCTRL:    keyboardModifierState |= LOGICAL_RCTRL;   break; 
      case YK_LALT:     keyboardModifierState |= LOGICAL_LALT;    break; 
      case YK_RALT:     keyboardModifierState |= LOGICAL_RALT;    break; 
      case YK_LMETA:    keyboardModifierState |= LOGICAL_LMETA;   break; 
      case YK_RMETA:    keyboardModifierState |= LOGICAL_RMETA;   break; 
      case YK_NUMLOCK:  keyboardModifierState |= LOGICAL_NUM;     break; 
      case YK_CAPSLOCK: keyboardModifierState |= LOGICAL_CAPS;    break; 
      case YK_MODE:     keyboardModifierState |= LOGICAL_MODE;    break;
      case YK_LSUPER:   wmKeys                |= WMKEY_LEFT;      break;
      case YK_RSUPER:   wmKeys                |= WMKEY_RIGHT;     break;
      default: ;
    }
}

static void
keyboardModifierUp (enum YKeyCode key)
{
  switch (key)
    {
      case YK_LSHIFT:   keyboardModifierState &= ~LOGICAL_LSHIFT;  break; 
      case YK_RSHIFT:   keyboardModifierState &= ~LOGICAL_RSHIFT;  break; 
      case YK_LCTRL:    keyboardModifierState &= ~LOGICAL_LCTRL;   break; 
      case YK_RCTRL:    keyboardModifierState &= ~LOGICAL_RCTRL;   break; 
      case YK_LALT:     keyboardModifierState &= ~LOGICAL_LALT;    break; 
      case YK_RALT:     keyboardModifierState &= ~LOGICAL_RALT;    break; 
      case YK_LMETA:    keyboardModifierState &= ~LOGICAL_LMETA;   break; 
      case YK_RMETA:    keyboardModifierState &= ~LOGICAL_RMETA;   break; 
      case YK_NUMLOCK:  keyboardModifierState &= ~LOGICAL_NUM;     break; 
      case YK_CAPSLOCK: keyboardModifierState &= ~LOGICAL_CAPS;    break; 
      case YK_MODE:     keyboardModifierState &= ~LOGICAL_MODE;    break; 
      case YK_LSUPER:   wmKeys                &= ~WMKEY_LEFT;      return;
      case YK_RSUPER:   wmKeys                &= ~WMKEY_RIGHT;     return;
      default: ;
    }
}

void
keyboardKeyDown (enum YKeyCode key)
{
  enum YKeyCode mappedKey;
  int modifiers;
  if (key >= YK_NUMLOCK && key <= YK_MODE)
    {
      keyboardModifierDown (key);
    }
  else
    {
      if (currentKey != key)
        {
          currentKey = key;
          if (keyRepeatTimer != 0)
            controlCancelTimerDelay (keyRepeatTimer);
          keyRepeatTimer = controlTimerDelay (0, 500, NULL, keyboardKeyRepeat);
        }
    }

  modifiers = modifiersConvertLogical (keyboardModifierState);
  mappedKey = keymapMapKey (key, modifiers);

  if (key == YK_KP_MULTIPLY && modifiers & YMOD_CTRL && modifiers & YMOD_ALT)
    {
      /* emergency shutdown */
      controlShutdownY ();
      return;
    }

  if (wmKeys & WMKEYS)
    wmKeyboard (mappedKey, 1, modifiers);
  else if (keyboardFocus != NULL)
    widgetKeyboardRaw (keyboardFocus, mappedKey, 1, modifiers);
}

void
keyboardKeyUp (enum YKeyCode key)
{
  int mappedKey;
  int modifiers;

  if (key >= YK_NUMLOCK && key <= YK_MODE)
    {
      keyboardModifierUp (key);
    }
  else
    {
      if (key == currentKey && keyRepeatTimer != 0)
        {
          controlCancelTimerDelay (keyRepeatTimer);
          keyRepeatTimer = 0;
          currentKey = YK_UNKNOWN;
        }
    }

  modifiers = modifiersConvertLogical (keyboardModifierState);
  mappedKey = keymapMapKey (key, modifiers);

  if (wmKeys & WMKEYS)
    wmKeyboard (mappedKey, 0, modifiers);
  else if (keyboardFocus != NULL)
    widgetKeyboardRaw (keyboardFocus, mappedKey, 0, modifiers);
}

int
keyboardGetModifierState ()
{
  return modifiersConvertLogical (keyboardModifierState);
}

void
keyboardReset ()
{
  if (currentKey != YK_UNKNOWN)
    {
      int modifiers, mappedKey;
      if (keyRepeatTimer != 0)
        {
          controlCancelTimerDelay (keyRepeatTimer);
          keyRepeatTimer = 0;
        }

      modifiers = modifiersConvertLogical (keyboardModifierState);
      mappedKey = keymapMapKey (currentKey, modifiers);

      if (wmKeys & WMKEYS)
        wmKeyboard (mappedKey, 0, modifiers);
      else if (keyboardFocus != NULL)
        widgetKeyboardRaw (keyboardFocus, mappedKey, 0, modifiers);

      wmKeys = 0;
      keyboardModifierState = 0;
      currentKey = YK_UNKNOWN;
    }
}


/* arch-tag: 9e4e45a7-d1eb-4982-99ee-1c2434b8efc3
 */
