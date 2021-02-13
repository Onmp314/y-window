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

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <Y/modules/videodriver_interface.h>
#include <Y/modules/module_interface.h>
#include <Y/main/control.h>
#include <Y/buffer/rgbabuffer.h>
#include <Y/screen/viewport.h>
#include <Y/screen/screen.h>
#include <Y/screen/hwrenderer.h>
#include <Y/screen/swrenderer.h>
#include <Y/screen/simplerenderer.h>
#include <Y/input/pointer.h>
#include <Y/input/ykb.h>
#include <Y/util/colour.h>
#include <Y/util/yutil.h>
#include <Y/util/index.h>
#include <SDL.h>

#define SDL_EVENT_POLL_FREQUENCY 100

enum SDLRenderMode
{
  SDL_RENDERMODE_SIMPLE,
  SDL_RENDERMODE_SOFTWARE_BLEND,
  SDL_RENDERMODE_SDL_BLEND
};

struct SDLVideoDriverData
{
  SDL_Surface *sdlSurface;
  struct Viewport *viewport;
  int pollingID;
  enum SDLRenderMode renderMode;
  uint32_t bufferContextID;
  struct Index *bufferContexts;
  bool swcursor;
  bool native_pointer;
  struct VideoResolution curRes;
  SDL_Cursor *cursor;
};

static inline struct SDLVideoDriverData *
sdlData (struct VideoDriver *self)
{
  return (struct SDLVideoDriverData *)(self -> d);
}

static void
sdlEventProbe (void *data_v)
{
  int resized = 0, resizedWidth = 0, resizedHeight = 0;
  struct SDLVideoDriverData *data = data_v;
  SDL_Event event;
  if (SDL_PollEvent (NULL))
    {
      while (SDL_PollEvent (&event))
        switch (event.type)
          {
            case SDL_MOUSEMOTION:
              {
                pointerSetPosition (event.motion.x, event.motion.y);
              }
            break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
              {
                int button = 0;
                int state = 0;
                switch (event.button.button)
                  {
                    case SDL_BUTTON_LEFT:   button = 0; break;
                    case SDL_BUTTON_MIDDLE: button = 1; break;
                    case SDL_BUTTON_RIGHT:  button = 2; break;
                  }
                if (event.button.state == SDL_PRESSED)
                  state = 1;
                else
                  state = 0;
                pointerButtonChange (button, state);
              }
            break;

            case SDL_KEYDOWN:
              keyboardKeyDown (event.key.keysym.sym);
              ykbKeyDown(event.key.keysym.scancode);
              break;
            case SDL_KEYUP:
              keyboardKeyUp (event.key.keysym.sym);
              ykbKeyUp(event.key.keysym.scancode);
              break;

            case SDL_VIDEORESIZE:
              resized = 1;
              resizedWidth = event.resize.w;
              resizedHeight = event.resize.h;
              break;

            case SDL_QUIT:
              controlShutdownY ();
              break;
          }
    }
 
  if (resized)
    {
      data -> sdlSurface =
        SDL_SetVideoMode (resizedWidth, resizedHeight, 32, SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_RESIZABLE);
      viewportSetSize (data -> viewport, resizedWidth, resizedHeight);
    }
  data -> pollingID = controlTimerDelay (0, SDL_EVENT_POLL_FREQUENCY, data_v, sdlEventProbe);
}


static void
sdlGetPixelDimensions (struct VideoDriver *self, int *x, int *y)
{
  *x = sdlData(self) -> sdlSurface -> w;
  *y = sdlData(self) -> sdlSurface -> h;
}

static const char *
sdlGetName (struct VideoDriver *self)
{
  return self -> module -> name;
}

static struct llist *
sdlGetResolutions (struct VideoDriver *self)
{
  struct SDLVideoDriverData *data = sdlData (self);
  struct llist *resolutions = new_llist ();
  yfree (data -> curRes.name);
  data -> curRes.name = ymalloc (50);
  sprintf (data -> curRes.name, "%dx%d",
           data -> sdlSurface -> w, data -> sdlSurface -> h);
  llist_add_tail (resolutions, &(data -> curRes));
  return resolutions;
}

