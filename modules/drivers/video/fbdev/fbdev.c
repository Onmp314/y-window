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
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/user.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/fb.h>
#include <linux/vt.h>
#include <linux/kd.h>
#include <Y/modules/videodriver_interface.h>
#include <Y/modules/module_interface.h>
#include <Y/screen/viewport.h>
#include <Y/screen/screen.h>
#include <Y/screen/swrenderer.h>
#include <Y/screen/simplerenderer.h>
#include <Y/util/colour.h>
#include <Y/util/yutil.h>
#include <Y/util/index.h>
#include <asm/page.h>

struct FBDevMode
{
  struct VideoResolution vres;
  struct fb_var_screeninfo vscreeninfo;
};

/* there can only be one... */
static struct VideoDriver *fbdevInstance = NULL;

enum FBDevSwitchState
{
  FBDEV_SWITCH_ACTIVE,
  FBDEV_SWITCH_RELEASED,
  FBDEV_SWITCH_REQUEST_ACQUIRE,
  FBDEV_SWITCH_REQUEST_RELEASE,
  FBDEV_SWITCH_FINISHING
};

struct FBDevVideoDriverData
{
  struct fb_fix_screeninfo fscreeninfo;
  struct fb_var_screeninfo vscreeninfo, old_vscreeninfo;
  uint16_t old_red[256], old_green[256], old_blue[256];
  uint16_t red[256], green[256], blue[256];
  struct fb_cmap old_cmap;
  struct fb_cmap cmap;
  int kd_mode, old_kdmode;
  struct vt_mode vtmode, old_vtmode;
  struct termios term;
  int ttynum;
  int fbfd, ttyfd;
  uint32_t *data;
  unsigned long dataOffset;
  struct Viewport *viewport;
  struct Index *modes;
  enum FBDevSwitchState switchState;
  int updating;
  int renderSimply;
};

static int
fbdevmodeComparisonFunction (const void *m1_v, const void *m2_v)
{
  const struct FBDevMode *m1 = m1_v, *m2 = m2_v;
  return strcmp (m1->vres.name, m2->vres.name);
}

static int
fbdevmodeKeyFunction (const void *key_v, const void *m_v)
{
  const char *key = key_v;
  const struct FBDevMode *m = m_v;
  return strcmp (key, m->vres.name);
}

static void
fbdevmodeDestructorFunction (struct FBDevMode *m)
{
  yfree (m->vres.name);
  yfree (m);
}

static void
fbdevReleaseConsole (struct VideoDriver *self)
{
  struct FBDevVideoDriverData *data = self -> d;
  ioctl (data -> ttyfd, VT_RELDISP, 1);
  data -> switchState = FBDEV_SWITCH_RELEASED;
}

static void
fbdevAcquireConsole (struct VideoDriver *self)
{
  struct FBDevVideoDriverData *data = self -> d;
  struct Rectangle *r;
  if (ioctl (data -> ttyfd, VT_RELDISP, VT_ACKACQ) < 0)
    {
      Y_ERROR ("Couldn't Reacquire Console (a): %s", strerror (errno));
      return;
    }
  if (ioctl (data -> ttyfd, VT_SETMODE, &(data -> vtmode)) < 0)
    {
      Y_ERROR ("Couldn't Reacquire Console (b): %s", strerror (errno));
      return;
    }
  if (ioctl (data -> ttyfd, KDSETMODE, KD_GRAPHICS) < 0)
    {
      Y_ERROR ("Couldn't Reacquire Console (c): %s", strerror (errno));
      return;
    }
  data -> switchState = FBDEV_SWITCH_ACTIVE;
  r = viewportGetRectangle (data -> viewport);
  viewportInvalidateRectangle (data -> viewport, r);
  rectangleDestroy (r);
}

static void
fbdevSwitchSignals (int signal)
{
  struct FBDevVideoDriverData *data = fbdevInstance -> d;
  if (data -> switchState == FBDEV_SWITCH_FINISHING)
    return;
  if (signal == SIGUSR1)
    {
      /* release the terminal */
      if (data -> updating)
        data -> switchState = FBDEV_SWITCH_REQUEST_RELEASE;
      else
        fbdevReleaseConsole (fbdevInstance);
    }
  if (signal == SIGUSR2)
    {
      /* acquire the terminal */
      if (data -> updating)
        data -> switchState = FBDEV_SWITCH_REQUEST_ACQUIRE;
      else
        fbdevAcquireConsole (fbdevInstance);
    }
}

