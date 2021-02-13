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
#include <Y/modules/module_interface.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <Y/main/control.h>
#include <Y/input/pointer.h>
#include <Y/input/ykb.h>
#include <Y/util/yutil.h>

#define  EVENT_FILE_BASE "/dev/input/event"

struct EvDevInputDriverData
{
  struct input_event cur;
  ssize_t bytes_read;
  struct Module *module;
  int fds[16];
};

static enum YKeyCode keytable[] =
{
  /* KEY_RESERVED      */  YK_UNKNOWN,
  /* KEY_ESC           */  YK_ESCAPE,
  /* KEY_1             */  YK_1,
  /* KEY_2             */  YK_2,
  /* KEY_3             */  YK_3,
  /* KEY_4             */  YK_4,
  /* KEY_5             */  YK_5,
  /* KEY_6             */  YK_6,
  /* KEY_7             */  YK_7,
  /* KEY_8             */  YK_8,
  /* KEY_9             */  YK_9,
  /* KEY_0             */  YK_0,
  /* KEY_MINUS         */  YK_MINUS,
  /* KEY_EQUAL         */  YK_EQUALS,
  /* KEY_BACKSPACE     */  YK_BACKSPACE,
  /* KEY_TAB           */  YK_TAB,
  /* KEY_Q             */  YK_q,
  /* KEY_W             */  YK_w,
  /* KEY_E             */  YK_e,
  /* KEY_R             */  YK_r,
  /* KEY_T             */  YK_t,
  /* KEY_Y             */  YK_y,
  /* KEY_U             */  YK_u,
  /* KEY_I             */  YK_i,
  /* KEY_O             */  YK_o,
  /* KEY_P             */  YK_p,
  /* KEY_LEFTBRACE     */  YK_LEFTBRACE,
  /* KEY_RIGHTBRACE    */  YK_RIGHTBRACE,
  /* KEY_ENTER         */  YK_RETURN,
  /* KEY_LEFTCTRL      */  YK_LCTRL,
  /* KEY_A             */  YK_a,
  /* KEY_S             */  YK_s,
  /* KEY_D             */  YK_d,
  /* KEY_F             */  YK_f,
  /* KEY_G             */  YK_g,
  /* KEY_H             */  YK_h,
  /* KEY_J             */  YK_j,
  /* KEY_K             */  YK_k,
  /* KEY_L             */  YK_l,
  /* KEY_SEMICOLON     */  YK_SEMICOLON,
  /* KEY_APOSTROPHE    */  YK_QUOTE,
  /* KEY_GRAVE         */  YK_BACKQUOTE,
  /* KEY_LEFTSHIFT     */  YK_LSHIFT,
  /* KEY_BACKSLASH     */  YK_BACKSLASH,
  /* KEY_Z             */  YK_z,
  /* KEY_X             */  YK_x,
  /* KEY_C             */  YK_c,
  /* KEY_V             */  YK_v,
  /* KEY_B             */  YK_b,
  /* KEY_N             */  YK_n,
  /* KEY_M             */  YK_m,
  /* KEY_COMMA         */  YK_COMMA,
  /* KEY_DOT           */  YK_PERIOD,
  /* KEY_SLASH         */  YK_SLASH,
  /* KEY_RIGHTSHIFT    */  YK_RSHIFT,
  /* KEY_KPASTERISK    */  YK_KP_MULTIPLY,
  /* KEY_LEFTALT       */  YK_LALT,
  /* KEY_SPACE         */  YK_SPACE,
  /* KEY_CAPSLOCK      */  YK_CAPSLOCK,
  /* KEY_F1            */  YK_F1,
  /* KEY_F2            */  YK_F2,
  /* KEY_F3            */  YK_F3,
  /* KEY_F4            */  YK_F4,
  /* KEY_F5            */  YK_F5,
  /* KEY_F6            */  YK_F6,
  /* KEY_F7            */  YK_F7,
  /* KEY_F8            */  YK_F8,
  /* KEY_F9            */  YK_F9,
  /* KEY_F10           */  YK_F10,
  /* KEY_NUMLOCK       */  YK_NUMLOCK,
  /* KEY_SCROLLLOCK    */  YK_SCROLLLOCK,
  /* KEY_KP7           */  YK_KP7,
  /* KEY_KP8           */  YK_KP8,
  /* KEY_KP9           */  YK_KP9,
  /* KEY_KPMINUS       */  YK_KP_MINUS,
  /* KEY_KP4           */  YK_KP4,
  /* KEY_KP5           */  YK_KP5,
  /* KEY_KP6           */  YK_KP6,
  /* KEY_KPPLUS        */  YK_KP_PLUS,
  /* KEY_KP1           */  YK_KP1,
  /* KEY_KP2           */  YK_KP2,
  /* KEY_KP3           */  YK_KP3,
  /* KEY_KP0           */  YK_KP0,
  /* KEY_KPDOT         */  YK_KP_PERIOD,
  /* KEY_103RD         */  YK_UNKNOWN,
  /* KEY_F13           */  YK_F13,
  /* KEY_102ND         */  YK_UNKNOWN,
  /* KEY_F11           */  YK_F11,
  /* KEY_F12           */  YK_F12,
  /* KEY_F14           */  YK_F14,
  /* KEY_F15           */  YK_F15,
  /* KEY_F16           */  YK_UNKNOWN,
  /* KEY_F17           */  YK_UNKNOWN,
  /* KEY_F18           */  YK_UNKNOWN,
  /* KEY_F19           */  YK_UNKNOWN,
  /* KEY_F20           */  YK_UNKNOWN,
  /* KEY_KPENTER       */  YK_KP_ENTER,
  /* KEY_RIGHTCTRL     */  YK_RCTRL,
  /* KEY_KPSLASH       */  YK_KP_DIVIDE,
  /* KEY_SYSRQ         */  YK_SYSREQ,
  /* KEY_RIGHTALT      */  YK_RALT,
  /* KEY_LINEFEED      */  YK_UNKNOWN,
  /* KEY_HOME          */  YK_HOME,
  /* KEY_UP            */  YK_UP,
  /* KEY_PAGEUP        */  YK_PAGEUP,
  /* KEY_LEFT          */  YK_LEFT,
  /* KEY_RIGHT         */  YK_RIGHT,
  /* KEY_END           */  YK_END,
  /* KEY_DOWN          */  YK_DOWN,
  /* KEY_PAGEDOWN      */  YK_PAGEDOWN,
  /* KEY_INSERT        */  YK_INSERT,
  /* KEY_DELETE        */  YK_DELETE,
  /* KEY_MACRO         */  YK_UNKNOWN,
  /* KEY_MUTE          */  YK_UNKNOWN,
  /* KEY_VOLUMEDOWN    */  YK_UNKNOWN,
  /* KEY_VOLUMEUP      */  YK_UNKNOWN,
  /* KEY_POWER         */  YK_UNKNOWN,
  /* KEY_KPEQUAL       */  YK_KP_EQUALS,
  /* KEY_KPPLUSMINUS   */  YK_UNKNOWN,
  /* KEY_PAUSE         */  YK_PAUSE,
  /* KEY_F21           */  YK_UNKNOWN,
  /* KEY_F22           */  YK_UNKNOWN,
  /* KEY_F23           */  YK_UNKNOWN,
  /* KEY_F24           */  YK_UNKNOWN,
  /* KEY_KPCOMMA       */  YK_UNKNOWN,
  /* KEY_LEFTMETA      */  YK_LSUPER,
  /* KEY_RIGHTMETA     */  YK_RSUPER,
  /* KEY_COMPOSE       */  YK_COMPOSE
};