static void
sdlSetPointer (struct VideoDriver *self, const uint8_t *data, int width, int height, int bpp, int hot_x, int hot_y)
{
  /* We should really support other depths */
  if (bpp != 4)
    return;

  if (sdlData (self) -> cursor)
    SDL_FreeCursor(sdlData (self) -> cursor);

  int w = width;
  int h = height;

  /* An SDL cursor must be a multiple of 8 wide, because it's
   * one-bit-per-pixel
   */
  if ((w % 8) != 0)
    w += 8 - (w % 8);

  uint8_t bitmap[w * h];
  uint8_t mask[w * h];

  /* This code is cadged from the SDL documentation */

  /* First pixel will increment this to zero
   */
  int i = -1;
  int y, x;
  for (y = 0; y < h; y++)
    for (x = 0; x < w; x++)
      {
        if ((x % 8) == 0)
          {
            /* New byte. Move along and clear it out */
            i++;
            bitmap[i] = 0;
            mask[i] = 0;
          }
        else
          {
            /* Next bit within this byte, shift the existing ones left */
            bitmap[i] <<= 1;
            mask[i] <<= 1;
          }

        if (x < width)
          {
            /* We should blend better than this. For now:
             *
             *  black -> black
             * !black -> white
             */
            uint32_t pixel;
            memcpy(&pixel, &data[((y * width) + x) * bpp], bpp);
            uint32_t alpha = pixel;
            pixel &= 0x00ffffff;
            alpha >>= 24;
            
            /* We don't want to set bitmap if we don't set mask - that
             * means "invert", which doesn't jive with our expected
             * alpha behaviour
             */
            if (alpha == 0xff)
              {
                mask[i] |= 1;
                if (pixel == 0)
                  bitmap[i] |= 1;
              }
          }
        else
          {
            /* Leave mask zero, so it's transparent */
          }
      }

  sdlData (self) -> cursor = SDL_CreateCursor(bitmap, mask, w, h, hot_x, hot_y);

  if (!sdlData (self) -> cursor)
    return;

  if (!sdlData (self) -> native_pointer)
    SDL_SetCursor(sdlData (self) -> cursor);
}

static struct Tuple *
sdlSpecial (struct VideoDriver *self, const struct Tuple *args)
{
  struct SDLVideoDriverData *data = sdlData (self);

  if (args->count < 1)
    return NULL;
  if (args->list[0].type != t_string)
    return NULL;

  if (strcasecmp (args->list[0].string.data, "simpleRender") == 0)
    {
      struct Rectangle *r = viewportGetRectangle (data -> viewport);
      data -> renderMode = SDL_RENDERMODE_SIMPLE;
      viewportInvalidateRectangle (data -> viewport, r);
      rectangleDestroy (r);
      return NULL;
    }
  if (strcasecmp (args->list[0].string.data, "swRender") == 0)
    {
      struct Rectangle *r = viewportGetRectangle (data -> viewport);
      data -> renderMode = SDL_RENDERMODE_SOFTWARE_BLEND;
      viewportInvalidateRectangle (data -> viewport, r);
      rectangleDestroy (r);
      return NULL;
    }
  if (strcasecmp (args->list[0].string.data, "sdlRender") == 0)
    {
      struct Rectangle *r = viewportGetRectangle (data -> viewport);
      data -> renderMode = SDL_RENDERMODE_SDL_BLEND;
      viewportInvalidateRectangle (data -> viewport, r);
      rectangleDestroy (r);
      return NULL;
    }
  return NULL;
}

static void
sdlBeginUpdates (struct VideoDriver *self)
{
}

static void
sdlEndUpdates (struct VideoDriver *self)
{
  SDL_Flip (sdlData(self) -> sdlSurface);
}

static void
sdlDrawPixel (struct VideoDriver *self, uint32_t col, int x, int y)
{
  unsigned pixel;
  Uint32 *bufp;

  if ( SDL_MUSTLOCK(sdlData(self) -> sdlSurface) )
    if ( SDL_LockSurface(sdlData(self) -> sdlSurface) < 0 )
      return;
  bufp = (Uint32 *)sdlData(self) -> sdlSurface->pixels + y*sdlData(self) -> sdlSurface->pitch/4 + x;

  colourBlendSourceOver (&pixel, *bufp << 8, col, 0xFF);
  *bufp = pixel;

  if ( SDL_MUSTLOCK(sdlData(self) -> sdlSurface) )
    SDL_UnlockSurface(sdlData(self) -> sdlSurface);

}