static void
fbdevGetPixelDimensions (struct VideoDriver *self, int *x, int *y)
{
  struct FBDevVideoDriverData *data = self -> d;
  *x = data -> vscreeninfo.xres;
  *y = data -> vscreeninfo.yres;
}

static void
fbdevBeginUpdates (struct VideoDriver *self)
{
  struct FBDevVideoDriverData *data = self -> d;
  data -> updating = 1;
}

static void
fbdevEndUpdates (struct VideoDriver *self)
{
  struct FBDevVideoDriverData *data = self -> d;
  if (data -> switchState == FBDEV_SWITCH_REQUEST_RELEASE)
    {
      fbdevReleaseConsole (self);
    }
  else if (data -> switchState == FBDEV_SWITCH_REQUEST_ACQUIRE)
    {
      fbdevAcquireConsole (self);
    }
  data -> updating = 0;
}

static void
fbdevDrawPixel (struct VideoDriver *self, uint32_t col, int x, int y)
{
/*  colourBlendSourceOver (&pixel, *bufp, col, 0xFF); */
}

static void
fbdevDrawRectangle (struct VideoDriver *self,
                    uint32_t colour, int x1, int y1, int x2, int y2)
{

}

static void
fbdevDrawFilledRectangle (struct VideoDriver *self, uint32_t colour,
                          int x1, int y1, int x2, int y2)
{
  int i, j;
  struct FBDevVideoDriverData *data = self -> d;
  if (data -> switchState != FBDEV_SWITCH_ACTIVE)
    return;
  for (j=y1; j<=y2; ++j)
    {
      uint32_t *line = data -> data + data -> dataOffset
                       + y1 * data -> vscreeninfo.xres_virtual
                       + x1;
      for (i=x1; i<=x2; ++i)
        {
          colourBlendSourceOver (line, *line | 0xFF000000, colour, 0xFF);
          ++line;
        }
      
    }
}

static void
fbdevBlit (struct VideoDriver *self, uint32_t *from,
         int x, int y, int w, int h, int stepping)
{
  uint32_t *fline, *tline;
  int i, j;
  struct FBDevVideoDriverData *data = self -> d;
  if (data -> switchState != FBDEV_SWITCH_ACTIVE)
    return;
  tline = data -> data + data -> dataOffset
          + y * data -> vscreeninfo.xres_virtual
          + x;
  fline = from;
  for (j=0; j<h; ++j)
    {
      uint32_t *fdata = fline;
      uint32_t *tdata = tline;
      for (i=0; i<w; ++i)
        {
          *tdata = *fdata;
          ++tdata; ++fdata;
        }
      fline += stepping;
      tline += data -> vscreeninfo.xres_virtual;
    }
}


static struct Renderer *
fbdevGetRenderer (struct VideoDriver *self, const struct Rectangle *rect)
{
  struct FBDevVideoDriverData *data = self -> d;
  if (data -> renderSimply)
    return simplerendererGetRenderer (simplerendererCreate (self, rect)); 
  else
    return swrendererGetRenderer (swrendererCreate (self, rect));
}