static size_t keytableLength = sizeof (keytable) / sizeof (enum YKeyCode);

static void
evdevDespatch (struct EvDevInputDriverData *data)
{
  switch (data->cur.type)
    {
      case EV_REL:
        switch (data->cur.code)
          {
             case REL_X:  pointerMovePosition (data->cur.value, 0); break;
             case REL_Y:  pointerMovePosition (0, data->cur.value); break;
             default:     ;
          }
        break;
      case EV_KEY:
        switch (data->cur.code)
          {
             case BTN_LEFT:    pointerButtonChange (0, data->cur.value); break;
             case BTN_MIDDLE:  pointerButtonChange (1, data->cur.value); break;
             case BTN_RIGHT:   pointerButtonChange (2, data->cur.value); break;
             case BTN_SIDE:    pointerButtonChange (3, data->cur.value); break;
             case BTN_EXTRA:   pointerButtonChange (4, data->cur.value); break;
             case BTN_FORWARD: pointerButtonChange (5, data->cur.value); break;
             case BTN_BACK:    pointerButtonChange (6, data->cur.value); break;
             default:
               if (data -> cur.code < keytableLength)
                 {
                   enum YKeyCode code = keytable[data -> cur.code];
                   if (code != YK_UNKNOWN)
                     {
                       if (data -> cur.value == 0) keyboardKeyUp (code);
                       if (data -> cur.value == 1) keyboardKeyDown (code);
                     }
                   if (data->cur.value == 0)
                     ykbKeyUp(data->cur.code);
                   else
                     ykbKeyDown(data->cur.code);
                 }
          }
        break;
      default: ;
    }
}

static void
evdevDataReady (int fd, int causeMask, void *data_v)
{
  struct EvDevInputDriverData *data = data_v;
  ssize_t r;

  r = read (fd, &(data -> cur) + data -> bytes_read, sizeof(struct input_event));
  if (r < 0)
    {
      Y_ERROR ("EvDev: Error Reading: %s", strerror (errno));
      return;
    }

  data -> bytes_read += r;
  if (data -> bytes_read == sizeof(struct input_event))
    {
      evdevDespatch (data);
      data -> bytes_read = 0;
    } 
}

int
initialise (struct Module *module, const struct Tuple *args)
{
  int fd;
  struct EvDevInputDriverData *data;
  char buffer[1024];
  int i;

  static char moduleName[] = "Linux Event Device Input";
  module -> name = moduleName;

  data = ymalloc (sizeof (struct EvDevInputDriverData));
  module -> data = data;
  data -> bytes_read = 0;
  data -> module = module;

  for(i=0; i<16; ++i)
    {
      sprintf (buffer, "%s%d", EVENT_FILE_BASE, i);
      fd = open (buffer, O_RDONLY|O_NONBLOCK|O_NOCTTY);
      data -> fds[i] = fd;
      if (fd > 0)
        {
           controlRegisterFileDescriptor (fd, CONTROL_WATCH_READ,
                                          data, evdevDataReady);
        }
    }

  return 0;
}


int
finalise (struct Module *self)
{
  int i;
  struct EvDevInputDriverData *data = self -> data;
  for (i=0; i<16; ++i)
    if (data -> fds[i] > 0)
      {
        controlUnregisterFileDescriptor (data -> fds[i]);
        close (data -> fds[i]);
      }
  keyboardReset ();
  yfree (self -> data);
  return 0;
}

/* arch-tag: 185781ee-a23d-48d3-9c94-ad81950f15d8
 */