static void
sdlDrawRectangle (struct VideoDriver *self,
                       uint32_t colour, int x1, int y1, int x2, int y2)
{
  int i;
  Uint32 *bufp1;
  Uint32 *bufp2;

  if ( SDL_MUSTLOCK(sdlData(self) -> sdlSurface) )
    if ( SDL_LockSurface(sdlData(self) -> sdlSurface) < 0 )
      return;

  bufp1 = (Uint32 *)sdlData(self) -> sdlSurface->pixels + (y1)*sdlData(self) -> sdlSurface->pitch/4 + x1;
  bufp2 = (Uint32 *)sdlData(self) -> sdlSurface->pixels + (y2)*sdlData(self) -> sdlSurface->pitch/4 + x1;
  for (i = x1; i < x2; ++i)
    {
      *bufp1 = colour;
      *bufp2 = colour;
      bufp1 ++;
      bufp2 ++;
    }
  bufp1 = (Uint32 *)sdlData(self) -> sdlSurface->pixels + (y1)*sdlData(self) -> sdlSurface->pitch/4 + x1;
  bufp2 = (Uint32 *)sdlData(self) -> sdlSurface->pixels + (y2)*sdlData(self) -> sdlSurface->pitch/4 + x1;
  for (i = y1; i < y2; ++i)
    {
      *bufp1 = colour;
      *bufp2 = colour;
      bufp1 += sdlData(self) -> sdlSurface->pitch/4;
      bufp2 += sdlData(self) -> sdlSurface->pitch/4;
    }

  if ( SDL_MUSTLOCK(sdlData(self) -> sdlSurface) )
    SDL_UnlockSurface(sdlData(self) -> sdlSurface);
}

static void
sdlDrawFilledRectangle (struct VideoDriver *self, uint32_t colour,
                        int x1, int y1, int x2, int y2)
{

  Uint32 *bufp;
  int i, j;

  if ( SDL_MUSTLOCK(sdlData(self) -> sdlSurface) )
    if ( SDL_LockSurface(sdlData(self) -> sdlSurface) < 0 )
      return;

  for (j = y1; j < y2; ++j)
    {
      bufp = (Uint32 *)sdlData(self) -> sdlSurface->pixels + (j)*sdlData(self) -> sdlSurface->pitch/4 + x1;
      for (i = x1; i < x2; ++i)
        {
          *bufp = colour;
          ++bufp;
        }
    }

  if ( SDL_MUSTLOCK(sdlData(self) -> sdlSurface) )
    SDL_UnlockSurface(sdlData(self) -> sdlSurface);

}

static void
sdlBlit (struct VideoDriver *self, uint32_t *data,
         int x, int y, int w, int h, int stepping)
{

  uint32_t * bufferLine;
  Uint32 *bufp;
  int i, j;

  if ( SDL_MUSTLOCK(sdlData(self) -> sdlSurface) )
    if ( SDL_LockSurface(sdlData(self) -> sdlSurface) < 0 )
      return;

  for (j = 0; j < h; ++j)
    {
      bufferLine = data + (j * stepping);
      bufp = (Uint32 *)sdlData(self) -> sdlSurface->pixels + (y+j)*sdlData(self) -> sdlSurface->pitch/4 + x;
      for (i = 0; i < w; ++i)
        {
          *bufp = *bufferLine; /* colourBlend (*bufp << 8, *bufferLine, *bufferLine & 0xFF); */
          ++bufferLine;
          ++bufp;
        }
    }

  if ( SDL_MUSTLOCK(sdlData(self) -> sdlSurface) )
    SDL_UnlockSurface(sdlData(self) -> sdlSurface);

}

struct SDLAccelBufferContext
{
  struct BufferContext context;
  struct VideoDriver *videodriver;
  SDL_Surface *sdlSurface;
};

static int
sdlAccelBufferKeyFunction (const void *key_v, const void *obj_v)
{
  const int *key = key_v;
  const struct SDLAccelBufferContext *obj = obj_v;
  return *key - bufferGetID (obj->context.buffer);
}

static int
sdlAccelBufferComparisonFunction (const void *obj1_v, const void *obj2_v)
{
  const struct SDLAccelBufferContext *obj1 = obj1_v;
  const struct SDLAccelBufferContext *obj2 = obj2_v;
  return bufferGetID (obj1->context.buffer) - bufferGetID (obj2->context.buffer);
}

static void
sdlAccelBufferDestructorFunction (void *obj)
{
  struct SDLAccelBufferContext *context = obj;
  bufferRemoveContext (context->context.buffer, context->context.id);
}

static void
sdlAccelBufferModified (struct BufferContext *context_b)
{
  struct SDLAccelBufferContext *context =
    (struct SDLAccelBufferContext *)context_b;

  SDL_FreeSurface (context->sdlSurface);
  context->sdlSurface = NULL;
}

static void
sdlAccelBufferDestroy (struct BufferContext *context_b)
{
  struct SDLAccelBufferContext *context =
    (struct SDLAccelBufferContext *)context_b;
  int bufferID = bufferGetID (context->context.buffer);

  indexRemove (sdlData (context->videodriver)->bufferContexts, &bufferID);
  SDL_FreeSurface (context->sdlSurface);
  yfree (context);
}