static void
parseLine (struct Index *idx, char *buffer, struct FBDevMode **mode,
           struct fb_var_screeninfo *bp)
{
  buffer += strspn (buffer, " \t");
  if (strncmp (buffer, "mode ", 5) == 0)
    {
      char *e;
      buffer = strchr (buffer, '"');
      if (buffer == NULL)
        return;
      ++buffer;
      e = strchr (buffer, '"');
      if (e == NULL)
        return;
      *e = '\0';
      if ((*mode) -> vres.name != NULL)
        yfree ((*mode) -> vres.name);
      (*mode) -> vres.name = ystrdup (buffer);
    }
  else if (strncmp (buffer, "geometry ", 9) == 0)
    {
      size_t l;
      uint32_t values[5];
      int i;
      buffer += 9;

      for (i=0; i<5; ++i)
        {
          buffer += strspn (buffer, " \t");
          l = strspn (buffer, "0123456789");
          if (l <= 0)
            return;
          values[i] = strtoul (buffer, NULL, 10);
          buffer += l;
        }
 
      (*mode) -> vscreeninfo.xres = values[0];
      (*mode) -> vscreeninfo.yres = values[1];
      (*mode) -> vscreeninfo.xres_virtual = values[2];
      (*mode) -> vscreeninfo.yres_virtual = values[3];
      (*mode) -> vscreeninfo.bits_per_pixel = values[4];
    }
  else if (strncmp (buffer, "timings ", 8) == 0)
    { 
      size_t l;
      uint32_t values[7];
      int i;
      buffer += 8;

      for (i=0; i<7; ++i)
        {
          buffer += strspn (buffer, " \t");
          l = strspn (buffer, "0123456789");
          if (l <= 0)
            return;
          values[i] = strtoul (buffer, NULL, 10);
          buffer += l;
        }

      (*mode) -> vscreeninfo.pixclock = values[0]; 
      (*mode) -> vscreeninfo.left_margin = values[1]; 
      (*mode) -> vscreeninfo.right_margin = values[2]; 
      (*mode) -> vscreeninfo.upper_margin = values[3]; 
      (*mode) -> vscreeninfo.lower_margin = values[4]; 
      (*mode) -> vscreeninfo.hsync_len = values[5]; 
      (*mode) -> vscreeninfo.vsync_len = values[6];
    }
  else if (strncmp (buffer, "hsync ", 6) == 0)
    {
      buffer += 6;
      buffer += strspn (buffer, " \t");
      if (strncmp (buffer, "low", 3) == 0)
        (*mode) -> vscreeninfo.sync &= ~FB_SYNC_HOR_HIGH_ACT;
      if (strncmp (buffer, "high", 4) == 0)
        (*mode) -> vscreeninfo.sync |=  FB_SYNC_HOR_HIGH_ACT;
    }
  else if (strncmp (buffer, "vsync ", 6) == 0)
    {
      buffer += 6;
      buffer += strspn (buffer, " \t");
      if (strncmp (buffer, "low", 3) == 0)
        (*mode) -> vscreeninfo.sync &= ~FB_SYNC_VERT_HIGH_ACT;
      if (strncmp (buffer, "high", 4) == 0)
        (*mode) -> vscreeninfo.sync |=  FB_SYNC_VERT_HIGH_ACT;
    }
  else if (strncmp (buffer, "csync ", 6) == 0)
    {
      buffer += 6;
      buffer += strspn (buffer, " \t");
      if (strncmp (buffer, "low", 3) == 0)
        (*mode) -> vscreeninfo.sync &= ~FB_SYNC_COMP_HIGH_ACT;
      if (strncmp (buffer, "high", 4) == 0)
        (*mode) -> vscreeninfo.sync |=  FB_SYNC_COMP_HIGH_ACT;
    }
  else if (strncmp (buffer, "extsync ", 8) == 0)
    {
      buffer += 8;
      buffer += strspn (buffer, " \t");
      if (strncmp (buffer, "false", 5) == 0)
        (*mode) -> vscreeninfo.sync &= ~FB_SYNC_EXT;
      if (strncmp (buffer, "true", 4) == 0)
        (*mode) -> vscreeninfo.sync |=  FB_SYNC_EXT;
    }
  else if (strncmp (buffer, "laced ", 6) == 0)
    {
      buffer += 6;
      buffer += strspn (buffer, " \t");
      if (strncmp (buffer, "false", 5) == 0)
        (*mode) -> vscreeninfo.vmode &= ~FB_VMODE_INTERLACED;
      if (strncmp (buffer, "true", 4) == 0)
        (*mode) -> vscreeninfo.vmode |=  FB_VMODE_INTERLACED;
    }
  else if (strncmp (buffer, "double ", 7) == 0)
    {
      buffer += 7;
      buffer += strspn (buffer, " \t");
      if (strncmp (buffer, "false", 5) == 0)
        (*mode) -> vscreeninfo.vmode &= ~FB_VMODE_DOUBLE;
      if (strncmp (buffer, "true", 4) == 0)
        (*mode) -> vscreeninfo.vmode |=  FB_VMODE_DOUBLE;
    }
  else if (strncmp (buffer, "endmode", 7) == 0)
    {
      if ((*mode) -> vres.name != NULL)
        {
          if ((*mode) -> vscreeninfo.bits_per_pixel == 32)
            {
              Y_TRACE ("Adding mode %s", (*mode) -> vres.name);
              indexAdd (idx, *mode);
            }
          else
            yfree (*mode);
          *mode = ymalloc (sizeof (struct FBDevMode));
          (*mode) -> vres.name = NULL;
          memcpy (&((*mode) -> vscreeninfo), bp,
                  sizeof (struct fb_var_screeninfo));
        }
    }
}

static int
fbdevLoadModeDB (struct VideoDriver *self, struct fb_var_screeninfo *bp,
                 const char *dbpath)
{
  struct FBDevVideoDriverData *data = self -> d;
  char buffer[1024];
  FILE *db;
  int dbfd;
  size_t ret;
  int position;
  struct FBDevMode *currentMode;
  
  dbfd = open (dbpath, O_RDONLY | O_NOCTTY);
  if (dbfd < 0)
    {
      Y_ERROR ("Error opening Framebuffer Mode Database %s: %s", dbpath,
               strerror (errno));
      return 1;
    }
  db =  fdopen (dbfd, "r");

  currentMode = ymalloc (sizeof (struct FBDevMode));
  currentMode -> vres.name = NULL;
  memcpy (&(currentMode -> vscreeninfo), bp, sizeof (struct fb_var_screeninfo));
  position = 0;
  do
    {
      ret = fread (buffer + position, sizeof (char), 1, db);
      if (ret > 0)
        {
          if (buffer[position] == '\n')
            {
              if (position == 0 || buffer[position - 1] != '\\')
                {
                  buffer[position] = '\0';
                  parseLine (data -> modes, buffer, &currentMode, bp);
                  position = 0;
                }
              else
                --position;
            }
          else
            {
             ++position;
               if (position >= 1023)
                 position = 1023;
            }
        }
    }
  while (ret > 0);

  yfree (currentMode);

  if (ferror (db))
    {
      Y_ERROR ("Error reading Framebuffer Mode Database %s: %s", dbpath,
               strerror (errno));
      fclose (db);
      return 1;
    }
  fclose (db);
  return 0; 
}

static void
fbdevSetResolution (struct VideoDriver *self, const char *modename)
{
  struct FBDevVideoDriverData *data = self -> d;
  struct FBDevMode *mode;

  Y_TRACE ("Looking for mode: %s", modename);
  mode = indexFind (data -> modes, modename);
  if (mode == NULL)
    return;

  Y_TRACE ("Found it; switching...");

  if (ioctl (data -> fbfd, FBIOPUT_VSCREENINFO, &(mode -> vscreeninfo)) < 0)
    {
      Y_ERROR ("Couldn't change resolution to %s", modename);
      return;
    }

  memcpy (&(data -> vscreeninfo), &(mode -> vscreeninfo),
          sizeof (struct fb_var_screeninfo));

  if (data -> viewport != NULL)
    {
      viewportSetSize (data -> viewport, data -> vscreeninfo.xres,
                       data -> vscreeninfo.yres);
    }
}

static struct Tuple *
fbdevSpecial (struct VideoDriver *self, const struct Tuple *args)
{
  struct FBDevVideoDriverData *data = self -> d;

  if (args->count < 1)
    return NULL;
  if (args->list[0].type != t_string)
    return NULL;

  if (strcasecmp (args->list[0].string.data, "simpleRender") == 0)
    {
      struct Rectangle *r = viewportGetRectangle (data -> viewport);
      data -> renderSimply = 1;
      viewportInvalidateRectangle (data -> viewport, r);
      rectangleDestroy (r);
      return NULL;
    }
  if (strcasecmp (args->list[0].string.data, "fullRender") == 0)
    {
      struct Rectangle *r = viewportGetRectangle (data -> viewport);
      data -> renderSimply = 0;
      viewportInvalidateRectangle (data -> viewport, r);
      rectangleDestroy (r);
      return NULL;
    }
  return NULL;
}

static const char *
fbdevGetName (struct VideoDriver *self)
{
  return self -> module -> name;
}

static struct llist *
fbdevGetResolutions (struct VideoDriver *self)
{
  struct llist *res;
  struct IndexIterator *iter;
  struct FBDevVideoDriverData *data = self -> d;

  res = new_llist ();
  iter = indexGetStartIterator (data -> modes);
  while (indexiteratorHasValue (iter))
    {
      struct FBDevMode *mode = indexiteratorGet (iter);
      llist_add_tail (res, &(mode -> vres));
      indexiteratorNext (iter);
    }
  indexiteratorDestroy (iter);
  return res;
}