static bool
sdlAccelRenderBuffer (struct VideoDriver *self, struct Buffer *buffer,
                 int x, int y, int xo, int yo, int rw, int rh)
{
  int w, h;
  bufferGetSize (buffer, &w, &h);

  if (bufferIsRGBABuffer (buffer))
    {
      struct RGBABuffer *rgbabuffer = (struct RGBABuffer *)buffer;

      /* have we seen this buffer before? */
      struct SDLAccelBufferContext *context =
        (struct SDLAccelBufferContext *)bufferGetContext (buffer,
        sdlData (self)->bufferContextID);

      if (context == NULL)
        {
          /* this is a new buffer, generate a new context */
          context = ymalloc (sizeof (struct SDLAccelBufferContext));
          context->context.id = sdlData (self)->bufferContextID;
          context->context.buffer = buffer;
          context->context.modified = sdlAccelBufferModified;
          context->context.destroy = sdlAccelBufferDestroy;
          context->videodriver = self;
          context->sdlSurface = NULL;

          bufferAddContext (buffer, &(context->context));
          indexAdd (sdlData (self)->bufferContexts, context);
        }

      if (context->sdlSurface == NULL)
        {
          /* the buffer is either new or modified, regenerate surface */
          int dw;
          uint32_t *data;
          rgbabufferAccessInternals (rgbabuffer, &dw, NULL, &data);

          context->sdlSurface = SDL_CreateRGBSurfaceFrom (data, w, h,
            32, dw*sizeof(uint32_t), 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
        }

      SDL_Rect srcRect = { xo, yo, rw, rh };
      SDL_Rect destRect = { x+xo, y+yo, rw, rh };

      SDL_BlitSurface (context->sdlSurface, &srcRect,
                       sdlData (self)->sdlSurface, &destRect);

      return true;
    }
  return false;
}

static void
sdlAccelBlitRGBAData (struct VideoDriver *self, int x, int y,
                      const uint32_t *data_const, int w, int h, int s)
{
  /* we can't do this because SDL blows goats */
  return;
}

static void
sdlAccelDrawFilledRectangle (struct VideoDriver *self, uint32_t colour,
                             int x, int y, int w, int h)
{
  SDL_Rect fillRect = { x, y, w, h };

  SDL_FillRect (sdlData (self)->sdlSurface, &fillRect, colour);
}

static struct HWRendererAccel sdlRendererAccel =
{
  complete:            NULL,
  destroy:             NULL,
  renderBuffer:        sdlAccelRenderBuffer,
  blitRGBAData:        sdlAccelBlitRGBAData,
  drawFilledRectangle: sdlAccelDrawFilledRectangle
};

static struct Renderer *
sdlGetRenderer (struct VideoDriver *self, const struct Rectangle *rect)
{
  struct Renderer *renderer;
  switch (sdlData (self) -> renderMode)
    {
    case SDL_RENDERMODE_SIMPLE:
      renderer = simplerendererGetRenderer (simplerendererCreate (self, rect));
      break;
    case SDL_RENDERMODE_SOFTWARE_BLEND:
      renderer = swrendererGetRenderer (swrendererCreate (self, rect));
      break;
    case SDL_RENDERMODE_SDL_BLEND:
      renderer = hwrendererGetRenderer (hwrendererCreate (self, rect,
        &sdlRendererAccel));
      break;
    default:
      return NULL;
    }

  if (!sdlData (self) -> swcursor)
    rendererSetOption (renderer, "hardware pointer", "yes");

  return renderer;
}

int
initialise (struct Module *module, const struct Tuple *args)
{
  struct VideoDriver *videodriver;
  bool swcursor = false;
  bool native_pointer = true;
  enum SDLRenderMode renderMode = SDL_RENDERMODE_SDL_BLEND;
  int screenx = 800;
  int screeny = 600;

  for (uint32_t i = 0; i < args->count; ++i)
    {
      if (args->list[i].type != t_string)
        continue;

      const char *arg = args->list[i].string.data;

      if (strncmp (arg, "swcursor=", 9) == 0)
        {
          if (strcmp(arg + 9, "yes") == 0 || strcmp(arg + 9, "true") == 0)
            {
              swcursor = true;
            }
          else if (strcmp(arg + 9, "no") == 0 || strcmp(arg + 9, "false") == 0)
            {
              swcursor = false;
            }
        }
      else if (strncmp (arg, "native_pointer=", 9) == 0)
        {
          if (strcmp(arg + 9, "yes") == 0 || strcmp(arg + 9, "true") == 0)
            {
              native_pointer = true;
            }
          else if (strcmp(arg + 9, "no") == 0 || strcmp(arg + 9, "false") == 0)
            {
              native_pointer = false;
            }
        }
      else if (strncmp (arg, "render_mode=", 12) == 0)
        {
          if (strcmp (arg + 12, "simple") == 0)
            {
              renderMode = SDL_RENDERMODE_SIMPLE;
            }
          else if (strcmp (arg + 12, "software") == 0 || strcmp (arg + 12, "sw") == 0)
            {
              renderMode = SDL_RENDERMODE_SOFTWARE_BLEND;
            }
          else if (strcmp (arg + 12, "sdl") == 0)
            {
              renderMode = SDL_RENDERMODE_SDL_BLEND;
            }
        }
      else if (strncmp (arg, "res=", 4) == 0)
        {
	  const char * res = arg + 4;
	  const char * xloc = strchr(res,'x');
	  if (xloc == NULL)
	    {
	      Y_ERROR ("sdl: can't parse resolution string %s:", res);
	    }
	  else
	    {
	      screeny = strtol (xloc + 1, NULL, 0);
	      screenx = strtol (res, NULL, 0);
              Y_TRACE ("sdl: setting resolution to %i x %i", screenx, screeny);
            }
        }
    }

  if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_NOPARACHUTE) < 0 )
    {
        Y_ERROR ("sdl: Unable to init: %s", SDL_GetError());
        return 1;
    }

  videodriver = ymalloc (sizeof (struct VideoDriver));
  videodriver -> d = ymalloc (sizeof (struct SDLVideoDriverData)); 
  videodriver -> module = module;

  sdlData(videodriver) -> sdlSurface =
    SDL_SetVideoMode(screenx, screeny, 32,
                     SDL_HWSURFACE|SDL_RESIZABLE);

  SDL_WM_SetCaption ("Y on SDL", NULL);

  videodriver -> getPixelDimensions = sdlGetPixelDimensions;
  videodriver -> getName = sdlGetName;
  videodriver -> getResolutions = sdlGetResolutions;
  videodriver -> setPointer = sdlSetPointer;
  videodriver -> special = sdlSpecial;
  videodriver -> beginUpdates = sdlBeginUpdates;
  videodriver -> endUpdates = sdlEndUpdates;
  videodriver -> drawPixel = sdlDrawPixel;
  videodriver -> drawRectangle = sdlDrawRectangle;
  videodriver -> drawFilledRectangle = sdlDrawFilledRectangle;
  videodriver -> blit = sdlBlit;  
  videodriver -> getRenderer = sdlGetRenderer;

  static char moduleName[] = "SDL Video Driver";
  module -> name = moduleName;
  module -> data = videodriver;

  sdlData (videodriver) -> cursor = NULL;
  sdlData (videodriver) -> curRes.name = NULL;
  sdlData (videodriver) -> renderMode = renderMode;
  sdlData (videodriver) -> swcursor = swcursor;
  sdlData (videodriver) -> native_pointer = native_pointer;
  sdlData (videodriver) -> viewport = viewportCreate (videodriver);
  sdlData (videodriver) -> bufferContextID = bufferNewContextID ();
  sdlData (videodriver) -> bufferContexts = indexCreate (
    &sdlAccelBufferKeyFunction, &sdlAccelBufferComparisonFunction);
  screenRegisterViewport (sdlData (videodriver) -> viewport);

  if (swcursor)
    SDL_ShowCursor (SDL_DISABLE);
 
  sdlData (videodriver) -> pollingID =
    controlTimerDelay (0, SDL_EVENT_POLL_FREQUENCY, sdlData(videodriver), sdlEventProbe);

  return 0;
}

int
finalise (struct Module *module)
{
  struct VideoDriver *videodriver = module -> data;
  bool swcursor = sdlData(videodriver) -> swcursor;
  keyboardReset ();
  screenUnregisterViewport (sdlData (videodriver) -> viewport);
  viewportDestroy (sdlData (videodriver) -> viewport);
  controlCancelTimerDelay (sdlData (videodriver) -> pollingID);
  if (sdlData (videodriver) -> cursor)
    SDL_FreeCursor(sdlData (videodriver) -> cursor);
  indexDestroy (sdlData (videodriver)->bufferContexts,
                &sdlAccelBufferDestructorFunction);
  yfree(sdlData(videodriver));
  yfree(videodriver);
  if (swcursor)
    SDL_ShowCursor (SDL_ENABLE);
  SDL_Quit ();
  return 0;
}

/* arch-tag: 66bb3c53-a3a8-4fe7-806f-d653d440e267
 */