int
initialise (struct Module *module, const struct Tuple *args)
{
  struct VideoDriver *videodriver;
  struct FBDevVideoDriverData *data;
  const char *fbpath = NULL;
  const char *ttypath = NULL;
  const char *dbpath = NULL;
  const char *mode = NULL;

  if (fbdevInstance != NULL)
    {
      /* there can be only one... */
      Y_ERROR ("There can only be one instance of the FBDev driver loaded at once");
      return 1;
    }

  videodriver = ymalloc (sizeof (struct VideoDriver));
  data = ymalloc (sizeof (struct FBDevVideoDriverData));
  videodriver -> d = data;
  videodriver -> module = module;
  data -> data = NULL;
  data -> viewport = NULL;
  data -> modes = indexCreate (fbdevmodeKeyFunction,
                               fbdevmodeComparisonFunction);

  fbdevInstance = videodriver;

  module -> name = ymalloc (256);
  strcpy (module -> name, "Framebuffer Device");

  for (uint32_t i = 0; i < args->count; ++i)
    {
      if (args->list[i].type != t_string)
        continue;

      const char *arg = args->list[i].string.data;

      if (strncmp (arg, "device=", 7) == 0)
        fbpath = arg + 7;
      if (strncmp (arg, "tty=", 4) == 0)
        ttypath = arg + 4;
      if (strncmp (arg, "db=", 3) == 0)
        dbpath = arg + 3;
      if (strncmp (arg, "mode=", 5) == 0)
        mode = arg + 5;
    }

  if (fbpath == NULL || strlen (fbpath) == 0)
    fbpath = getenv ("FRAMEBUFFER");
  if (fbpath == NULL || strlen (fbpath) == 0)
    fbpath = "/dev/fb0";

  if (ttypath == NULL || strlen (ttypath) == 0)
    ttypath = "/dev/tty";

  if (dbpath == NULL || strlen (dbpath) == 0)
    dbpath = "/etc/fb.modes";

  data -> ttynum = strtol (ttypath + strcspn (ttypath, "0123456789"), NULL, 10);

  if (data -> ttynum == 0)
    {
       /* obtain the first available tty */
       /* FIXME */
    }

  Y_TRACE ("Using fb:%s tty:%s (%d)", fbpath, ttypath, data -> ttynum);

  /* obtain the tty */
  chown (ttypath, getuid (), getgid ());

  /* open the tty */
  data -> ttyfd = open (ttypath, O_RDWR | O_NOCTTY);
  if (data -> ttyfd < 0)
    {
      Y_ERROR ("Couldn't open %s: %s", ttypath, strerror (errno));
      yfree (videodriver);
      yfree (data);
      return 1;
    }

  /* activate the tty */
  if (ioctl (data -> ttyfd, VT_ACTIVATE, data -> ttynum) < 0)
    {
      Y_ERROR ("Couldn't activate %s: %s", ttypath, strerror (errno));
      close (data -> ttyfd);
      yfree (videodriver);
      yfree (data);
      return 1;
    }

  if (ioctl (data -> ttyfd, VT_WAITACTIVE, data -> ttynum) < 0)
    {
      Y_ERROR ("Couldn't wait for %s to activate (?!): %s", ttypath, strerror (errno));
      close (data -> ttyfd);
      yfree (videodriver);
      yfree (data);
      return 1;
   }

  /* open the fb device */
  data -> fbfd = open (fbpath, O_RDWR | O_NOCTTY);
  if (data -> fbfd < 0)
    {
      Y_ERROR ("Couldn't open %s: %s", fbpath, strerror (errno));
      close (data -> ttyfd);
      yfree (videodriver);
      yfree (data);
      return 1;
    }

  data -> old_cmap.start = 0;
  data -> old_cmap.len = 256;
  data -> old_cmap.red = data -> old_red;
  data -> old_cmap.green = data -> old_green;
  data -> old_cmap.blue = data -> old_blue;
  data -> old_cmap.transp = 0;

  /* get frame buffer settings */
  if (ioctl (data -> fbfd, FBIOGET_FSCREENINFO, &(data -> fscreeninfo)) < 0)
    {
      Y_ERROR ("Couldn't get fixed screen info: %s", strerror (errno));
      close (data -> fbfd);
      close (data -> ttyfd);
      yfree (videodriver);
      yfree (data);
      return 1;
    }

  if (ioctl (data -> fbfd, FBIOGET_VSCREENINFO, &(data -> old_vscreeninfo)) < 0)
    {
      Y_ERROR ("Couldn't get variable screen info: %s", strerror (errno));
      close (data -> fbfd);
      close (data -> ttyfd);
      yfree (videodriver);
      yfree (data);
      return 1;
    }

  if (ioctl (data -> fbfd, FBIOGETCMAP, &(data -> old_cmap)))
    {
      Y_ERROR ("Couldn't get screen colour map info: %s", strerror (errno));
      close (data -> fbfd);
      close (data -> ttyfd);
      yfree (videodriver);
      yfree (data);
      return 1;
    }

  memcpy (&(data -> vscreeninfo), &(data -> old_vscreeninfo), sizeof (struct fb_var_screeninfo));

  /* get tty settings */
  if (ioctl (data -> ttyfd, KDGETMODE, &(data -> old_kdmode)) < 0
      || ioctl (data -> ttyfd, VT_GETMODE, &(data -> old_vtmode)) < 0
      || tcgetattr (data -> ttyfd, &(data -> term)) < 0)
    {
      Y_ERROR ("Couldn't get terminal info: %s", strerror (errno));
      close (data -> fbfd);
      close (data -> ttyfd);
      yfree (videodriver);
      yfree (data);
      return 1;
    }

  /* mmap the video ram */
  data -> data = mmap (0, data -> fscreeninfo.smem_len,
                       PROT_READ | PROT_WRITE, MAP_SHARED,
                       data -> fbfd, 0);
  data -> dataOffset = data -> fscreeninfo.smem_start & ~PAGE_MASK;
 
  /* pull some useful information out of the screen info */

  if (strlen (data -> fscreeninfo.id) > 0)
    {
      strcat (module -> name, ": ");
      strncat (module -> name, data -> fscreeninfo.id, 16);
    }

  data -> vscreeninfo.accel_flags = FB_ACCEL_NONE;

  /* find all the modes from the mode db */
  if (fbdevLoadModeDB (videodriver, &(data -> vscreeninfo), dbpath) != 0)
    {
      Y_ERROR ("Couldn't read mode database");
      munmap (data -> data, data -> fscreeninfo.smem_len);
      close (data -> fbfd);
      close (data -> ttyfd);
      yfree (videodriver);
      yfree (data);
      return 1;
    }

  /* change to any specified resolution */
  if (mode != NULL && strlen (mode) > 0)
    fbdevSetResolution (videodriver, mode); 

  /* switch to graphics mode */
  if (ioctl (data -> ttyfd, KDSETMODE, KD_GRAPHICS))
    {
      Y_ERROR ("Couldn't switch to Graphics Mode: %s", strerror (errno));
      munmap (data -> data, data -> fscreeninfo.smem_len);
      close (data -> fbfd);
      close (data -> ttyfd);
      yfree (videodriver);
      yfree (data);
      return 1;
    }

  /* set the colour map */
  data -> cmap.start = 0;
  data -> cmap.len = 256;
  data -> cmap.red = data -> red;
  data -> cmap.green = data -> green;
  data -> cmap.blue = data -> blue;
  data -> cmap.transp = 0;
  {
    uint16_t * red_p = data -> cmap.red;
    uint16_t * green_p = data -> cmap.green;
    uint16_t * blue_p = data -> cmap.blue;
    uint16_t j;
    for (j=0; j<256; ++j)
      {
        *red_p++ = j << 8 | j;
        *green_p++ = j << 8 | j;
        *blue_p++ = j << 8 | j;
      }
  }
  if (ioctl (data -> fbfd, FBIOPUTCMAP, &(data -> cmap)) < 0)
    {
      Y_ERROR ("Couldn't set gamma profile: %s", strerror (errno));
      munmap (data -> data, data -> fscreeninfo.smem_len);
      close (data -> fbfd);
      close (data -> ttyfd);
      yfree (videodriver);
      yfree (data);
      return 1;
    }

  /* initialise switching */
  {
    struct sigaction act;

    memset (&act, 0, sizeof(act));
    act.sa_handler = fbdevSwitchSignals;
    sigemptyset(&act.sa_mask);
    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGUSR2, &act, NULL);

    memcpy (&(data -> vtmode), &(data -> old_vtmode), sizeof (struct vt_mode));
    data -> vtmode.mode   = VT_PROCESS;
    data -> vtmode.waitv  = 0;
    data -> vtmode.relsig = SIGUSR1;
    data -> vtmode.acqsig = SIGUSR2;

    if (ioctl (data -> ttyfd, VT_SETMODE, &(data -> vtmode)) < 0)
      {
        Y_ERROR ("Couldn't set terminal mode: %s", strerror (errno));
        munmap (data -> data, data -> fscreeninfo.smem_len);
        close (data -> fbfd);
        close (data -> ttyfd);
        yfree (videodriver);
        yfree (data);
        return 1;
      }
  }

  data -> switchState = FBDEV_SWITCH_ACTIVE;
  data -> renderSimply = 0;

  videodriver -> getPixelDimensions  = fbdevGetPixelDimensions;
  videodriver -> getName             = fbdevGetName;
  videodriver -> getResolutions      = fbdevGetResolutions;
  videodriver -> setResolution       = fbdevSetResolution;
  videodriver -> special             = fbdevSpecial;
  videodriver -> beginUpdates        = fbdevBeginUpdates;
  videodriver -> endUpdates          = fbdevEndUpdates;
  videodriver -> drawPixel           = fbdevDrawPixel;
  videodriver -> drawRectangle       = fbdevDrawRectangle;
  videodriver -> drawFilledRectangle = fbdevDrawFilledRectangle;
  videodriver -> blit                = fbdevBlit;  
  videodriver -> getRenderer         = fbdevGetRenderer;

  module -> data = videodriver;

  data -> viewport = viewportCreate (videodriver);
  screenRegisterViewport (data -> viewport);

  return 0;
}

int
finalise (struct Module *module)
{
  struct VideoDriver *videodriver = module -> data;
  struct FBDevVideoDriverData *data = videodriver -> d;
  if (fbdevInstance == NULL)
    return 0;
  Y_TRACE ("Finalising");

  screenUnregisterViewport (data -> viewport);
  viewportDestroy (data -> viewport);
  munmap (data -> data, data -> fscreeninfo.smem_len);
  if (ioctl (data -> fbfd, FBIOPUT_VSCREENINFO, &(data -> old_vscreeninfo)) < 0)
    Y_WARN ("Failed to restore screen settings: %s", strerror (errno));
  if (ioctl (data -> fbfd, FBIOPUTCMAP, &(data -> old_cmap)) < 0)
    Y_WARN ("Failed to restore colour map: %s", strerror (errno));
  close (data -> fbfd);

  if (!isatty (data -> ttyfd))
    Y_WARN ("FD %d is not a TTY... strange things lie ahead...", data -> ttyfd);
  else
    Y_TRACE ("FD %d is a TTY [%s]... all (should be) well...", data -> ttyfd,
             ttyname (data -> ttyfd));

  if (ioctl (data -> ttyfd, KDSETMODE, data -> old_kdmode) < 0)
    Y_WARN ("Failed to restore terminal mode (a): %s", strerror (errno));
  if (ioctl (data -> ttyfd, VT_SETMODE, &(data -> old_vtmode)) < 0)
    Y_WARN ("Failed to restore terminal mode (b): %s", strerror (errno));
  if (tcsetattr (data -> ttyfd, TCSANOW, &(data -> term)) != 0)
    Y_WARN ("Failed to restore terminal mode (c): %s", strerror (errno));

  sigaction (SIGUSR1, NULL, NULL);
  sigaction (SIGUSR1, NULL, NULL);

  data -> switchState = FBDEV_SWITCH_FINISHING;
  close (data -> ttyfd);

  indexDestroy (data -> modes, (void (*)(void *))fbdevmodeDestructorFunction);

  fbdevInstance = NULL;
  yfree (data);
  yfree (videodriver);

  yfree (module -> name);

  return 0;
}

/* arch-tag: 59bc859e-a24d-41bb-95c1-86ff5f6136c9
 */
